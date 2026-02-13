
#include "../../includes/Server.hpp"
//:nick KICK #canal alvo :motivo


std::string Server::_kick(commandRequest& request, int fd)
{
    if (!_clients[fd]->isAuth())
        return ":localhost 451 * :You have not registered\r\n";

    if (request.args.size() < 2)
        return ":localhost 461 " + _clients[fd]->getNickname() + " KICK :Not enough parameters\r\n";

    std::string channel_name = request.args[0];
    std::string target_nick   = request.args[1];
    std::string reason        = (request.args.size() > 2) ? request.args[2] : "kicked";

    // 1. Verificar se o canal existe
    if (_channels.find(channel_name) == _channels.end())
        return ":localhost 403 " + _clients[fd]->getNickname() + " " + channel_name + " :No such channel\r\n";

    Channel* chan = _channels[channel_name];

    // 2. Verificar se o executor está no canal
    if (!chan->isMember(fd))
        return ":localhost 442 " + _clients[fd]->getNickname() + " " + channel_name + " :You're not on that channel\r\n";

    // 3. Verificar se o executor é operador do canal
    if (!chan->isOperator(fd))
        return ":localhost 482 " + _clients[fd]->getNickname() + " " + channel_name + " :You're not channel operator\r\n";
    
    // 4. Encontrar o FD do alvo pelo Nickname
    int target_fd = -1;
    std::map<int, Client*>::iterator it;
    for (it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getNickname() == target_nick) {
            target_fd = it->first;
            break;
        }
    }

    if (target_fd == -1 || !chan->isMember(target_fd))
        return ":localhost 441 " + _clients[fd]->getNickname() + " " + target_nick + " " + channel_name + " :They aren't on that channel\r\n";

    // 5. Montar mensagem de KICK no padrão RFC (Nick!User@Host)
    std::string kickMsg = ":" + _clients[fd]->getNickname() + "!~" + _clients[fd]->getUsername() + 
                          "@localhost KICK " + channel_name + " " + target_nick + " :" + reason + "\r\n";

    // 6. Broadcast apenas para membros do canal
    std::map<int, Client*> members = chan->getMembers();
    for (std::map<int, Client*>::iterator mit = members.begin(); mit != members.end(); ++mit) {
        mit->second->setSendBuffer(mit->second->getSendBuffer() + kickMsg);
        enablePollout(mit->first);
    }

    // 7. Efetuar a remoção
    chan->removeMember(target_fd);

    const std::map<int, Client*>& ops = chan->getOperators();
    if (ops.empty() && !members.empty())
    {
        chan->addOperator(members.begin()->second);
    }
    return "";
}
