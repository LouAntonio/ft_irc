#ifndef SERVER_HPP
#define SERVER_HPP

// contains necessary libraries
#include <vector>
#include <map>
#include "string"

//server libraries
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <exception>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <iostream>
#include <csignal>
#include <cerrno>

// includes project files
#include "Client.hpp"
#include "Channel.hpp"

// Struct Request
typedef struct s_Request
{
    std::vector<std::string> args;
    std::string command;
    bool invalidMessage;
} commandRequest;

class Client;
class Channel;
class Server
{
	private:
		int _serverFd;
		int _port;
		std::string _password;
		std::vector<struct pollfd> _pollfds;
		std::map<int, Client*> _clients;

		void setupSocket(void);
		void acceptClient(void);
		void receiveData(int indexFd);
		void sendData(int indexFd);
		void removeClient(int indexFd);
		static void handler(int signal);
		void enablePollout(int fd);
		void disablePollout(int fd);
		std::string welcome(void);
		//bool	auth(int clientFd);

		std::string _parsing(const std::string& msg, int sender_fd);
		commandRequest _splitRequest(const std::string& req);
		std::string _printHelpInfo(int sender_fd);
		std::string _setNickName(commandRequest& request, int sender_fd);
		std::string _pingPong(commandRequest& request, int sender_fd);

	public:
		Server(int port, const std::string& password);
		~Server();
		void run(void);
		//void ft_free();
		
};

#endif
