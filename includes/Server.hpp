#ifndef SERVER_HPP
#define SERVER_HPP
#include <vector>
#include <poll.h>
#include <map>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include "Client.hpp"

class Server
{
	private:
		int _serverFd;
		std::string _password;
		std::vector<struct pollfd> _pollfds;
		std::map<int, Client*> _clients;

		void setupSocket(int port);
		void acceptClient();
		void receiveData(int clientFd);
		void sendData(int clientFd);
		void removeClient(int clientFd);

	public:
		Server(int port, const std::string& password);
		~Server();
		void run();
		
};

#endif
