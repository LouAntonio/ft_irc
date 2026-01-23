#include "../includes/Client.hpp"

Client::Client(int fd) : fd(fd), auth(false) {}

Client::~Client() {}
