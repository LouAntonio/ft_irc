/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   topic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 09:35:28 by lantonio          #+#    #+#             */
/*   Updated: 2026/02/03 10:14:36 by lantonio         ###   ########.fr       */
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
			return ":localhost 461 * :Not enounth params\r\n";
		if (request.args.size() == 1)
		{
			// channel exists and user is in it
			if (_channels.find(request.args[0]) != _channels.end() && _channels[request.args[0]]->isMember(sender_fd))
				return ":localhost 332 * :No topic setted to channel\r\n";
		}
	}
	return ":localhost 461 * :Not enounth params\r\n";
}
