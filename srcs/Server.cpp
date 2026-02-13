#include "../includes/Server.hpp"
#include "../includes/Client.hpp"

// Variável global para controle de sinal (signal-safe)
static volatile sig_atomic_t g_running = 1;

void Server::handler(int signal)
{
	(void)signal;
	g_running = 0;
}

Server::Server(int port, const std::string& password) : _serverFd(-1), _port(port), _password(password)
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
	// Deletar todos os clientes
	for (std::map<int, Client*>::iterator it = _clients.begin();
		it != _clients.end(); ++it)
	{
		close(it->first);
		delete it->second;
	}
	_clients.clear();
	
	// Deletar todos os canais
	for (std::map<std::string, Channel*>::iterator it = _channels.begin();
		it != _channels.end(); ++it)
	{
		delete it->second;
	}
	_channels.clear();
	
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
    signal(SIGINT, handler);
    signal(SIGQUIT, handler);
    signal(SIGPIPE, SIG_IGN); 
    std::cout << "Server running in localhost port " << this->_port << std::endl;
    while (g_running)
    {
        if(poll(&_pollfds[0], _pollfds.size(), 1000) == -1)
        {
            if (!g_running)
                break;
            continue;
        }
        
        // Coletar eventos ANTES de processar (evita problemas com índices mudando)
        std::vector<std::pair<int, short> > events; // fd, revents
        for (size_t i = 0; i < _pollfds.size(); ++i)
        {
            if (_pollfds[i].revents != 0)
                events.push_back(std::make_pair(_pollfds[i].fd, _pollfds[i].revents));
        }
        
        // Processar eventos usando FDs (não índices)
        for (size_t i = 0; i < events.size(); ++i)
        {
            int fd = events[i].first;
            short revents = events[i].second;
            
            // Processar POLLIN
            if (revents & POLLIN)
            {
                if (fd == _serverFd)
                {
                    acceptClient();
                }
                else
                {
                    // Verificar se o cliente ainda existe
                    if (_clients.find(fd) != _clients.end())
                    {
                        // Encontrar o índice atual do fd
                        for (size_t j = 0; j < _pollfds.size(); ++j)
                        {
                            if (_pollfds[j].fd == fd)
                            {
                                receiveData(j);
                                break;
                            }
                        }
                    }
                }
            }
            
            // Processar POLLOUT - verificar novamente se cliente existe
            if ((revents & POLLOUT) && _clients.find(fd) != _clients.end())
            {
                for (size_t j = 0; j < _pollfds.size(); ++j)
                {
                    if (_pollfds[j].fd == fd)
                    {
                        sendData(j);
                        break;
                    }
                }
            }
        }
    }
    std::cout << "Server shutting down..." << std::endl;
}

void Server::acceptClient(void)
{
	int clientFd = accept(_serverFd, NULL, NULL);
	if (clientFd < 0)
		return;

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
	std::cout << "New client connected: " << clientFd << std::endl;
}

std::vector<std::string> Server::_input_builder(Client* client, char *newBuffer, int bytesRead)
{
	std::vector<std::string> comandos;

	// Obter o buffer acumulado do cliente
	std::string bufferAcumulado = client->getRecvBuffer();
	
	// Adicionar novos dados ao buffer
	bufferAcumulado.append(newBuffer, bytesRead);

	// Extrair comandos completos (terminados com \r\n ou \n)
	size_t pos;
	while ((pos = bufferAcumulado.find("\n")) != std::string::npos)
	{
		std::string cmd = bufferAcumulado.substr(0, pos);
		// Remove trailing \r if present (handles both \r\n and \n)
		if (!cmd.empty() && cmd[cmd.size() - 1] == '\r')
			cmd.erase(cmd.size() - 1);
		comandos.push_back(cmd);
		bufferAcumulado.erase(0, pos + 1);
	}

	// Atualizar o buffer do cliente com os dados restantes
	client->setRecvBuffer(bufferAcumulado);

	return comandos;
}

