/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 00:00:00 by hmateque          #+#    #+#             */
/*   Updated: 2026/01/26 14:50:28 by hmateque         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../../includes/Server.hpp"

// Função auxiliar para validar nickname segundo RFC 1459
static bool isValidNickname(const std::string& nick)
{
	if (nick.empty() || nick.length() > 9)
		return false;
	
	// Primeiro caractere deve ser letra ou especial
	char first = nick[0];
	if (!std::isalpha(first) && first != '[' && first != ']' && 
		first != '\\' && first != '`' && first != '_' && 
		first != '^' && first != '{' && first != '|' && first != '}')
		return false;
	
	// Demais caracteres podem ser letra, dígito ou especial
	for (size_t i = 1; i < nick.length(); ++i)
	{
		char c = nick[i];
		if (!std::isalnum(c) && c != '[' && c != ']' && 
			c != '\\' && c != '`' && c != '_' && 
			c != '^' && c != '{' && c != '|' && c != '}' && c != '-')
			return false;
	}
	return true;
}

std::string Server::_setNickName(commandRequest& request, int sender_fd)
{
	std::string response;
	std::string oldNick = _clients[sender_fd]->getNickname();
	
	// Se não foi fornecido nickname
	if (request.args.empty())
	{
		// ERR_NONICKNAMEGIVEN (431)
		std::string nick = oldNick.empty() ? "*" : oldNick;
		response = ":localhost 431 " + nick + " :No nickname given\r\n";
		return response;
	}
	
	std::string newNick = request.args[0];
	
	// Validar formato do nickname
	if (!isValidNickname(newNick))
	{
		// ERR_ERRONEUSNICKNAME (432)
		std::string nick = oldNick.empty() ? "*" : oldNick;
		response = ":localhost 432 " + nick + " " + newNick + " :Erroneous nickname\r\n";
		return response;
	}
	
	// Verificar se o nickname já está em uso
	for (std::map<int, Client*>::iterator it = _clients.begin(); 
		it != _clients.end(); ++it)
	{
		if (it->first != sender_fd && it->second->getNickname() == newNick)
		{
			// ERR_NICKNAMEINUSE (433)
			std::string nick = oldNick.empty() ? "*" : oldNick;
			response = ":localhost 433 " + nick + " " + newNick + " :Nickname is already in use\r\n";
			return response;
		}
	}
	
	// Definir o novo nickname
	_clients[sender_fd]->setNickname(newNick);
	
	// Se o cliente já tinha um nickname (mudança de nick)
	if (!oldNick.empty())
	{
		// Notificar a mudança
		response = ":" + oldNick + "!~" + _clients[sender_fd]->getUsername() + 
				   "@localhost NICK :" + newNick + "\r\n";
		std::cout << "Client " << sender_fd << " changed nickname from " 
				  << oldNick << " to " << newNick << std::endl;
	}
	else
	{
		// Primeiro nickname definido
		std::cout << "Client " << sender_fd << " set nickname to: " 
				  << newNick << std::endl;
		
		// Se já tiver username definido, consideramos autenticado
		if (!_clients[sender_fd]->getUsername().empty())
		{
			// RPL_WELCOME (001)
			response = ":localhost 001 " + newNick + " :Welcome to the Internet Relay Network " + 
					   newNick + "!~" + _clients[sender_fd]->getUsername() + "@localhost\r\n";
			_clients[sender_fd]->auth = true;
		}
	}
	
	return response;
}

