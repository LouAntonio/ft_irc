#include "../includes/Client.hpp"

Client::Client(int fd) : fd(fd), auth(false) {}

Client::~Client() {}

int Client::getClientfd() const { return this->fd; }

std::string Client::getNickname() const { return this->nickName; }

std::string Client::getUsername() const { return this->userName; }