void Server::receiveData(int indexFd)
{
	char        buffer[6000];
	int         fd =_pollfds[indexFd].fd;
	Client      *client = _clients[fd];
	
	ssize_t     bytes = recv(fd, buffer, sizeof(buffer), 0);

	if (bytes <= 0)
	{
		if (bytes < 0)
			std::cout << "Error in recv()" << std::endl;
		else
			std::cout << "Client disconnected: " << fd << std::endl;
		removeClient(indexFd);
		return;
	}

	//std::cout << "Buffer received from client " << fd << ": " << std::string(buffer, bytes) << std::endl;

	std::vector<std::string> commands = _input_builder(client, buffer, bytes);

	for (size_t i = 0; i < commands.size(); ++i)
	{
		std::string &cmd = commands[i];
		if (cmd.empty()) continue;

		if (cmd.length() > 512)
		{
			client->setSendBuffer(client->getSendBuffer().append("ERROR :Command too long\r\n"));
		}
		else
		{
			std::string reply = _parsing(cmd, fd);
			client->setSendBuffer(client->getSendBuffer().append(reply));
		}
		
		// PROTEÇÃO: Se o cliente foi removido durante o processamento (ex: QUIT), parar
		if (_clients.find(fd) == _clients.end())
			return;
		
		enablePollout(fd);
	}
}

void Server::sendData(int indexFd)
{
	int clientFd = _pollfds[indexFd].fd;
	Client* client = _clients[clientFd];

	if (client->getSendBuffer().empty()) {
		disablePollout(clientFd);
		return;
	}
	ssize_t bytes = send(clientFd, client->getSendBuffer().c_str(), client->getSendBuffer().size(), 0);
	if (bytes <= 0) 
	{
		if (bytes < 0) 
			return;
		removeClient(indexFd);
		return;
	}
	client->setSendBuffer(client->getSendBuffer().substr(bytes));
	if (client->getSendBuffer().empty())
		disablePollout(clientFd);
}

void Server::removeClient(int indexFd)
{
	int clientFd = _pollfds[indexFd].fd;
	Client* client = _clients[clientFd];

	// 1. REMOVER DO CLIENTE DE TODOS OS CANAIS (crítico para evitar dangling pointers)
	std::map<std::string, Channel*>::iterator it = _channels.begin();
	while (it != _channels.end())
	{
		it->second->removeMember(clientFd);
		it->second->removeOperator(clientFd);
		it->second->removeEnvited(clientFd);
		
		// Se o canal ficou vazio, deletar o canal também
		if (it->second->getMembers().empty())
		{
			delete it->second;
			_channels.erase(it++);
		}
		else
		{
			++it;
		}
	}

	// 2. LIMPEZA DO SERVIDOR
	std::cout << "DEBUG: Removendo cliente " << clientFd << std::endl;
	close(clientFd);
	_pollfds.erase(_pollfds.begin() + indexFd);

	// 3. DELETAR O OBJETO DO CLIENTE (sempre por último!)
	delete client;
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
	msg.append("██╗    ██╗███████╗██╗      ██████╗ ██████╗ ███╗   ███╗███████╗\r\n");
	msg.append("██║    ██║██╔════╝██║     ██╔════╝██╔═══██╗████╗ ████║██╔════╝\r\n");
	msg.append("██║ █╗ ██║█████╗  ██║     ██║     ██║   ██║██╔████╔██║█████╗\r\n");
	msg.append("██║███╗██║██╔══╝  ██║     ██║     ██║   ██║██║╚██╔╝██║██╔══╝\r\n");
	msg.append("╚███╔███╔╝███████╗███████╗╚██████╗╚██████╔╝██║ ╚═╝ ██║███████╗\r\n");
	msg.append(" ╚══╝╚══╝ ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝\r\n");
	msg.append("You can send /QUOTE HELP (irssi) or /HELP (nc) for help\r\n");
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