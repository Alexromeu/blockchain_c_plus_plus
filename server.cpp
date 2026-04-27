#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <sstream>
#include "blockchain_cplusplus.cpp"


class tcp_server {
public:
    tcp_server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::cout << "New client connected: " << socket.remote_endpoint() << std::endl;
                    std::thread(&tcp_server::handle_client, this, std::move(socket)).detach();
                }
                do_accept();
            });
    }

    void handle_client(tcp::socket socket) {
        try {
            boost::asio::streambuf buffer;
            while (true) {
                boost::asio::read_until(socket, buffer, "\n");
                std::istream is(&buffer);
                std::string command;
                std::getline(is, command);
                std::cout << "Received command: " << command << std::endl;

                // Process the command and interact with the blockchain
                std::string response = process_command(command);
                boost::asio::write(socket, boost::asio::buffer(response + "\n"));
            }
        } catch (std::exception& e) {
            std::cerr << "Client disconnected: " << e.what() << std::endl;
        }
    }

    std::string process_command(const std::string& command) {
        // This is where you would parse the command and interact with the blockchain
        // For example, you could have commands like "MINE", "BALANCE <address>", etc.
        // For now, we'll just echo the command back
        return "Command received: " + command;
    }

    tcp::acceptor acceptor_;
};