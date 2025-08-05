#include <iostream>

#include "conversions.h"
#include "requests.h"
#include "libgrmproto/Material.pb.h"
#include "boost/asio.hpp"
#include "libgrmproto/Render.pb.h"
#include "boost/program_options.hpp"

using boost::asio::ip::tcp;
namespace po = boost::program_options;

int main(int argc, char *argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("port", po::value<unsigned short>()->default_value(0), "port to listen on. Use 0 (default) to have one picked for you.");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    auto listen_port = vm["port"].as<unsigned short>();

    boost::asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), listen_port));
    std::cout << "listening on port " << acceptor.local_endpoint().port() << std::endl;
    while (true) {
        auto connectionStream = tcp::iostream();
        acceptor.accept(connectionStream.socket());
        auto remote_endpoint = connectionStream.socket().remote_endpoint();
        std::cout
            << std::format("{}: Connected", toString(remote_endpoint))
            << std::endl;
        try {
            handle_request(std::move(connectionStream));
        }catch (std::exception& e) {
            std::cout
                << std::format("{}: General error: {}", toString(remote_endpoint), e.what())
                << std::endl;
        }
    }
}