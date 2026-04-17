#include "boost_log_shell.h"
#include "log_request_handler.h"
#include "sdk.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/date_time.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <optional>
#include <thread>

#include "json_loader.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;

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

struct Args {
  boost::optional<uint64_t> tick_period;
  std::string config_file;
  std::string www_root;
  bool randomize_spawn_point;
};

[[nodiscard]] std::optional<Args> ParseCommandLine(int argc,
                                                   const char *const argv[]) {
  namespace po = boost::program_options;
  po::options_description desc{"Allowed options"s};

  Args args;
  desc.add_options()("help,h", "produce help message");
  desc.add_options()("tick-period,t",
                     po::value(&args.tick_period)->value_name("milliseconds"s),
                     "set tick period");
  desc.add_options()("config-file,c",
                     po::value(&args.config_file)->value_name("file"s),
                     "set config file path");
  desc.add_options()("www-root,w",
                     po::value(&args.www_root)->value_name("dir"s),
                     "set static files root");
  desc.add_options()("randomize-spawn-points,r",
                     po::bool_switch(&args.randomize_spawn_point),
                     "spawn dogs at random positions");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.contains("help"s)) {
    std::cout << desc;
    return std::nullopt;
  }

  if (!vm.contains("config-file"s)) {
    throw std::runtime_error("Config file have not been specified"s);
  }
  if (!vm.contains("www-root"s)) {
    throw std::runtime_error("Static directory path is not specified"s);
  }

  return args;
}

int main(int argc, const char *argv[]) {
  try {
    if (const auto args = ParseCommandLine(argc, argv)) {

      // 1. Инициализируем io_context
      const unsigned num_threads = std::thread::hardware_concurrency();
      net::io_context ioc(num_threads);

      // 2. Загружаем карту из файла и построить модель игры

      model::Game game =
          json_loader::LoadGame(args->config_file, ioc, args->tick_period,
                                args->randomize_spawn_point);

      std::string static_catalog = args->www_root;

      // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
      net::signal_set signals(ioc, SIGINT, SIGTERM);

      signals.async_wait([&ioc](const sys::error_code &ec,
                                [[maybe_unused]] int signal_number) {
        server_log::GetLogShell().OnExit(ec);
        ioc.stop();
      });

      // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
      http_handler::LogRequestHandler handler{
          http_handler::RequestHandler{game, std::move(static_catalog)}};

      // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику
      // запросов
      const auto address = net::ip::make_address("0.0.0.0");
      constexpr net::ip::port_type port = 8080;
      http_server::ServeHttp(
          ioc, {address, port},
          [&handler](auto &&req, std::string host, auto &&send) {
            handler(std::forward<decltype(req)>(req),
                    std::forward<decltype(host)>(host),
                    std::forward<decltype(send)>(send));
          });

      server_log::GetLogShell().OnStart(port, address.to_string());

      // 6. Запускаем обработку асинхронных операций
      RunWorkers(std::max(1u, num_threads), [&ioc] { ioc.run(); });
    }
  } catch (const std::exception &ex) {

    server_log::GetLogShell().OnExit(ex);
    return EXIT_FAILURE;
  }
}
