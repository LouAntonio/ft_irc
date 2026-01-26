/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 13:31:12 by hmateque          #+#    #+#             */
/*   Updated: 2026/01/26 15:02:15 by hmateque         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <string>

class	Client
{
	public:
		int		fd;
		std::string	recvBuffer;
		std::string	sendBuffer;
		std::string	nickName;
		std::string	userName;
		bool hasPass;
		bool hasNick;
		bool hasUser;
		bool auth;

		Client(int fd);
		~Client();
		int getClientfd() const;
		std::string getNickname() const;
		std::string getUsername() const;
		void setNickname(const std::string& nick) { this->nickName = nick; }
		void setUsername(const std::string& user) { this->userName = user; }
};

#endif

