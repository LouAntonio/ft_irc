
#include "../../includes/Server.hpp"
//:nick KICK #canal alvo :motivo


std::string Server::_kick(commandRequest& request, int fd)
{
    if (!_clients[fd]->isAuth())
        return ":localhost 451 * :You have not registered\r\n";

    if (request.args.size() < 2)
        return ":localhost 461 * KICK :Not enough parameters\r\n";

    std::string channel   = request.args[0];
    std::string nick_name = request.args[1];

    if (_channels.find(channel) == _channels.end())
        return ":localhost 403 " + _clients[fd]->getNickname() + " " + channel + " :No such channel\r\n";

    if (!_channels[channel]->isMember(fd))
        return ":localhost 442 " + _clients[fd]->getNickname() + " " + channel + " :You're not on that channel\r\n";

    if (!_channels[channel]->isOperator(fd))
        return ":localhost 482 " + _clients[fd]->getNickname() + " " + channel + " :You're not operator\r\n";

    int kick_fd = -1;

    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second->getNickname() == nick_name)
        {
            kick_fd = it->first;
            break;
        }
    }

    if (kick_fd == -1)
        return ":localhost 401 " + _clients[fd]->getNickname() + " " + nick_name + " :No such nick\r\n";

    if (!_channels[channel]->isMember(kick_fd))
        return ":localhost 441 " + _clients[fd]->getNickname() + " " + nick_name + " " + channel + " :They aren't on that channel\r\n";

    std::string reason = (request.args.size() > 2) ? request.args[2] : "kicked";
    std::string kickMsg = ":" + _clients[fd]->getNickname() + " KICK " + channel + " " + nick_name + " :" + reason + "\r\n";

    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        int member_fd = it->first;

        if (_channels[channel]->isMember(member_fd))
        {
            _clients[member_fd]->setSendBuffer(
                _clients[member_fd]->getSendBuffer() + kickMsg);
            enablePollout(member_fd);
        }
    }

    _channels[channel]->removeMember(kick_fd);

    return "";
}
