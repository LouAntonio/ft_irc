/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/24 12:03:08 by hmateque          #+#    #+#             */
/*   Updated: 2026/01/26 13:56:20 by hmateque         ###   ########.fr       */
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