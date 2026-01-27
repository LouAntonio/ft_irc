/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/24 12:03:08 by hmateque          #+#    #+#             */
/*   Updated: 2026/01/27 14:40:24 by hmateque         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../../includes/Server.hpp"

std::string Server::_parsing(const std::string& msg, int sender_fd) {
    commandRequest request(_splitRequest(msg));
    if (request.command.empty()) return "";

    for (size_t i = 0; i < request.command.length(); ++i)
        request.command[i] = std::toupper(request.command[i]);
    printRequest(msg, request);
    if (request.invalidMessage)
        return ("Invalid message!\r\n");

    if (request.command == "CAP") return ""; 
    if (request.command == "PASS") return _setPassWord(request, sender_fd);
    if (request.command == "NICK") return _setNickName(request, sender_fd);
    if (request.command == "USER") return _setUserName(request, sender_fd);
    if (request.command == "PING") return _pingPong(request, sender_fd);
    if (request.command == "JOIN") return _joinChannel(request, sender_fd);
    if (request.command == "HELP" || request.command == "H") return _printHelpInfo(sender_fd);

    // Resposta padrão RFC para comando desconhecido (421)
    std::string nick = _clients[sender_fd]->getNickname().empty() ? "*" : _clients[sender_fd]->getNickname();
    return ":localhost 421 " + nick + " " + request.command + " :Unknown command\r\n";
}

commandRequest Server::_splitRequest(const std::string& req)
{
    commandRequest request;
    request.invalidMessage = false;
    size_t i = 0;
    size_t j = 0;

    if (req[i] == ' ' || !req[i]) 
    {
        request.invalidMessage = true;
        return (request);
    }
    j = i;
    while (req[i])
    {
        if (req[i] == ' ')
        {
            if (req[i + 1] == ' ') 
            {
                request.invalidMessage = true;
                return (request);
            }
            request.args.push_back(req.substr(j, i - j));
            while (req[i] == ' ')
                i++;
            j = i;
        }
        if (req[i] == ':')
        {
            if (req[i - 1] != ' ') 
            {
                request.invalidMessage = true;
                return (request);
            }
            request.args.push_back(req.substr(i + 1, req.length() - i));
            request.command = request.args[0];
            request.args.erase(request.args.begin());
            return (request);
        }
        i++;
    }

    if (i && req[j])
        request.args.push_back(req.substr(j, i - j));
    request.command = request.args[0];
    request.args.erase(request.args.begin());
    return (request);
}

void Server::printRequest(const std::string& input, const commandRequest& req)
{
    std::cout << CYAN << "========================================" << RESET << std::endl;
    std::cout << YELLOW << "Input: " << RESET << "\"" << input << "\"" << std::endl;
    
    if (req.invalidMessage) {
        std::cout << RED << "❌ MENSAGEM INVÁLIDA!" << RESET << std::endl;
        return;
    }
    
    std::cout << GREEN << "✓ Válida" << RESET << std::endl;
    std::cout << BLUE << "Comando: " << RESET << "\"" << req.command << "\"" << std::endl;
    std::cout << MAGENTA << "Argumentos (" << req.args.size() << "):" << RESET << std::endl;
    
    for (size_t i = 0; i < req.args.size(); i++) {
        std::cout << "  [" << i << "] = \"" << req.args[i] << "\"" << std::endl;
    }
    std::cout << std::endl;
}

std::string Server::_pingPong(commandRequest& request, int sender_fd)
{
    std::string response;
    (void)sender_fd;
    if (request.args.empty())
    {
        response = ":localhost 409 * :No origin specified\r\n"; // ERR_NOORIGIN (409)
    }
    else
    {
        response = ":localhost PONG :" + request.args[0] + "\r\n";
    }
    std::cout << "DEBUG: Responding to PING with PONG " << request.args[0] << std::endl;
    return response;
}

std::string Server::_setPassWord(commandRequest& request, int fd) 
{
    if (_clients[fd]->isAuth()) return "";

    if (request.args.empty())
        return ":localhost 461 * PASS :Not enough parameters\r\n";

    if (request.args[0] != _password)
        return ":localhost 464 * :Password incorrect\r\n";

    _clients[fd]->setHasPass(true);
    return "";
}

std::string Server::_setUserName(commandRequest& request, int fd) 
{
    if (_clients[fd]->isAuth()) return ""; 

    if (!_clients[fd]->getHasPass())
        return ":localhost 464 * :Password required. Send PASS first.\r\n";

    if (request.args.size() < 4)
        return ":localhost 461 * USER :Not enough parameters\r\n";

    _clients[fd]->setUsername(request.args[0]);
    _clients[fd]->setHasUser(true);

    return attemptRegistration(fd);
}



std::string Server::attemptRegistration(int fd) 
{
    Client *c = _clients[fd];

    if (c->getHasPass() && c->getHasNick() && c->getHasUser() && !c->isAuth()) 
    {
        c->setAuth(true);
        std::string nick = c->getNickname();
        
        std::string res;
        // RPL_WELCOME (001) - O Irssi exige isso para não desconectar
        res += ":localhost 001 " + nick + " :Welcome to the IRC Network, " + nick + "\r\n";
        
        // Agora sim você envia o welcome banner
        res += welcome();
        
        std::cout << "DEBUG: Cliente " << fd << " registrado como " << nick << std::endl;
        return res;
    }
    return "";
}


std::string Server::_joinChannel(commandRequest& request, int fd)
{
    if (!_clients[fd]->isAuth())
        return ":localhost 464 * :You need to register first.\r\n";

    if (request.args.empty())
        return ":localhost 461 * JOIN :Not enough parameters\r\n";

    std::string channelName = request.args[0];

    if (!_channels[channelName])
    {
        _channels[channelName] = new Channel(channelName, _clients[fd]);
        return ":localhost 331 " + _clients[fd]->getNickname() + " " + channelName + " :No topic is set\r\n";
    }
    else if (_channels[channelName]->isMember(fd))
    {
        return ":localhost 443 " + _clients[fd]->getNickname() + " " + channelName + " :You're already on that channel\r\n";
    }

    _channels[channelName]->addMember(_clients[fd]);
    return ":localhost 331 " + _clients[fd]->getNickname() + " " + channelName + " :No topic is set\r\n";
}