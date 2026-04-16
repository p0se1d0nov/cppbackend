#include "http_server.h"
#include "log_request_handler.h"

#include <boost/json.hpp>
#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <iostream>

namespace http_server {

using namespace std::literals;
namespace json = boost::json;
namespace logging = boost::log;
void ReportError(beast::error_code ec, std::string_view what) {
  BOOST_LOG_TRIVIAL(error)
        << logging::add_value(additional_data,
                              json::value{{"code"s, ec.value()}, {"text", ec.message()}, {"where", what}})
        << "error";
  // std::cerr << what << ": "sv << ec.message() << std::endl;
}

std::string SessionBase::GetRemoteHost() const {
  return stream_.socket().remote_endpoint().address().to_string();
}

void SessionBase::Run() {
  net::dispatch(stream_.get_executor(),
                beast::bind_front_handler(&SessionBase::Read, GetSharedThis()));
}

void SessionBase::Read() {
  // Очищаем запрос от прежнего значения (метод Read может быть вызван
  // несколько раз)
  request_ = {};
  stream_.expires_after(30s);
  // Считываем request_ из stream_, используя buffer_ для хранения считанных
  // данных
  http::async_read(
      stream_, buffer_, request_,
      // По окончании операции будет вызван метод OnRead
      beast::bind_front_handler(&SessionBase::OnRead, GetSharedThis()));
}

void SessionBase::OnRead(beast::error_code ec, std::size_t bytes_read) {
  if (ec == http::error::end_of_stream) {
    // Нормальная ситуация - клиент закрыл соединение
    return Close();
  }
  if (ec) {
    return ReportError(ec, "read"sv);
  }
  HandleRequest(std::move(request_));
}

void SessionBase::OnWrite(bool close, beast::error_code ec,
                          std::size_t bytes_written) {
  if (ec) {
    return ReportError(ec, "write"sv);
  }

  if (close) {
    // Семантика ответа требует закрыть соединение
    return Close();
  }

  // Считываем следующий запрос
  Read();
}

void SessionBase::Close() {
  beast::error_code ec;
  ec = stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
  if (ec) {
    return ReportError(ec, "close"sv);
    // std::cerr << ec.message() << std::endl;
  }
}

} // namespace http_server