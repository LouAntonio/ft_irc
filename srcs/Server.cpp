#include "../includes/Server.hpp"

Server::Server(int port, const std::string& password)
	: _serverFd(-1), _password(password)
{
	setupSocket(port);

	struct pollfd pfd;
	pfd.fd = _serverFd;
	pfd.events = POLLIN;
	pfd.revents = 0;

	_pollfds.push_back(pfd);
}

void Server::setupSocket(int port)
{
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
	addr.sin_port = htons(port);

	if (bind(_serverFd, (sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("bind() failed");

	if (listen(_serverFd, SOMAXCONN) < 0)
		throw std::runtime_error("listen() failed");

	if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl() failed");
}

void Server::run()
{
	while (true)
	{
		int ret = poll(&_pollfds[0], _pollfds.size(), -1);
		if (ret < 0)
			throw std::runtime_error("poll() failed");

		for (size_t i = 0; i < _pollfds.size(); ++i)
		{
			if (_pollfds[i].revents & POLLIN)
			{
				if (_pollfds[i].fd == _serverFd)
					acceptClient();
				else
					receiveData(_pollfds[i].fd);
			}

			if (_pollfds[i].revents & POLLOUT)
				sendData(_pollfds[i].fd);
		}
	}
}

void Server::acceptClient()
{
	int clientFd = accept(_serverFd, NULL, NULL);
	if (clientFd < 0)
		return;

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
		return;

	struct pollfd pfd;
	pfd.fd = clientFd;
	pfd.events = POLLIN;
	pfd.revents = 0;

	_pollfds.push_back(pfd);
	_clients[clientFd] = new Client(clientFd);

	std::cout << "New client connected: " << clientFd << std::endl;
}

void Server::receiveData(int clientFd)
{
	char buffer[512];
	ssize_t bytes = recv(clientFd, buffer, sizeof(buffer), 0);

	if (bytes <= 0)
	{
		removeClient(clientFd);
		return;
	}

	_clients[clientFd]->recvBuffer.append(buffer, bytes);
}

void Server::sendData(int clientFd)
{
	Client* client = _clients[clientFd];
	if (client->sendBuffer.empty())
		return;

	ssize_t bytes = send(clientFd,
		client->sendBuffer.c_str(),
		client->sendBuffer.size(),
		0);

	if (bytes <= 0)
	{
		removeClient(clientFd);
		return;
	}

	client->sendBuffer.erase(0, bytes);
}

void Server::removeClient(int clientFd)
{
	close(clientFd);

	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd == clientFd)
		{
			_pollfds.erase(_pollfds.begin() + i);
			break;
		}
	}

	delete _clients[clientFd];
	_clients.erase(clientFd);

	std::cout << "Client disconnected: " << clientFd << std::endl;
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
