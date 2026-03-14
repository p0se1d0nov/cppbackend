#ifdef WIN32
#include <sdkddkver.h>
#endif

#include "seabattle.h"

#include <atomic>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <string_view>

namespace net = boost::asio;
using net::ip::tcp;
using namespace std::literals;

void PrintFieldPair(const SeabattleField &left, const SeabattleField &right)
{
    auto left_pad = "  "s;
    auto delimeter = "    "s;
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
    for (size_t i = 0; i < SeabattleField::field_size; ++i)
    {
        std::cout << left_pad;
        left.PrintLine(std::cout, i);
        std::cout << delimeter;
        right.PrintLine(std::cout, i);
        std::cout << std::endl;
    }
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
}

template <size_t sz>
static std::optional<std::string> ReadExact(tcp::socket &socket)
{
    boost::array<char, sz> buf;
    boost::system::error_code ec;

    net::read(socket, net::buffer(buf), net::transfer_exactly(sz), ec);

    if (ec)
    {
        return std::nullopt;
    }

    return {{buf.data(), sz}};
}

static bool WriteExact(tcp::socket &socket, std::string_view data)
{
    boost::system::error_code ec;

    net::write(socket, net::buffer(data), net::transfer_exactly(data.size()), ec);

    return !ec;
}

class SeabattleAgent
{
public:
    SeabattleAgent(const SeabattleField &field)
        : my_field_(field)
    {
    }

    void StartGame(tcp::socket &socket, bool my_initiative)
    {
        while (true)
        {
            PrintFields();

            if (IsGameEnded())
            {
                std::cout << "Game ended. " << std::endl;
                
                if(my_field_.IsLoser()) {
                    std::cout << "You lose!" << std::endl;
                }
                else {
                    std::cout << "You win!" << std::endl;
                }

                break;
            }

            if (my_initiative)
            {
                std::cout << "Your move: ";
                std::string move_str;
                std::cin >> move_str;

                auto move = ParseMove(move_str);
                if (!move)
                {
                    std::cout << "Wrong move format" << std::endl;
                    continue;
                }

                if (!WriteExact(socket, MoveToString(*move)))
                {
                    std::cout << "Connection lost" << std::endl;
                    break;
                }

                auto result = ReadExact<1>(socket);
                if (!result)
                {
                    std::cout << "Connection lost" << std::endl;
                    break;
                }

                if (static_cast<SeabattleField::ShotResult>((*result)[0]) == SeabattleField::ShotResult::HIT)
                {
                    other_field_.MarkHit(move->second, move->first);
                    std::cout << "Hit!" << std::endl;
                    my_initiative = true;
                }
                else if(static_cast<SeabattleField::ShotResult>((*result)[0]) == SeabattleField::ShotResult::KILL)
                {
                    other_field_.MarkKill(move->second, move->first);
                    std::cout << "Kill!" << std::endl;
                    my_initiative = true;
                }
                else
                {
                    other_field_.MarkMiss(move->second, move->first);
                    std::cout << "Miss!" << std::endl;
                    my_initiative = false;
                }
            }
            else
            {
                std::cout << "Waiting for turn..." << std::endl;
                auto move_str_opt = ReadExact<2>(socket);
                if (!move_str_opt)
                {
                    std::cout << "Connection lost" << std::endl;
                    break;
                }

                auto move = ParseMove(*move_str_opt);
                if (!move)
                {
                    std::cout << "Received wrong move format" << std::endl;
                    continue;
                }

                auto result = my_field_.Shoot(move->second, move->first);
                if (result == SeabattleField::ShotResult::HIT)
                {
                    char response = static_cast<char>(SeabattleField::ShotResult::HIT);
                    if (!WriteExact(socket, std::string_view(&response, 1)))
                    {
                        std::cout << "Connection lost" << std::endl;
                        break;
                    }
                    my_initiative = false;
                }
                else if(result == SeabattleField::ShotResult::KILL)
                {
                    char response = static_cast<char>(SeabattleField::ShotResult::KILL);
                    if (!WriteExact(socket, std::string_view(&response, 1)))
                    {
                        std::cout << "Connection lost" << std::endl;
                        break;
                    }
                    my_initiative = false;
                }
                else
                {
                    char response = static_cast<char>(SeabattleField::ShotResult::MISS);
                    if (!WriteExact(socket, std::string_view(&response, 1)))
                    {
                        std::cout << "Connection lost" << std::endl;
                        break;
                    }
                    my_initiative = true;
                }
            }
        }
    }

private:
    static std::optional<std::pair<int, int>> ParseMove(const std::string_view &sv)
    {
        if (sv.size() != 2)
            return std::nullopt;

        int p1 = sv[0] - 'A', p2 = sv[1] - '1';

        if (p1 < 0 || p1 > 8)
            return std::nullopt;
        if (p2 < 0 || p2 > 8)
            return std::nullopt;

        return {{p1, p2}};
    }

    static std::string MoveToString(std::pair<int, int> move)
    {
        char buff[] = {static_cast<char>(move.first) + 'A', static_cast<char>(move.second) + '1'};
        return {buff, 2};
    }

    void PrintFields() const
    {
        PrintFieldPair(my_field_, other_field_);
    }

    bool IsGameEnded() const
    {
        return my_field_.IsLoser() || other_field_.IsLoser();
    }

    // TODO: добавьте методы по вашему желанию

private:
    SeabattleField my_field_;
    SeabattleField other_field_;
};

void StartServer(const SeabattleField &field, unsigned short port)
{
    SeabattleAgent agent(field);

    net::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

    boost::system::error_code ec;
    tcp::socket socket{io_context};
    
    std::cout << "Waiting for connection..." << std::endl;
    acceptor.accept(socket, ec);

    if (ec)
    {
        std::cout << "Cannot accept connection" << std::endl;
        return;
    }

    agent.StartGame(socket, false);
};

void StartClient(const SeabattleField &field, const std::string &ip_str, unsigned short port)
{
    SeabattleAgent agent(field);

    boost::system::error_code ec;
    auto endpoint = tcp::endpoint(net::ip::make_address(ip_str, ec), port);

    if (ec)
    {
        std::cout << "Wrong ip format" << std::endl;
    }

    net::io_context io_context;
    tcp::socket socket{io_context};
    socket.connect(endpoint, ec);

    if (ec)
    {
        std::cout << "Can't connect tot server" << std::endl;
        return;
    }

    agent.StartGame(socket, true);
};

int main(int argc, const char **argv)
{
    if (argc != 3 && argc != 4)
    {
        std::cout << "Usage: program <seed> [<ip>] <port>" << std::endl;
        return 1;
    }

    std::mt19937 engine(std::stoi(argv[1]));
    SeabattleField fieldL = SeabattleField::GetRandomField(engine);

    if (argc == 3)
    {
        StartServer(fieldL, std::stoi(argv[2]));
    }
    else if (argc == 4)
    {
        StartClient(fieldL, argv[2], std::stoi(argv[3]));
    }
}
