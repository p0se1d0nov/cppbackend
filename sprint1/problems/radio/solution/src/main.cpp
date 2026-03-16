#include "audio.h"
#include <iostream>
#include <string>

#include <boost/asio.hpp>

namespace net = boost::asio;
using net::ip::udp;
using namespace std::literals;

void StartClient(uint16_t port) {
    net::io_context io_context;
    udp::socket socket(io_context, udp::v4());
    
    Recorder recorder(ma_format_u8, 1);
    
    while (true) {
        std::string dummy;
        std::cout << "Press Enter to record message..." << std::endl;
        std::getline(std::cin, dummy);
        
        auto rec_result = recorder.Record(65000, 1.5s);
        std::cout << "Recording done. Enter IP: " << std::endl;
        std::string ip;
        std::getline(std::cin, ip);
        
        boost::system::error_code error;
        udp::endpoint endpoint(net::ip::make_address(ip, error), port);
        if (error) {
            std::cerr << "Invalid IP address: " << ip << std::endl;
            continue;
        }
        
        size_t bytes_to_send = rec_result.frames * recorder.GetFrameSize();
        socket.send_to(net::buffer(rec_result.data.data(), bytes_to_send), endpoint, 0, error);
        
        if (error) {
            std::cerr << "Send error: " << error.message() << std::endl;
        } else {
            std::cout << "Message sent" << std::endl;
        }
    }
}

void StartServer(uint16_t port) {
    net::io_context io_context;
    udp::socket socket(io_context, udp::endpoint(udp::v4(), port));
    
    Player player(ma_format_u8, 1);
    
    std::vector<char> buffer;
    buffer.resize(65000 * player.GetFrameSize()); // Максимальный размер
    
    while (true) {
        udp::endpoint sender_endpoint;
        boost::system::error_code error;
        size_t bytes_received = socket.receive_from(net::buffer(buffer), sender_endpoint, 0, error);
        
        if (error) {
            std::cerr << "Receive error: " << error.message() << std::endl;
            continue;
        }
        
        size_t frames = bytes_received / player.GetFrameSize();
        
        player.PlayBuffer(buffer.data(), frames, 1.5s);
        
        std::cout << "Message played" << std::endl;
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <client|server> <port>" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    uint16_t port = std::stoi(argv[2]);

    if (mode == "client") {
        StartClient(port);
    } else if (mode == "server") {
        StartServer(port);
    } else {
        std::cerr << "Invalid mode. Use 'client' or 'server'" << std::endl;
        return 1;
    }

    return 0;
}