#include "log_attributes.h"
#include "log_request_handler.h"
#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/date_time.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <iostream>
#include <thread>

#include "json_loader.h"

using namespace std::literals;
namespace keywords = boost::log::keywords;
namespace json = boost::json;
namespace net = boost::asio;
namespace sys = boost::system;
namespace logging = boost::log;

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn> void RunWorkers(unsigned n, const Fn &fn) {
  n = std::max(1u, n);
  std::vector<std::jthread> workers;
  workers.reserve(n - 1);
  // Запускаем n-1 рабочих потоков, выполняющих функцию fn
  while (--n) {
    workers.emplace_back(fn);
  }
  fn();
}

void MyFormatter(logging::record_view const &rec,
                 logging::formatting_ostream &strm) {
  // auto ts = rec[logging::trivial::];
  // strm << to_iso_extended_string(*ts) << ": ";

  // выводим само сообщение
  // strm << rec[logging::expressions::smessage];

  strm << "{\"timestamp\":\""
       << to_iso_extended_string(
              boost::posix_time::microsec_clock::local_time())
       << "\",\"data\":"s
       << logging::extract<json::value>("AdditionalData", rec)
       << ",\"message\":\"" << rec[logging::expressions::smessage] << "\"}";

  // // выводим уровень, заключая его в угловые скобки
  // strm << "<" << rec[logging::trivial::severity] << "> ";

  // // выводим само сообщение
  // strm << rec[logging::expressions::smessage];
}

int main(int argc, const char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: game_server <game-config-json> <static-catalog>"sv
              << std::endl;
    return EXIT_FAILURE;
  }
  try {
    // logging::add_common_attributes();
    logging::add_console_log(std::clog, keywords::format = &MyFormatter,
                             keywords::auto_flush = true);
    // 1. Загружаем карту из файла и построить модель игры
    model::Game game = json_loader::LoadGame(argv[1]);
    std::string static_catalog = argv[2];

    // 2. Инициализируем io_context
    const unsigned num_threads = std::thread::hardware_concurrency();
    net::io_context ioc(num_threads);

    // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&ioc](const sys::error_code &ec, [[maybe_unused]] int signal_number) {
          if (!ec) {
            BOOST_LOG_TRIVIAL(info)
                << logging::add_value(additional_data,
                                      json::value{{"code"s, ec.value()}})
                << "server exited";
            ioc.stop();
          };

          BOOST_LOG_TRIVIAL(error)
                << logging::add_value(additional_data,
                                      json::value{{"code"s, ec.value()}, {"exception", ec.what()}})
                << "server exited";
        });

    // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
    http_handler::LogRequestHandler handler{
        http_handler::RequestHandler{game, std::move(static_catalog)}};

    // http_handler::RequestHandler handler{game, std::move(static_catalog)};

    // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
    const auto address = net::ip::make_address("0.0.0.0");
    constexpr net::ip::port_type port = 8080;
    http_server::ServeHttp(ioc, {address, port},
                           [&handler](auto &&req, std::string host, auto &&send) {
                             handler(std::forward<decltype(req)>(req), 
                                     std::forward<decltype(host)>(host),
                                     std::forward<decltype(send)>(send));
                           });

    // Эта надпись сообщает тестам о том, что сервер запущен и готов
    // обрабатывать запросы
    // std::cout << "Server has started..."sv << std::endl;

    BOOST_LOG_TRIVIAL(info)
        << logging::add_value(
               additional_data,
               json::value{{"port"s, port}, {"address"s, address.to_string()}})
        << "server started"sv;

    // 6. Запускаем обработку асинхронных операций
    RunWorkers(std::max(1u, num_threads), [&ioc] { ioc.run(); });
  } catch (const std::exception &ex) {
    BOOST_LOG_TRIVIAL(error)
                << logging::add_value(additional_data,
                                      json::value{{"code"s, EXIT_FAILURE}, {"exception", ex.what()}})
                << "server exited";
    return EXIT_FAILURE;
  }
}
