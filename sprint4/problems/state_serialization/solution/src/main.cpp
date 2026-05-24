#include "sdk.h"
#include "logger.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>
#include <memory>

#include "json_loader.h"
#include "request_handler.h"
#include "players.h"
#include "app.h"
#include "ticker.h"
#include "command_line_parser.h"
#include "serialization.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <fstream>

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;
namespace fs = std::filesystem;

namespace {

void SaveStateToFile(const model::Game& game, const app::Players& players,
                     const std::filesystem::path& file_path) {
    serialization::GameStateRepr repr(game, players);
    auto tmp_path = file_path;
    tmp_path += ".tmp";
    {
        std::ofstream ofs(tmp_path, std::ios::binary);
        if (!ofs) {
            throw std::runtime_error("Failed to open temp file for writing");
        }
        boost::archive::text_oarchive oa(ofs);
        oa << repr;
        ofs.flush();
        ofs.close();
        if (ofs.fail()) {
            throw std::runtime_error("Failed to write state to temp file");
        }
    }
    std::filesystem::rename(tmp_path, file_path);
}

bool LoadStateFromFile(model::Game& game, app::Players& players,
                       const std::filesystem::path& file_path) {
    try {
        std::ifstream ifs(file_path, std::ios::binary);
        if (!ifs) return false;
        boost::archive::text_iarchive ia(ifs);
        serialization::GameStateRepr repr;
        ia >> repr;
        repr.RestoreTo(game, players);
        return true;
    } catch (const std::exception& e) {
        logging::LogError(0, e.what(), "load state");
        return false;
    }
}

}


namespace {

template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

}  // namespace

int main(int argc, const char* argv[]) {
    try {
        logging::InitBoostLog();

        auto args = command::ParseCommandLine(argc, argv);

        if (!args) {
            return EXIT_FAILURE;
        }

        if (args->help) {
            return EXIT_SUCCESS;
        }

        model::Game game = json_loader::LoadGame(args->config_file,args->random_spawn);
        fs::path static_dir = args->www_root;

        app::Players players;

        if (args->state_file && std::filesystem::exists(*args->state_file)) {
            if (!LoadStateFromFile(game, players, *args->state_file)) {
                return EXIT_FAILURE;
            }
        }

        app::Application app(game, players);   // создаём Application



        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, int) {
            if (!ec) ioc.stop();
        });

        auto api_strand = net::make_strand(ioc);
        bool auto_tick = false;
        if (args->tick_period.has_value()) {
            auto_tick = true;
        }
        if (args->tick_period.has_value()) {
        auto ticker = std::make_shared<Ticker>(api_strand, std::chrono::milliseconds(args->tick_period.value()),
                                               [&app](std::chrono::milliseconds delta) {app.UpdateGameState(delta); }
                                               );
        ticker->Start();
        }

        std::chrono::milliseconds accumulated_save_time{0};
        boost::signals2::scoped_connection auto_save_conn;
        if (args->state_file) {
            auto save_state = [&app, state_path = *args->state_file]() {
                try {
                    SaveStateToFile(app.GetGame(), app.GetPlayers(), state_path);
                } catch (const std::exception& e) {
                    logging::LogError(0, e.what(), "auto-save");
                }
            };

            if (args->save_state_period) {
                auto_save_conn = app.DoOnTick(
                    [save_state, period = *args->save_state_period, &accumulated_save_time]
                    (std::chrono::milliseconds delta) mutable {
                        accumulated_save_time += delta;
                        if (accumulated_save_time >= period) {
                            save_state();
                            accumulated_save_time = std::chrono::milliseconds{0};
                        }
                    });
            }
        }



        auto handler = std::make_shared<http_handler::RequestHandler>(
            app, static_dir, api_strand, auto_tick);

        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;

        logging::LogServerStarted(port, address.to_string());

        http_server::ServeHttp(ioc, {address, port},
                               [handler](auto&& req, auto&& send) {
                                   (*handler)(std::forward<decltype(req)>(req),
                                              std::forward<decltype(send)>(send));
                               }
                               );

        RunWorkers(std::max(1u, num_threads), [&ioc] { ioc.run(); });

        if (args->state_file) {
            try {
                SaveStateToFile(app.GetGame(), app.GetPlayers(), *args->state_file);
            } catch (const std::exception& e) {
                logging::LogError(0, e.what(), "final save");
                // не прерываем завершение
            }
        }

        logging::LogServerExited(0);
        return EXIT_SUCCESS;

    } catch (const std::exception& ex) {
        logging::LogServerExited(EXIT_FAILURE, ex.what());
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}
