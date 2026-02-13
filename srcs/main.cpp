/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parmando <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/13 08:45:47 by parmando          #+#    #+#             */
/*   Updated: 2026/02/13 08:45:56 by parmando         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include <iostream>
#include <cstdlib>
static bool ft_check(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Usage: " << av[0] << " <port> <password>" << std::endl;
		return false;
	}
	for(int i = 0; av[1][i]; i++)
	{
		if(!std::isdigit(av[1][i]))
		{
			std::cerr << "Error: only numbers can be used for the <port> argument" << std::endl;
			return false;
		}
	}
	std::string password = av[2];
	if(password.empty() 
		|| password.find(' ') != std::string::npos
		|| password.find('\t') != std::string::npos)
	{
		std::cerr << "Error: the server password is wrong" << std::endl;
		return false;
	}
	return true;	
}

int main(int ac, char **av)
{
	if (ft_check(ac, av))
	{
		int port = std::atoi(av[1]);
		std::string password = av[2];
		try
		{
			Server server(port, password);
			server.run();
			return 0;
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
			return 1;
		}
	}
	return 1;
}
