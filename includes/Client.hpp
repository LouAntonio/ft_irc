#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include "Command.hpp"

class	Client
{
	public:
		int			fd;
		std::string	recvBuffer;
		std::string	sendBuffer;
		bool		auth;

		bool		readyCommand;
		Command		*command;

		Client(int fd);
		~Client();
		int		getClientfd() const;
		void	setReadyCommand(bool status);
		bool	getReadyCommand() const;
};

#endif
