#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <string>

class	Client
{
	public:
		int		fd;
		std::string	recvBuffer;
		std::string	sendBuffer;

		Client(int fd);
		~Client();
		int getClientfd() const;
};

#endif

