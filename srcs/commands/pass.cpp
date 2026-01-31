/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pass.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 12:22:15 by hmateque          #+#    #+#             */
/*   Updated: 2026/01/30 12:22:21 by hmateque         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../../includes/Server.hpp"

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