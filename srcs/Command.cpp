/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 11:03:43 by lantonio          #+#    #+#             */
/*   Updated: 2026/01/26 13:46:24 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Command.hpp"

Command::Command() {
	prefix = "";
	command = "";
	params = std::vector<std::string>(0);
	trailing = "";
}

std::vector<std::string> Command::input_builder(std::string &bufferAcumulado, char *newBuffer, int bytesRead)
{
    std::vector<std::string> comandos;

    bufferAcumulado.append(newBuffer, bytesRead);

    size_t pos;
    while ((pos = bufferAcumulado.find("\r\n")) != std::string::npos)
    {
        std::string cmd = bufferAcumulado.substr(0, pos);
        comandos.push_back(cmd);
        bufferAcumulado.erase(0, pos + 2);
    }

    return comandos;
}


Command::~Command() {}
