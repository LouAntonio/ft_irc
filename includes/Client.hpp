#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <string>

class	Client
{
	public:
		int		fd;
		std::string	recvBuffer;
		std::string	sendBuffer;
		bool		auth;

		Client(int fd);
		~Client();
};

#endif

