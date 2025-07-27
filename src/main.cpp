#include <iostream>

#include "conversions.h"
#include "requests.h"
#include "libgrmproto/Material.pb.h"
#include "boost/asio.hpp"
#include "libgrmproto/Render.pb.h"

using boost::asio::ip::tcp;

int main() {
    auto material = grm::protobuf::Material();
    auto const mc = material.mutable_color();
    mc->set_x(1);
    mc->set_y(2);
    mc->set_z(3);
    material.SerializeToOstream(&std::cout);
    std::cout << std::endl;
    std::cout << material.DebugString() << std::endl;

    boost::asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 9000));
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