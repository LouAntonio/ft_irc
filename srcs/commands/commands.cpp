/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/24 12:03:08 by hmateque          #+#    #+#             */
/*   Updated: 2026/01/26 15:09:40 by hmateque         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../../includes/Server.hpp"

std::string	Server::_parsing(const std::string& msg, int sender_fd)
{
	commandRequest	request(_splitRequest(msg));

    for (size_t i = 0; i < request.command.length(); ++i)
        request.command[i] = std::toupper(request.command[i]);
    // comandos a serem implementados
    /*
    PASS
    NICK
    USER
    JOIN
    PART
    TOPIC
    PRIVMSG
    MODE
    INVITE
    KICK
    HELP
    QUIT
    */
    if (request.invalidMessage)
	    return ("Invalid message!\r\n");
    else if (request.command == "CAP")
        return ""; // Apenas ignore o CAP LS por enquanto
    else if (request.command == "PASS")
        return (_setPassWord(request, sender_fd));
    else if (request.command == "PING")
        return (_pingPong(request, sender_fd));
	else if (request.command == "NICK")
		return (_setNickName(request, sender_fd));
    else if (request.command == "HELP" || request.command == "H")
		return (_printHelpInfo(sender_fd));
	else
		return ("Invalid command\r\n");
};

commandRequest Server::_splitRequest(const std::string& req)
{
    commandRequest request;
    request.invalidMessage = false;
    size_t i = 0;
    size_t j = 0;

    if (req[i] == ' ' || !req[i]) {
        request.invalidMessage = true;
        return (request);
    }
    j = i;
    while (req[i])
    {
        if (req[i] == ' ')
        {
            if (req[i + 1] == ' ') {
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
            if (req[i - 1] != ' ') {
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
    return response;
}

std::string Server::_setPassWord(commandRequest& request, int fd) 
{
    if (_clients[fd]->auth) return "";

    if (request.args.empty())
        return ":localhost 461 * PASS :Not enough parameters\r\n";

    if (request.args[0] != _password)
        return ":localhost 464 * :Password incorrect\r\n";

    _clients[fd]->hasPass = true;
    return "";
}

std::string Server::_setUserName(commandRequest& request, int fd) 
{
    if (_clients[fd]->auth) return ""; 

    if (request.args.size() < 4)
        return ":localhost 461 * USER :Not enough parameters\r\n";

    _clients[fd]->setUsername(request.args[0]);
    _clients[fd]->hasUser = true;

    return attemptRegistration(fd);
}

std::string Server::attemptRegistration(int fd) 
{
    if (_clients[fd]->hasPass && _clients[fd]->hasNick && _clients[fd]->hasUser && !_clients[fd]->auth) 
    {
        _clients[fd]->auth = true;
        std::string nick = _clients[fd]->getNickname();
        std::string welcome;
        welcome += ":localhost 001 " + nick + " :Welcome to the IRC server, " + nick + "\r\n";
        welcome += welcome();
        return welcome;
    }
    return "";
}