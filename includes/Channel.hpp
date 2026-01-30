/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 10:15:54 by hmateque          #+#    #+#             */
/*   Updated: 2026/01/30 12:36:38 by hmateque         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/


#ifndef CHANNEL_HPP
#define CHANNEL_HPP


#define USERALREADYJOINED 0
#define USERISJOINED 1
#define USERNOTMEMBER -1

#include "Server.hpp"
#include "Client.hpp"

class Channel
{
    private:
        std::string _name;
        std::string _channelPassword;
        std::map<int, Client *> _bannedMembers;
        std::map<int, Client *> _invitedMembers;
        std::map<int, Client *> _members;
        std::map<int, Client *> _operators;
        bool _isInviteOnly;
        bool _hasPassword;

    public:
        Channel(const std::string& channelName, Client* creator);
        ~Channel();

        // Getters
        std::string const &getName() const;
        std::map<int, Client *> const &getMembers() const;
        std::map<int, Client *> const &getOperators() const;
        std::string const &getChannelPassword() const;
        std::map<int, Client *> const &getBannedMembers() const;
        std::map<int, Client *> const &getInvitedMembers() const;
        bool getIsInviteOnly() const;
        bool getHasPassword() const;

        // Member management
        int addMember(Client* member);
        int addOperator(Client* member);
        void removeMember(int clientFd);
        void removeOperator(int clientFd);

        //setters
        void setChannelPassword(const std::string& password);
        void setInviteOnly(void);
        void setHasPassword(void);
        void setBannedMember(Client* member);
        void setInvitedMember(Client* member);

        // Verification
        bool isMember(int clientFd) const;
        bool isOperator(int clientFd) const;

        // Broadcast message to all members
        void broadcastMessage(const std::string& message, int senderFd);

};

#endif