/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   topic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 09:35:28 by lantonio          #+#    #+#             */
/*   Updated: 2026/02/04 12:08:23 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/irc.hpp"
#include "../../includes/Server.hpp"

std::string	Server::_topic(commandRequest& request, int sender_fd) {
	if (!_clients[sender_fd]->isAuth())
		return ":localhost 451 * :You have not registered\r\n";

	std::string	channel;
	if (request.args.size())
	{
		if (request.args[0][0] != '#')
			return ":localhost 461 * :First param must be the channel name\r\n";

		// ask to see channel topic
		if (request.args.size() == 1)
		{
			// channel exists and user is in it
			if (_channels.find(request.args[0]) != _channels.end() && _channels[request.args[0]]->isMember(sender_fd))
			{
				if (_channels[request.args[0]]->getHasTopic())
					return ":localhost 332 " + _clients[sender_fd]->getNickname() + " " + _channels[request.args[0]]->getName() + " :"  + _channels[request.args[0]]->getTopic() + "\r\n";
				else
					return ":localhost 331 " + _clients[sender_fd]->getNickname() + " " + request.args[0] + " :No topic is set\r\n";
			} else
				return ":localhost 403 * :Non-existent channel or user not in it\r\n";
		}

		if (request.args.size() == 2)
		{
			// channel exists and user is in it
			if (_channels.find(request.args[0]) != _channels.end() && _channels[request.args[0]]->isMember(sender_fd))
			{
				// if channel is in mode +t and user is not an operatorm return 482 
				_channels[request.args[0]]->setTopic(sender_fd, request.args[1]);
				
				std::string nick = _clients[sender_fd]->getNickname();
				std::string broadcastMsg = ":" + nick + " TOPIC " + request.args[0] + " :" + request.args[1] + "\r\n";

				std::map<int, Client*> const &members = _channels[request.args[0]]->getMembers();
				std::map<int, Client*>::const_iterator it;
				for (it = members.begin(); it != members.end(); ++it)
				{
					Client* member = it->second;
					if (member && member->getClientfd() != -1)
					{
						member->setSendBuffer(member->getSendBuffer() + broadcastMsg);
						enablePollout(member->getClientfd());
					}
				}
				
				return "";
			}
		}
	}
	return ":localhost 461 * :Not enounth params\r\n";
}
