/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   privmsg.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 12:22:50 by hmateque          #+#    #+#             */
/*   Updated: 2026/01/30 14:20:46 by hmateque         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../../includes/Server.hpp"

std::string Server::_privmsg(commandRequest& request, int sender_fd)
{
    // 1. Verificar autenticação
    if (!_clients[sender_fd]->isAuth())
        return ":localhost 451 * :You have not registered\r\n";
    
    // 2. Verificar parâmetros (precisa de destino e mensagem)
    if (request.args.size() < 2)
        return ":localhost 461 " + _clients[sender_fd]->getNickname() + " PRIVMSG :Not enough parameters\r\n";
    
    std::string target = request.args[0];
    std::string message = request.args[1];
    
    // 3. Montar o prefixo da mensagem (quem enviou)
    std::string prefix = ":" + _clients[sender_fd]->getNickname() + "!" + 
                        _clients[sender_fd]->getUsername() + "@localhost";
    
    // 4. Verificar se o destino é um CANAL (começa com #)
    if (target[0] == '#')
    {
        // Verificar se o canal existe
        if (_channels.find(target) == _channels.end())
            return ":localhost 403 " + _clients[sender_fd]->getNickname() + " " + target + " :No such channel\r\n";
        
        // Verificar se o remetente está no canal
        if (!_channels[target]->isMember(sender_fd))
            return ":localhost 442 " + _clients[sender_fd]->getNickname() + " " + target + " :You're not on that channel\r\n";
        
        // Enviar para TODOS os membros do canal (exceto o remetente)
        std::map<int, Client*> members = _channels[target]->getMembers();
        std::string fullMsg = prefix + " PRIVMSG " + target + " :" + message + "\r\n";
        
        for (std::map<int, Client*>::iterator it = members.begin(); it != members.end(); ++it)
        {
            // Não envia para si mesmo
            if (it->first != sender_fd)
            {
                it->second->setSendBuffer(it->second->getSendBuffer() + fullMsg);
                enablePollout(it->first);
            }
        }
        
        return ""; // Sucesso (sem resposta ao remetente)
    }
    
    // 5. Se não é canal, é mensagem PRIVADA para usuário
    int recv_fd = -1;
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second->getNickname() == target)
        {
            recv_fd = it->first;
            break;
        }
    }
    
    // Verificar se o usuário existe
    if (recv_fd == -1)
        return ":localhost 401 " + _clients[sender_fd]->getNickname() + " " + target + " :No such nick/channel\r\n";
    
    // Enviar mensagem privada
    std::string fullMsg = prefix + " PRIVMSG " + target + " :" + message + "\r\n";
    _clients[recv_fd]->setSendBuffer(_clients[recv_fd]->getSendBuffer() + fullMsg);
    enablePollout(recv_fd);
    
    return ""; // Sucesso
}