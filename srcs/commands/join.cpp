/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 12:17:26 by hmateque          #+#    #+#             */
/*   Updated: 2026/02/12 16:21:11 by hmateque         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../../includes/Server.hpp"


bool Server::isValidChannelName(const std::string& name)
{
    if (name.empty() || name[0] != '#')
        return false;
    for (size_t i = 1; i < name.length(); ++i)
    {
        if (!isalnum(name[i]) && name[i] != '_' && name[i] != '-')
            return false;
    }
    return true;
}

std::string Server::_joinChannel(commandRequest& request, int fd)
{
    if (!_clients[fd]->isAuth())
        return ":localhost 464 * :You need to register first.\r\n";

    if (request.args.empty())
        return ":localhost 461 * JOIN :Not enough parameters\r\n";
    
    std::string channelName = request.args[0];
    if (!isValidChannelName(channelName))
        return ":localhost 403 * " + channelName + " :Invalid channel name\r\n";
    
    std::string password = (request.args.size() > 1) ? request.args[1] : "";
    std::string nick = _clients[fd]->getNickname();
    std::string user = _clients[fd]->getUsername();
    std::string prefix = ":" + nick + "!" + user + "@localhost";

    if (!_channels[channelName])
    {
        _channels[channelName] = new Channel(channelName, _clients[fd]);
        
        if (!password.empty())
        {
            _channels[channelName]->setChannelPassword(password);
            _channels[channelName]->setHasPassword();
        }

        // Resposta JOIN para o criador
        std::string response;
        response += prefix + " JOIN " + channelName + "\r\n";
        response += ":localhost 331 " + nick + " " + channelName + " :No topic is set\r\n";
        response += ":localhost 353 " + nick + " = " + channelName + " :@" + nick + "\r\n";
        response += ":localhost 366 " + nick + " " + channelName + " :End of /NAMES list\r\n";
        return response;
    }

    if (_channels[channelName]->isMember(fd))
        return ":localhost 443 " + nick + " " + channelName + " :You're already on that channel\r\n";

    // Verifica se o canal é invite-only
    if (_channels[channelName]->getIsInviteOnly())
    {
        if (_channels[channelName]->getInvitedMembers().find(fd) == _channels[channelName]->getInvitedMembers().end())
            return ":localhost 473 " + nick + " " + channelName + " :Cannot join channel (+i)\r\n";
    }

    if (_channels[channelName]->getHasPassword())
    {
        if (password.empty())
            return ":localhost 475 " + nick + " " + channelName + " :Cannot join channel (+k) - bad key\r\n";
        
        if (password != _channels[channelName]->getChannelPassword())
            return ":localhost 475 " + nick + " " + channelName + " :Cannot join channel (+k) - bad key\r\n";
    }

    // Verificar key do canal (definida por MODE +k)
    if (_channels[channelName]->getHasKey())
    {
        if (password.empty())
            return ":localhost 475 " + nick + " " + channelName + " :Cannot join channel (+k) - bad key\r\n";
        
        if (password != _channels[channelName]->getKey())
            return ":localhost 475 " + nick + " " + channelName + " :Cannot join channel (+k) - bad key\r\n";
    }

    // Verificar limite de usuários
    if (_channels[channelName]->getHasLimit())
    {
        if ((int)_channels[channelName]->getMembers().size() >= _channels[channelName]->getLimit())
            return ":localhost 471 " + nick + " " + channelName + " :Cannot join channel (+l)\r\n";
    }

    // remover o convite se existir
    _channels[channelName]->removeEnvited(fd);

    _channels[channelName]->addMember(_clients[fd]);

    // 1. Notificar TODOS os membros do canal sobre o JOIN (exceto o novo membro, que recebe via response)
    std::string joinMsg = prefix + " JOIN " + channelName + "\r\n";
    _channels[channelName]->broadcastMessage(joinMsg, fd); // fd = não enviar para o novo membro

    // Ativar POLLOUT para todos os outros membros que receberam o broadcast
    std::map<int, Client*> const &allMembers = _channels[channelName]->getMembers();
    for (std::map<int, Client*>::const_iterator it = allMembers.begin(); it != allMembers.end(); ++it)
    {
        if (it->first != fd)
            enablePollout(it->first);
    }

    // 2. Montar resposta para o novo membro (inclui a mensagem JOIN para ele próprio)
    std::string response;
    response += prefix + " JOIN " + channelName + "\r\n";

    // Enviar tópico se existir
    if (_channels[channelName]->getHasTopic() && !_channels[channelName]->getTopic().empty())
        response += ":localhost 332 " + nick + " " + channelName + " :" + _channels[channelName]->getTopic() + "\r\n";
    else
        response += ":localhost 331 " + nick + " " + channelName + " :No topic is set\r\n";

    // 3. Enviar lista de nomes (RPL_NAMREPLY + RPL_ENDOFNAMES)
    std::string namesList;
    std::map<int, Client*> const &members = _channels[channelName]->getMembers();
    for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        if (!namesList.empty())
            namesList += " ";
        if (_channels[channelName]->isOperator(it->first))
            namesList += "@";
        namesList += it->second->getNickname();
    }
    response += ":localhost 353 " + nick + " = " + channelName + " :" + namesList + "\r\n";
    response += ":localhost 366 " + nick + " " + channelName + " :End of /NAMES list\r\n";

    return response;
}