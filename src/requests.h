//
// Created by atomr on 7/27/2025.
//

#ifndef REQUESTS_H
#define REQUESTS_H
#include <boost/asio/ip/tcp.hpp>

void handle_request(boost::asio::ip::tcp::iostream sockStream);

#endif //REQUESTS_H
