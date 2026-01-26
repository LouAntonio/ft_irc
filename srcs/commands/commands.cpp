/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/24 12:03:08 by hmateque          #+#    #+#             */
/*   Updated: 2026/01/24 14:57:25 by hmateque         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../../includes/Server.hpp"

std::string	Server::_parsing(const std::string& msg, int sender_fd)
{
	commandRequest	request(_splitRequest(msg));

    for (size_t i = 0; i < request.command.length(); ++i)
        request.command[i] = std::toupper(request.command[i]);
    if (request.invalidMessage)
	    return ("Invalid message!\r\n");
	if (request.command == "PASS")
		return ("_setPassWord(request, sender_fd)");
	else if (request.command == "NICK")
		return ("_setNickName(request, sender_fd)");
	else if (request.command == "USER")
		return ("_setUserName(request, sender_fd)");
	else if (request.command == "JOIN")
		return ("_joinChannel(request, sender_fd)");
    else if (request.command == "PART")
		return ("_part(request, sender_fd)");
	else if (request.command == "TOPIC")
		return ("_topic(request, sender_fd)");
    else if (request.command == "PRIVMSG")
        return ("_privmsg(request, sender_fd)");
	else if (request.command == "MODE")
		return ("_setMode(request, sender_fd)");
	else if (request.command == "INVITE")
		return ("_invite(request, sender_fd)");
	else if (request.command == "KICK")
		return ("_kick(request, sender_fd)");
    else if (request.command == "HELP" || request.command == "H")
		return (_printHelpInfo(sender_fd));
	else if (request.command == "QUIT")
		return ("_quit(request, sender_fd)");
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