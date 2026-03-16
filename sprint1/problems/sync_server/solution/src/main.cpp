#ifdef WIN32
#include <sdkddkver.h>
#endif
// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <optional>

namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace std::literals;
namespace beast = boost::beast;
namespace http = beast::http;
using StringRequest = http::request<http::string_body>;
using StringResponse = http::response<http::string_body>;

// Структура ContentType задаёт область видимости для констант,
// задающих значения HTTP-заголовка Content-Type
struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    // При необходимости внутрь ContentType можно добавить и другие типы контента
};

// Создаёт StringResponse с заданными параметрами
StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                  bool keep_alive,
                                  std::string_view content_type = ContentType::TEXT_HTML) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}

StringResponse HandleRequest(StringRequest&& req) {
    const auto text_response = [&req](http::status status, std::string_view text) {
        return MakeStringResponse(status, text, req.version(), req.keep_alive());
    };

    if(req.method() == boost::beast::http::verb::get) {
        return text_response(http::status::ok, "Hello, "s + req.target().substr(1));
    }
    else if(req.method() == boost::beast::http::verb::head) {
        return text_response(http::status::ok, "");
    }
    
    auto response = text_response(http::status::method_not_allowed, "Invalid method"sv);
    response.set(http::field::allow, "GET, HEAD");
    return response;
} 

std::optional<StringRequest> ReadRequest(tcp::socket& socket, beast::flat_buffer& buffer){
    beast::error_code ec;
    StringRequest req;

    http::read(socket, buffer, req, ec);

    if(ec == http::error::end_of_stream){
        return std::nullopt;
    }

    if(ec) {
        throw std::runtime_error("Failed to read request: "s.append(ec.message()));
    }

    return req;
}

void DumpRequest(const StringRequest& req) {
    std::cout << req.method_string() << ' ' << req.target() << std::endl;
    // Выводим заголовки запроса
    for (const auto& header : req) {
        std::cout << "  "sv << header.name_string() << ": "sv << header.value() << std::endl;
    }
} 

template <typename RequestHandler>
void HandleConnection(tcp::socket& socket, RequestHandler&& handle_request) {
    try {
        // Буфер для чтения данных в рамках текущей сессии.
        beast::flat_buffer buffer;

        // Продолжаем обработку запросов, пока клиент их отправляет
        while (auto request = ReadRequest(socket, buffer)) {
            DumpRequest(*request);
            StringResponse response = HandleRequest(*std::move(request));
            http::write(socket, response);
            if (response.need_eof()) {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    beast::error_code ec;
    // Запрещаем дальнейшую отправку данных через сокет
    socket.shutdown(tcp::socket::shutdown_send, ec);
}

int main() {
    net::io_context io_context;

    const auto address = net::ip::make_address("0.0.0.0");
    constexpr unsigned short port = 8080;

    tcp::acceptor acceptor(io_context, {address, port});
    std::cout << "Server has started..." << std::endl;

    while (true) {
        tcp::socket socket(io_context);
        acceptor.accept(socket);

        // Запускаем обработку взаимодействия с клиентом в отдельном потоке
        std::thread t(
            [](tcp::socket socket) {
                // Вызываем HandleConnection, передавая ей функцию-обработчик запроса
                HandleConnection(socket, HandleRequest);
            },
            std::move(socket));
        t.detach();
    }
}
