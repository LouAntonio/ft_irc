/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lantonio <lantonio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 10:45:21 by hmateque          #+#    #+#             */
/*   Updated: 2026/02/04 14:21:59 by lantonio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Channel.hpp"

Channel::Channel(const std::string& channelName, Client* creator)
	: _name(channelName), _members(), _operators()
{
	_key = "";
	_topic = "";
	_hasTopic = false;
	_isOperatorsOnly = false;
	_hasKey = false;
	_hasLimit = false;
	_limit = 0;
	_operators.insert(std::pair<int, Client*>(creator->getClientfd(), creator));
	_members.insert(std::pair<int, Client*>(creator->getClientfd(), creator));
}

Channel::~Channel() {}

// Getters
std::string const &Channel::getName() const // retorna o nome do canal
{
	return _name;
}

std::map<int, Client *> const &Channel::getMembers() const // retorna os membros do canal
{
	return _members;
}

std::map<int, Client *> const &Channel::getOperators() const // retorna os operadores do canal
{
	return _operators;
}

std::string const &Channel::getChannelPassword() const // retorna a senha do canal
{
	return _channelPassword;
}

std::map<int, Client *> const &Channel::getBannedMembers() const // retorna os membros banidos do canal
{
	return _bannedMembers;
}

std::map<int, Client *> const &Channel::getInvitedMembers() const // retorna os membros convidados do canal
{
	return _invitedMembers;
}

bool Channel::getIsInviteOnly() const // retorna se o canal é invite-only
{
	return _isInviteOnly;
}

bool Channel::getHasPassword() const // retorna se o canal tem senha
{
	return _hasPassword;
}

bool    Channel::getHasTopic() const { // check if channel has a topic
	return this->_hasTopic;
}

bool	Channel::getIsOperatorsOnly() const { // check if channel is in mode +t
	return this->_isOperatorsOnly;
}

bool	Channel::getHasKey() const {
	return this->_hasKey;
}

bool	Channel::getHasLimit() const {
	return this->_hasLimit;
}

int		Channel::getLimit() const {
	return this->_limit;
}

std::string	Channel::getKey() const {
	return this->_key;
}

std::string Channel::getTopic() const {
	return this->_topic;
}

int Channel::addEnvited(Client* member)
{
	if (_invitedMembers.find(member->getClientfd()) == _invitedMembers.end())
	{
		_invitedMembers.insert(std::pair<int, Client *>(member->getClientfd(), member));
		return 1;
	}
	return -1;
}
void Channel::removeEnvited(int clientFd)
{
	_members.erase(clientFd);
}

// Member management
int Channel::addMember(Client* member) // adiciona um membro ao canal
{
	if (_members.find(member->getClientfd()) == _members.end())
	{
		_members.insert(std::pair<int, Client *>(member->getClientfd(), member));
		return USERISJOINED;
	}
	return USERALREADYJOINED;
}

int Channel::addOperator(Client* member) // adiciona um operador ao canal
{
	if (_members.find(member->getClientfd()) == _members.end())
		return USERNOTMEMBER;
		
	if (_operators.find(member->getClientfd()) != _operators.end())
		return USERALREADYJOINED;

	_operators.insert(std::pair<int, Client *>(member->getClientfd(), member));
	return USERISJOINED;
}

void Channel::removeMember(int clientFd) // remove um membro do canal
{
	_members.erase(clientFd);
	_operators.erase(clientFd);
}

void Channel::removeOperator(int clientFd) // remove um operador do canal
{
	_operators.erase(clientFd);
}

// Verification
bool Channel::isMember(int clientFd) const // verifica se o cliente é membro do canal
{
	return _members.find(clientFd) != _members.end();
}

bool Channel::isOperator(int clientFd) const // verifica se o cliente é operador do canal
{
	return _operators.find(clientFd) != _operators.end();
}

// setters
void Channel::setChannelPassword(const std::string& password) // seta a senha do canal
{
	_channelPassword = password;
}

void Channel::setInviteOnly(int member_id, std::string mode) // seta o canal como invite-only
{
	if (!isOperator(member_id))
		return;
	if (mode == "+i")
		_isInviteOnly = true;
	else
		_isInviteOnly = false;
}

void Channel::setHasPassword(void) // seta o canal como tendo senha
{
	 _hasPassword = true; 
}

void Channel::setBannedMember(Client* member) // adiciona um membro à lista de banidos
{
	if (_bannedMembers.find(member->getClientfd()) != _bannedMembers.end())
		return;
	if (!(isMember(member->getClientfd())))
		return;
	removeMember(member->getClientfd());
	_bannedMembers.insert(std::pair<int, Client *>(member->getClientfd(), member));

}

void    Channel::setTopic(int member_id, std::string topic) { //set a topic to the channel
	if (!(_members.find(member_id) != _members.end()))
	{
		std::cout << "Saindo" << std::endl;
		return;
	}
	if (topic.size() >= 512 || topic.size() <= 0)
	{
		std::cout << "Saindo 2" << std::endl;
		return;
	}
	
	// return if is in +t mode and member is not an operator

	this->_topic = topic;
	this->_hasTopic = true;
}

void	Channel::setIsOperatorsOnly(int member_id, std::string mode) {
	if (!isOperator(member_id))
		return;
	if (mode == "+o")
		_isOperatorsOnly = true;
	else
		_isOperatorsOnly = false;
}

void	Channel::setKey(int member_id, std::string mode, std::string key) {
	if (!isOperator(member_id))
		return;

	if (mode == "-k")
		_hasKey = false;
	else
	{
		_hasKey = true;
		_key = key;
	}
}

void	Channel::setLimit(int member_id, std::string mode, int limit) {
	if (!isOperator(member_id))
		return;

	if (mode == "-l")
		_hasKey = false;
	else
	{
		_hasKey = true;
		_limit = limit;
	}
}

void Channel::broadcastMessage(const std::string& message, int sender_fd) // Broadcast message to all members
{
	if (!(_members.find(sender_fd) != _members.end()))
		return;

	std::map<int, Client*>::const_iterator it;
	for (it = _members.begin(); it != _members.end(); ++it)
	{
		Client* member = it->second;
		if (member && member->getClientfd() != -1)
		{
			std::string currentBuffer = member->getSendBuffer();
			member->setSendBuffer(currentBuffer + message);
		}
	}
}

/*void Channel::setInvitedMember(Client* member) // adiciona um membro à lista de convidados
{
	if (_invitedMembers.find(member->getClientfd()) != _invitedMembers.end())
		return;
	
	if (isMember(member->getClientfd()))
		return;
	
	if (_bannedMembers.find(member->getClientfd()) != _bannedMembers.end())
		_bannedMembers.erase(member->getClientfd());
	_invitedMembers.insert(std::pair<int, Client *>(member->getClientfd(), member));
}*/



