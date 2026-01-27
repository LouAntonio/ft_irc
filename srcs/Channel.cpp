/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hmateque <hmateque@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 10:45:21 by hmateque          #+#    #+#             */
/*   Updated: 2026/01/27 15:45:43 by hmateque         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../includes/Channel.hpp"

Channel::Channel(const std::string& channelName, Client* creator)
    : _name(channelName), _members(), _operators()
{
    _operators.insert(std::pair<int, Client*>(creator->getClientfd(), creator));
    _members.insert(std::pair<int, Client*>(creator->getClientfd(), creator));
}

Channel::~Channel() {}

// Getters
std::string const &Channel::getName() const
{
    return _name;
}

std::map<int, Client *> const &Channel::getMembers() const
{
    return _members;
}

std::map<int, Client *> const &Channel::getOperators() const
{
    return _operators;
}

std::string const &Channel::getChannelPassword() const
{
    return _channelPassword;
}

std::map<int, Client *> const &Channel::getBannedMembers() const
{
    return _bannedMembers;
}

std::map<int, Client *> const &Channel::getInvitedMembers() const
{
    return _invitedMembers;
}

bool Channel::getIsInviteOnly() const
{
    return _isInviteOnly;
}

bool Channel::getHasPassword() const
{
    return _hasPassword;
}



// Member management
int Channel::addMember(Client* member)
{
    if (_members.find(member->getClientfd()) == _members.end())
    {
        _members.insert(std::pair<int, Client *>(member->getClientfd(), member));
        return USERISJOINED;
    }
    return USERALREADYJOINED;
}

int Channel::addOperator(Client* member)
{
    if (_members.find(member->getClientfd()) == _members.end())
        return USERNOTMEMBER;
        
    if (_operators.find(member->getClientfd()) != _operators.end())
        return USERALREADYJOINED;

    _operators.insert(std::pair<int, Client *>(member->getClientfd(), member));
    return USERISJOINED;
}

void Channel::removeMember(int clientFd)
{
    _members.erase(clientFd);
    _operators.erase(clientFd);
}

void Channel::removeOperator(int clientFd)
{
    _operators.erase(clientFd);
}

// Verification
bool Channel::isMember(int clientFd) const
{
    return _members.find(clientFd) != _members.end();
}

bool Channel::isOperator(int clientFd) const
{
    return _operators.find(clientFd) != _operators.end();
}

// setters
void Channel::setChannelPassword(const std::string& password)
{
    _channelPassword = password;
    _hasPassword = true;
}

void Channel::setInviteOnly(void)
{
     _isInviteOnly = true; 
}

void Channel::setHasPassword(void) { _hasPassword = true; }

