/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 00:00:00 by hmateque          #+#    #+#             */
/*   Updated: 2026/01/27 11:36:02 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

std::string Server::_setNickName(commandRequest& request, int fd) 
{
    if (!_clients[fd]->getHasPass())
        return ":localhost 464 * :Password required. Send PASS first.\r\n";

    if (request.args.empty())
        return ":localhost 431 * :No nickname given\r\n";

    if (request.args.size() > 1)
        return ":localhost 431 * :To many arguments for command\r\n";
    std::string newNick = request.args[0];
    if (!isValidNickname(newNick))
        return ":localhost 432 * " + newNick + " :Erroneous nickname\r\n";

    // Declarar oldNick ANTES de usar
    std::string oldNick = _clients[fd]->getNickname();

    // Verificar se o nickname já está em uso
    for (std::map<int, Client*>::iterator it = _clients.begin(); 
        it != _clients.end(); ++it)
    {
        if (it->first != fd && it->second->getNickname() == newNick)
        {
            // ERR_NICKNAMEINUSE (433)
            std::string nick = oldNick.empty() ? "*" : oldNick;
            return ":localhost 433 " + nick + " " + newNick + " :Nickname is already in use\r\n";
        }
    }
    
    // Definir o novo nickname
    _clients[fd]->setNickname(newNick);
    _clients[fd]->setHasNick(true);

    // Se já tinha nickname, notificar a mudança
    if (!oldNick.empty())
        return ":" + oldNick + " NICK " + newNick + "\r\n";
    
    // Caso contrário, tentar completar o registro
    return attemptRegistration(fd);
}

