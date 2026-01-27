/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 11:04:10 by lantonio          #+#    #+#             */
/*   Updated: 2026/01/26 13:45:33 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "irc.hpp"

class Command {
	private:
		std::string					prefix;
		std::string					command;
		std::vector<std::string>	params;
		std::string					trailing;

	public:
		Command();
		Command(std::string prefix, std::string command, std::vector<std::string> params, std::string trailing);
		~Command();

		std::vector<std::string> input_builder(std::string &bufferAcumulado, char *newBuffer, int bytesRead);
		
};

#endif
