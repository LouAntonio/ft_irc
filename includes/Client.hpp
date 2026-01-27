/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 13:31:12 by hmateque          #+#    #+#             */
/*   Updated: 2026/01/27 09:06:04 by hmateque         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include "Command.hpp"

class	Client
{
	private:
		int			fd;
		std::string	recvBuffer;
		std::string	sendBuffer;
		std::string	nickName;
		std::string	userName;
		bool 		hasPass;
		bool 		hasNick;
		bool 		hasUser;
		bool 		auth;
	public:
		// Constructor and Destructor
		Client(int fd);
		~Client();

		// Getters
		int getClientfd() const;
		std::string getRecvBuffer() const;
		std::string getSendBuffer() const;
		std::string getNickname() const;
		std::string getUsername() const;
		bool getHasPass() const;
		bool getHasNick() const;
		bool getHasUser() const;
		bool isAuth() const;

		// Setters
		void setClientfd(int fd);
		void setRecvBuffer(const std::string& buffer);
		void setSendBuffer(const std::string& buffer);
		void setNickname(const std::string& nick);
		void setUsername(const std::string& user);
		void setHasPass(bool hasPass);
		void setHasNick(bool hasNick);
		void setHasUser(bool hasUser);
		void setAuth(bool auth);
};

#endif
