// Copyright 2021 byteihq <kotov038@gmail.com>

#ifndef TCPSERVER_HANDLECONNECTION_H
#define TCPSERVER_HANDLECONNECTION_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <Logger.h>

using boost::asio::ip::tcp;

class HandleConnection : public boost::enable_shared_from_this<HandleConnection> {
private:
    std::shared_ptr<tcp::socket> socket_;
    boost::asio::ip::address ip_;
    boost::asio::streambuf data_;
    Logger& logger_;
public:
    using pointer = boost::shared_ptr<HandleConnection>;

    explicit HandleConnection(boost::asio::io_service &io_service, Logger& logger);

    static pointer create(boost::asio::io_service &io_service, Logger& logger);

    void start();

    void sendMessage(const std::string &msg);

    void getMessage();

    std::shared_ptr<tcp::socket> getSocket() const;
};


#endif //TCPSERVER_HANDLECONNECTION_H
