#include "../includes/Client.hpp"

Client::Client(int fd) : fd(fd) {}

Client::~Client() {}

int Client::getClientfd() const { return this->fd; }
