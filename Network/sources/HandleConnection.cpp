// Copyright 2021 byteihq <kotov038@gmail.com>

#include <HandleConnection.h>
#include <RequestHandler.h>
#include <Logger.h>
#include <nlohmann/json.hpp>
#include <algorithm>

HandleConnection::HandleConnection(boost::asio::io_service &io_service) : socket_(
        std::make_shared<tcp::socket>(io_service)) {}

HandleConnection::pointer HandleConnection::create(boost::asio::io_service &io_service) {
    return pointer(new HandleConnection(io_service));
}

void HandleConnection::deleteUser(std::shared_ptr<tcp::socket> &socket) {
    for (const auto &user: usersSockets_) {
        if (user.second == socket) {
            Logger::log(user.first + " disconnected", __FILE__, __LINE__);
            usersSockets_.erase(user.first);
            socket->close();
            break;
        }
    }
}

void HandleConnection::sendAll(const std::string &msg) {
    for (const auto& user : usersSockets_) {
        boost::asio::async_write(*user.second, boost::asio::buffer(msg + '\n', msg.size() + 1),
                                 [msg](boost::system::error_code ec, std::size_t /*length*/) {
                                     if (!ec) {
                                         Logger::log("Message successfully sent", __FILE__, __LINE__);
                                     } else {
                                         Logger::log("Failed send message", __FILE__, __LINE__);
                                     }
                                 });
    }
}

void HandleConnection::getMessage() {
    Logger::log("Getting new message", __FILE__, __LINE__);
    auto self(shared_from_this());
    boost::asio::async_read_until(*socket_, data_, '\n',
                                  [this, self](boost::system::error_code ec, std::size_t length) {
                                      if (!ec) {
                                          std::istream ss(&data_);
                                          std::string sData;
                                          std::getline(ss, sData);
                                          if (sData != "\"null\"") {
                                              Logger::log("Message - " + sData, __FILE__, __LINE__);
                                              Logger::log("RequestHandling new message", __FILE__, __LINE__);
                                              auto res = RequestHandler::handle(sData);
                                              if (res.first["type"] == Requests::Auth &&
                                                  res.second["data"] == Replies::Auth::Successful) {
                                                  Logger::log(res.first["sender"].get<std::string>() +
                                                              " successfully connected", __FILE__, __LINE__);
                                                  usersSockets_.insert({res.first["sender"], socket_});
                                              } else if (res.first["type"] == Requests::Disconnect) {
                                                  deleteUser(socket_);
                                              } else if (res.first["type"] == Requests::Msg) {
                                                  for (const auto &users: usersSockets_) {
                                                      if (users.second != socket_) {
                                                          sendMessage(users.second, res.first.dump());
                                                      }
                                                  }
                                              }
                                              sendMessage(res.second.dump());
                                          } else {
                                              getMessage();
                                          }
                                      } else {
                                          deleteUser(socket_);
                                          Logger::log("Error receiving message", __FILE__, __LINE__);
                                      }
                                  });
}

void HandleConnection::sendMessage(const std::string &msg) {
    sendMessage(socket_, msg);
    getMessage();
}

void HandleConnection::start() {
    ip_ = socket_->remote_endpoint().address();
    Logger::log("New connection " + ip_.to_string(), __FILE__, __LINE__);
    getMessage();
}

std::shared_ptr<tcp::socket> HandleConnection::getSocket() const {
    return socket_;
}

void HandleConnection::sendMessage(const std::shared_ptr<tcp::socket> &socket, const std::string &msg) {
    if (!msg.empty()) {
        auto self(shared_from_this());
        boost::asio::async_write(*socket, boost::asio::buffer(msg + '\n', msg.size() + 1),
                                 [self, msg](boost::system::error_code ec, std::size_t /*length*/) {
                                     if (!ec) {
                                         Logger::log("Message successfully sent", __FILE__, __LINE__);
                                     } else {
                                         Logger::log("Failed send message", __FILE__, __LINE__);
                                     }
                                 });
    }
}
