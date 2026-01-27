#ifndef SERVER_HPP
#define SERVER_HPP

#include "irc.hpp"
#include "Client.hpp"
#include "Channel.hpp"

// Macros para cores
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

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
		std::map<std::string, Channel*> _channels;

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
		void printRequest(const std::string& input, const commandRequest& req);
		commandRequest _splitRequest(const std::string& req);
		std::string _printHelpInfo(int sender_fd);
		std::string _setNickName(commandRequest& request, int sender_fd);
		std::string _pingPong(commandRequest& request, int sender_fd);
		std::string _setPassWord(commandRequest& request, int fd);
        std::string _setUserName(commandRequest& request, int fd);
        std::string attemptRegistration(int fd);
		std::string _joinChannel(commandRequest& request, int fd);

	public:
		Server(int port, const std::string& password);
		~Server();
		void run(void);
		//void ft_free();
		
};

#endif
