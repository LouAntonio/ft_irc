/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/24 12:03:08 by hmateque          #+#    #+#             */
/*   Updated: 2026/02/03 11:43:09 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.hpp"

std::string Server::_parsing(const std::string& msg, int sender_fd) 
{
	commandRequest request(_splitRequest(msg));
	if (request.command.empty()) return "";

	for (size_t i = 0; i < request.command.length(); ++i)
		request.command[i] = std::toupper(request.command[i]);
	printRequest(msg, request);
	if (request.invalidMessage)
		return ("Invalid message!\r\n");

	if (request.command == "CAP") return ""; 
	if (request.command == "PASS") return _setPassWord(request, sender_fd);
	if (request.command == "NICK") return _setNickName(request, sender_fd);
	if (request.command == "USER") return _setUserName(request, sender_fd);
	if (request.command == "PING") return _pingPong(request, sender_fd);
	if (request.command == "JOIN") return _joinChannel(request, sender_fd);
	if (request.command == "INVITE") return _invite(request, sender_fd);
	if (request.command == "PRIVMSG") return _privmsg(request, sender_fd);
	if (request.command == "KICK") return _kick(request, sender_fd);
	if (request.command == "TOPIC") return _topic(request, sender_fd);
	if (request.command == "MODE") return _mode(request, sender_fd);
	if (request.command == "HELP" || request.command == "H") return _printHelpInfo(sender_fd);

	// Resposta padrão RFC para comando desconhecido (421)
	std::string nick = _clients[sender_fd]->getNickname().empty() ? "*" : _clients[sender_fd]->getNickname();
	return ":localhost 421 " + nick + " " + request.command + " :Unknown command\r\n";
}

commandRequest Server::_splitRequest(const std::string& req)
{
	commandRequest request;
	request.invalidMessage = false;
	size_t i = 0;
	size_t j = 0;

	if (req[i] == ' ' || !req[i]) 
	{
		request.invalidMessage = true;
		return (request);
	}
	j = i;
	while (req[i])
	{
		if (req[i] == ' ')
		{
			if (req[i + 1] == ' ') 
			{
				request.invalidMessage = true;
				return (request);
			}
			request.args.push_back(req.substr(j, i - j));
			while (req[i] == ' ')
				i++;
			j = i;
		}
		if (req[i] == ':')
		{
			if (req[i - 1] != ' ') 
			{
				request.invalidMessage = true;
				return (request);
			}
			request.args.push_back(req.substr(i + 1, req.length() - i));
			request.command = request.args[0];
			request.args.erase(request.args.begin());
			return (request);
		}
		i++;
	}

	if (i && req[j])
		request.args.push_back(req.substr(j, i - j));
	request.command = request.args[0];
	request.args.erase(request.args.begin());
	return (request);
}

void Server::printRequest(const std::string& input, const commandRequest& req)
{
	std::cout << CYAN << "========================================" << RESET << std::endl;
	std::cout << YELLOW << "Input: " << RESET << "\"" << input << "\"" << std::endl;
	
	if (req.invalidMessage) {
		std::cout << RED << "❌ MENSAGEM INVÁLIDA!" << RESET << std::endl;
		return;
	}
	
	std::cout << GREEN << "✓ Válida" << RESET << std::endl;
	std::cout << BLUE << "Comando: " << RESET << "\"" << req.command << "\"" << std::endl;
	std::cout << MAGENTA << "Argumentos (" << req.args.size() << "):" << RESET << std::endl;
	
	for (size_t i = 0; i < req.args.size(); i++) {
		std::cout << "  [" << i << "] = \"" << req.args[i] << "\"" << std::endl;
	}
	std::cout << std::endl;
}

std::string Server::_pingPong(commandRequest& request, int sender_fd)
{
	std::string response;
	(void)sender_fd;
	if (request.args.empty())
	{
		response = ":localhost 409 * :No origin specified\r\n"; // ERR_NOORIGIN (409)
	}
	else
	{
		response = ":localhost PONG :" + request.args[0] + "\r\n";
	}
	return response;
}

std::string Server::attemptRegistration(int fd) 
{
	Client *c = _clients[fd];

	if (c->getHasPass() && c->getHasNick() && c->getHasUser() && !c->isAuth()) 
	{
		c->setAuth(true);
		std::string nick = c->getNickname();
		
		std::string res;
		// RPL_WELCOME (001) - O Irssi exige isso para não desconectar
		res += ":localhost 001 " + nick + " :Welcome to the IRC Network, " + nick + "\r\n";
		
		// Agora sim você envia o welcome banner
		res += welcome();
		
		std::cout << "DEBUG: Cliente " << fd << " registrado como " << nick << std::endl;
		return res;
	}
	return "";
}
