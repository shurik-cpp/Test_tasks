#pragma once

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h> // read(), write(), close()
#include <poll.h>
#include <set>
#include <cstdlib>
#include <string>

#include "handler.h"
#include "device.h"
#include "parser.h"

class SocketThread {
public:
	static SocketThread* Instance();
	static SocketThread* Instance(const int number_of_channels);

	void RunServer();
	void Stop();
	void SetSocketPath(const std::string& path);

private:
	SocketThread(const int number_of_channels);
	~SocketThread();

	void CleanSocket();
	int Socket(const int domain, const int type, const int protocol) const;
	void Bind(const int sockfd, sockaddr* addr, const socklen_t addrlen) const;
	void Listen(const int sockfd, const int backlog) const;
	int Poll(pollfd *fds, const nfds_t nfds, const int timeout);
	int Accept(const int fd, sockaddr* addr, socklen_t* const addr_len);

	const int MAX_BACKLOG = 5;
	const int BUFFER_SIZE = 8192;
	const int TIMEOUT = 1000;

	Device device;
	std::string socket_path;
	pollfd listener;
	std::set<pollfd> clients;
	static SocketThread* instance_;

};

