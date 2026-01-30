/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 12:17:26 by hmateque          #+#    #+#             */
/*   Updated: 2026/01/30 12:53:09 by hmateque         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../../includes/Server.hpp"

std::string Server::_joinChannel(commandRequest& request, int fd)
{
    if (!_clients[fd]->isAuth())
        return ":localhost 464 * :You need to register first.\r\n";

    if (request.args.empty())
        return ":localhost 461 * JOIN :Not enough parameters\r\n";

    std::string channelName = request.args[0];
    std::string password = (request.args.size() > 1) ? request.args[1] : "";

    if (!_channels[channelName])
    {
        _channels[channelName] = new Channel(channelName, _clients[fd]);
        
        if (!password.empty())
        {
            _channels[channelName]->setChannelPassword(password);
            _channels[channelName]->setHasPassword();
        }
        return ":localhost 331 " + _clients[fd]->getNickname() + " " + channelName + " :No topic is set\r\n";
    }

    if (_channels[channelName]->isMember(fd))
        return ":localhost 443 " + _clients[fd]->getNickname() + " " + channelName + " :You're already on that channel\r\n";

    // Verifica se o canal é invite-only
    if (_channels[channelName]->getIsInviteOnly())
    {
        if (_channels[channelName]->getInvitedMembers().find(fd) == _channels[channelName]->getInvitedMembers().end())
            return ":localhost 473 " + _clients[fd]->getNickname() + " " + channelName + " :Cannot join channel (+i)\r\n";
    }

    if (_channels[channelName]->getHasPassword())
    {
        if (password.empty())
            return ":localhost 475 " + _clients[fd]->getNickname() + " " + channelName + " :Cannot join channel (+k) - bad key\r\n";
        
        if (password != _channels[channelName]->getChannelPassword())
            return ":localhost 475 " + _clients[fd]->getNickname() + " " + channelName + " :Cannot join channel (+k) - bad key\r\n";
    }

    _channels[channelName]->addMember(_clients[fd]);

    return ":localhost 331 " + _clients[fd]->getNickname() + " " + channelName + " :No topic is set\r\n";
}