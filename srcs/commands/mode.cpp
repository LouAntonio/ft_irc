/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 11:43:50 by lantonio          #+#    #+#             */
/*   Updated: 2026/02/04 12:33:00 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.hpp"

static std::string toggleInviteOnly(commandRequest& request, int sender_fd, std::map<int, Client*>& clients, std::map<std::string, Channel*>& channels, std::string &modeMsg) {
	if (channels.find(request.args[0]) != channels.end())
	{
		if (!channels[request.args[0]]->isMember(sender_fd) || !channels[request.args[0]]->isOperator(sender_fd))
			return ":localhost 403 " + channels[request.args[0]]->getName() + " :You're not channel operator\r\n";

		channels[request.args[0]]->setIsOperatorsOnly(sender_fd, request.args[1]);
		modeMsg = ":" + clients[sender_fd]->getNickname() + " MODE " + channels[request.args[0]]->getName() + " " + request.args[1] + "\r\n";
		channels[request.args[0]]->broadcastMessage(modeMsg, sender_fd);
		return "";
	}
	return ":localhost 403 * :Non-existent channel\r\n";
}

static std::string toggleRestrictMode(commandRequest& request, int sender_fd, std::map<int, Client*>& clients, std::map<std::string, Channel*>& channels, std::string &modeMsg) {
	(void)request;
	(void)sender_fd;
	(void)clients;
	(void)channels;
	(void)modeMsg;
	return "";
}

static std::string toggleKey(commandRequest& request, int sender_fd, std::map<int, Client*>& clients, std::map<std::string, Channel*>& channels, std::string &modeMsg) {
	(void)request;
	(void)sender_fd;
	(void)clients;
	(void)channels;
	(void)modeMsg;
	return "";
}

static std::string toggleLimit(commandRequest& request, int sender_fd, std::map<int, Client*>& clients, std::map<std::string, Channel*>& channels, std::string &modeMsg) {
	(void)request;
	(void)sender_fd;
	(void)clients;
	(void)channels;
	(void)modeMsg;
	return "";
}

static std::string toggleOperator(commandRequest& request, int sender_fd, std::map<int, Client*>& clients, std::map<std::string, Channel*>& channels, std::string &modeMsg)
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
		modeMsg = ":" + clients[sender_fd]->getNickname() + " MODE " + channel + " +o " + target_nick + "\r\n";
		channels[channel]->broadcastMessage(modeMsg, sender_fd);
	}
	else if (mode == "-o")
	{
		channels[channel]->removeOperator(target_fd);
		modeMsg = ":" + clients[sender_fd]->getNickname() + " MODE " + channel + " -o " + target_nick + "\r\n";
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

	std::string	modeMsg;
	std::string errorMsg;
	
	if (request.args[1] == "+i" || request.args[1] == "-i")
		errorMsg = toggleInviteOnly(request, sender_fd, _clients, _channels, modeMsg);
	else if (request.args[1] == "+t" || request.args[1] == "-t")
		errorMsg = toggleRestrictMode(request, sender_fd, _clients, _channels, modeMsg);
	else if (request.args[1] == "+k" || request.args[1] == "-k")
		errorMsg = toggleKey(request, sender_fd, _clients, _channels, modeMsg);
	else if (request.args[1] == "+o" || request.args[1] == "-o")
		errorMsg = toggleOperator(request, sender_fd, _clients, _channels, modeMsg);
	else if (request.args[1] == "+l" || request.args[1] == "-l")
		errorMsg = toggleLimit(request, sender_fd, _clients, _channels, modeMsg);
	else
		return ":localhost 501 * :Unknown MODE flag " + request.args[1] + "\r\n";
	
	if (!errorMsg.empty())
		return errorMsg;

	if (!modeMsg.empty()) {
		std::map<int, Client*> const &members = _channels[request.args[0]]->getMembers();
		std::map<int, Client*>::const_iterator it;
		for (it = members.begin(); it != members.end(); ++it)
		{
			Client* member = it->second;
			if (member && member->getClientfd() != -1)
			{
				member->setSendBuffer(member->getSendBuffer());
				enablePollout(member->getClientfd());
			}
		}
		return "";
	}
	return":localhost 501 * :Unknown MODE flag" + request.args[1] + " \r\n";
}
