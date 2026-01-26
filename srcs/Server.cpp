#include "../includes/Server.hpp"

void Server::handler(int signal)
{
	std::cout << "Server closed by signal: " << signal << std::endl;
}

Server::Server(int port, const std::string& password)
	: _serverFd(-1), _port(port), _password(password)
{
	setupSocket();
	struct pollfd pfd;
	pfd.fd = _serverFd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
}

Server::~Server()
{
	for (std::map<int, Client*>::iterator it = _clients.begin();
		it != _clients.end(); ++it)
	{
		close(it->first);
		delete it->second;
	}
	_clients.clear();
	if (_serverFd >= 0)
		close(_serverFd);
}

void Server::setupSocket(void)
{
	if(this->_port > 65535 || this->_port < 1024)
		throw std::invalid_argument("<port> must be between 1024 and 65535");
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd < 0)
		throw std::runtime_error("socket() failed");
    int opt = 1;
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("setsockopt() failed");
	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(this->_port);

	if (bind(_serverFd, (sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("bind() failed");

	if (listen(_serverFd, SOMAXCONN) < 0)
		throw std::runtime_error("listen() failed");

	if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl() failed");
}

void Server::run(void)
{
	signal(SIGINT, handler); //^C
	signal(SIGQUIT, handler); // "^\" ou ^|
	//signal(SIGTSTP, handler); // ^Z
	std::cout << "Server running in localhost port " << this->_port << std::endl;
	while (true)
	{
		if(poll(&_pollfds[0], _pollfds.size(), -1) == -1)
			break;
		for (size_t i = 0; i < _pollfds.size(); ++i)
		{
			if (_pollfds[i].revents & POLLIN)
			{
				if (_pollfds[i].fd == _serverFd)
					acceptClient();
				else
					receiveData(i);
			}
			if (_pollfds[i].revents & POLLOUT)
				sendData(i);
		}
	}
}

void Server::acceptClient(void)
{
	int clientFd = accept(_serverFd, NULL, NULL);
	if (clientFd < 0)
		return;;

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(clientFd);
		return;
	}
	struct pollfd pfd;
	pfd.fd = clientFd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
	_clients[clientFd] = new Client(clientFd);
	// std::cout << "New client connected: " << clientFd << std::endl;
	// _clients[clientFd]->sendBuffer += welcome();
	// enablePollout(clientFd);
}

void Server::receiveData(int indexFd)
{
    char buffer[6000];
    int clientFd = _pollfds[indexFd].fd;
    
    ssize_t bytes = recv(clientFd, buffer, sizeof(buffer), 0);

    if (bytes <= 0)
	{
        if (bytes < 0) std::cerr << "Erro no recv" << std::endl;
        removeClient(indexFd);
        return;
    }

    _clients[clientFd]->recvBuffer.append(buffer, bytes);

    size_t pos;
    while ((pos = _clients[clientFd]->recvBuffer.find("\n")) != std::string::npos) 
    {
        std::string command = _clients[clientFd]->recvBuffer.substr(0, pos);
        _clients[clientFd]->recvBuffer.erase(0, pos + 1);

        if (!command.empty() && command[command.size() - 1] == '\r')
            command.erase(command.size() - 1);

        if (command.empty()) continue;

        std::cout << "Executando comando do socket " << clientFd << ": " << command << std::endl;

        // O _parsing deve processar a lógica e retornar a string formatada da RFC 1459
        // Ex: Se recebeu "PING", retorna "PONG :servidor\r\n"
        std::string response = _parsing(command, clientFd); 

        if (!response.empty()) 
{
            _clients[clientFd]->sendBuffer.append(response);
            enablePollout(clientFd);
        }
    }
}

void Server::sendData(int indexFd)
{
    int clientFd = _pollfds[indexFd].fd;
    Client* client = _clients[clientFd];

    if (client->sendBuffer.empty()) {
        disablePollout(clientFd);
        return;
    }
    ssize_t bytes = send(clientFd, client->sendBuffer.c_str(), client->sendBuffer.size(), 0);
    if (bytes <= 0) {
        if (bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            std::cerr << "Erro fatal no send" << std::endl;
        }
        removeClient(indexFd);
        return;
    }
    client->sendBuffer.erase(0, bytes);
    if (client->sendBuffer.empty())
        disablePollout(clientFd);
}

void Server::removeClient(int indexFd)
{
	int clientFd = _pollfds[indexFd].fd;
	close(clientFd);
	_pollfds[indexFd];
	_pollfds.erase(_pollfds.begin() + indexFd);

	delete _clients[clientFd];
	_clients.erase(clientFd);
}

void Server::enablePollout(int fd)
{
    for (size_t i = 0; i < _pollfds.size(); ++i)
    {
        if (_pollfds[i].fd == fd)
        {
            _pollfds[i].events |= POLLOUT;
            break;
        }
    }
}

void Server::disablePollout(int fd)
{
    for (size_t i = 0; i < _pollfds.size(); ++i)
    {
        if (_pollfds[i].fd == fd)
        {
            _pollfds[i].events &= ~POLLOUT;
            break;
        }
    }
}

std::string	Server::welcome(void)
{
	std::string msg;
	msg.append("██╗    ██╗███████╗██╗      ██████╗ ██████╗ ███╗   ███╗███████╗\n");
	msg.append("██║    ██║██╔════╝██║     ██╔════╝██╔═══██╗████╗ ████║██╔════╝\n");
	msg.append("██║ █╗ ██║█████╗  ██║     ██║     ██║   ██║██╔████╔██║█████╗\n");
	msg.append("██║███╗██║██╔══╝  ██║     ██║     ██║   ██║██║╚██╔╝██║██╔══╝\n");
	msg.append("╚███╔███╔╝███████╗███████╗╚██████╗╚██████╔╝██║ ╚═╝ ██║███████╗\n");
	msg.append(" ╚══╝╚══╝ ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝\n");
	msg.append("You need to login so you can start chatting\r\n");
	return (msg);
};

std::string Server::_printHelpInfo(int sender_fd)
{
    std::string helpMsg;
    std::string nick = _clients[sender_fd]->getNickname();
    if (nick.empty())
        nick = "*";
    
    // RPL_INFO (371) para cada linha de ajuda
    helpMsg.append(":localhost 371 " + nick + " :=== Available IRC Commands ===\r\n");
    helpMsg.append(":localhost 371 " + nick + " :\r\n");
    helpMsg.append(":localhost 371 " + nick + " :Authentication & Setup:\r\n");
    helpMsg.append(":localhost 371 " + nick + " :  PASS <password>           - Authenticate with server password\r\n");
    helpMsg.append(":localhost 371 " + nick + " :  NICK <nickname>           - Set your nickname\r\n");
    helpMsg.append(":localhost 371 " + nick + " :  USER <user> <mode> <unused> :<realname> - Set user information\r\n");
    helpMsg.append(":localhost 371 " + nick + " :\r\n");
    helpMsg.append(":localhost 371 " + nick + " :Channel Operations:\r\n");
    helpMsg.append(":localhost 371 " + nick + " :  JOIN <#channel> [key]     - Join a channel (with optional password)\r\n");
    helpMsg.append(":localhost 371 " + nick + " :  PART <#channel> [reason]  - Leave a channel\r\n");
    helpMsg.append(":localhost 371 " + nick + " :  TOPIC <#channel> [topic]  - View or set channel topic\r\n");
    helpMsg.append(":localhost 371 " + nick + " :\r\n");
    helpMsg.append(":localhost 371 " + nick + " :Messaging:\r\n");
    helpMsg.append(":localhost 371 " + nick + " :  PRIVMSG <target> :<msg>   - Send private message to user or channel\r\n");
    helpMsg.append(":localhost 371 " + nick + " :\r\n");
    helpMsg.append(":localhost 371 " + nick + " :Operator Commands:\r\n");
    helpMsg.append(":localhost 371 " + nick + " :  KICK <#channel> <user> [reason] - Eject user from channel\r\n");
    helpMsg.append(":localhost 371 " + nick + " :  INVITE <user> <#channel>  - Invite user to channel\r\n");
    helpMsg.append(":localhost 371 " + nick + " :  MODE <#channel> <flags>   - Change channel mode:\r\n");
    helpMsg.append(":localhost 371 " + nick + " :    +i/-i  Invite-only channel\r\n");
    helpMsg.append(":localhost 371 " + nick + " :    +t/-t  Topic restricted to operators\r\n");
    helpMsg.append(":localhost 371 " + nick + " :    +k/-k <key>  Set/remove channel password\r\n");
    helpMsg.append(":localhost 371 " + nick + " :    +o/-o <user>  Give/take operator privilege\r\n");
    helpMsg.append(":localhost 371 " + nick + " :    +l/-l <limit>  Set/remove user limit\r\n");
    helpMsg.append(":localhost 371 " + nick + " :\r\n");
    helpMsg.append(":localhost 371 " + nick + " :Other:\r\n");
    helpMsg.append(":localhost 371 " + nick + " :  QUIT [message]            - Disconnect from server\r\n");
    helpMsg.append(":localhost 371 " + nick + " :  HELP                      - Display this help\r\n");
    // RPL_ENDOFINFO (374)
    helpMsg.append(":localhost 374 " + nick + " :End of INFO list\r\n");
    
    std::cout << "Help information sent to client: " << sender_fd << std::endl;
    return helpMsg;
}