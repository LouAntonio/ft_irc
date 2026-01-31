/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   user.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 12:19:26 by hmateque          #+#    #+#             */
/*   Updated: 2026/01/30 12:19:44 by hmateque         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../../includes/Server.hpp"

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