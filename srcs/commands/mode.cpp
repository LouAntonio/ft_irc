/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 11:43:50 by lantonio          #+#    #+#             */
/*   Updated: 2026/02/03 14:04:00 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.hpp"

static std::string modeO(commandRequest& request, int sender_fd, std::map<int, Client*>& clients, std::map<std::string, Channel*>& channels)
{
	if (request.args.size() != 3)
		return ":localhost 461 * :Invalid number of parameters\r\n";
	
	std::string mode = request.args[1];
	std::string channel = request.args[0];
	std::string target_nick = request.args[2];
	
	if (channels.find(channel) == channels.end())
		return ":localhost 403 * :Non-existent channel\r\n";
	if (!channels[channel]->isOperator(sender_fd))
		return ":localhost 482 * :You're not channel operator\r\n";
	
	// check if client exists
	int target_fd = -1;
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second->getNickname() == target_nick)
		{
			target_fd = it->first;
			break;
		}
	}
	
	if (target_fd == -1)
		return ":localhost 401 " + clients[sender_fd]->getNickname() + " " + target_nick + " :No such nick\r\n";
	
	if (!channels[channel]->isMember(target_fd))
		return ":localhost 441 " + clients[sender_fd]->getNickname() + " " + target_nick + " " + channel + " :They aren't on that channel\r\n";
	
	// Add or remove operator privilege
	if (mode == "+o")
	{
		channels[channel]->addOperator(clients[target_fd]);
		std::string modeMsg = ":" + clients[sender_fd]->getNickname() + " MODE " + channel + " +o " + target_nick + "\r\n";
		channels[channel]->broadcastMessage(modeMsg, sender_fd);
	}
	else if (mode == "-o")
	{
		channels[channel]->removeOperator(target_fd);
		std::string modeMsg = ":" + clients[sender_fd]->getNickname() + " MODE " + channel + " -o " + target_nick + "\r\n";
		channels[channel]->broadcastMessage(modeMsg, sender_fd);
	}
	
	return "";
}

std::string Server::_mode(commandRequest& request, int sender_fd)
{
	if (!_clients[sender_fd]->isAuth())
		return ":localhost 451 * :You have not registered\r\n";
	
	if (request.args.size() < 2)
		return ":localhost 461 * :Invalid params\r\n";

	if (request.args[1] == "+o" || request.args[1] == "-o")
		return modeO(request, sender_fd, _clients, _channels);

	return ":localhost 501 * :Unknown MODE flag\r\n";
}
