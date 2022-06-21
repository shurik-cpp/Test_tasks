#include "server.h"

#include <iostream>
#include <exception>
#include <future>


using namespace std;

bool operator< (const pollfd& lhs, const pollfd& rhs) {
	return lhs.fd < rhs.fd;
}

SocketThread* SocketThread::instance_ = nullptr;

SocketThread* SocketThread::Instance() {
	return instance_;
}

SocketThread* SocketThread::Instance(const int number_of_channels) {
	if (!instance_) {
		instance_ = new SocketThread(number_of_channels);
	}
	return instance_;
}

SocketThread::SocketThread(const int number_of_channels)
	: device(Device(number_of_channels)) {
	if (socket_path.empty()) {
		const string home_path(getenv("HOME")); // получаем путь домашней директории текущего пользователя
		socket_path = home_path + "/.cache/.server-socket";
	}
}

SocketThread::~SocketThread() {
	Stop();
	if (instance_) {
		delete instance_;
		instance_ = nullptr;
	}
}

void SocketThread::Stop() {
	if (listener.fd) {
		close(listener.fd);
		listener.fd = -1;
	}
	CleanSocket();
	cout << "Server is down." << endl;
}

void SocketThread::SetSocketPath(const string& path) {
	socket_path = path;
}

int SocketThread::Socket(const int domain, const int type, const int protocol) const {
	int sockfd = socket(domain, type, protocol);
	if (sockfd == -1) {
		perror("Socket");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}

void SocketThread::Bind(const int sockfd, sockaddr* addr, const socklen_t addrlen) const {
	int result = bind(sockfd, addr, addrlen);
	if (result == -1) {
		perror("Bind");
		exit(EXIT_FAILURE);
	}
}

void SocketThread::Listen(const int sockfd, const int backlog) const {
	int result = listen(sockfd, backlog);
	if (result == -1) {
		perror("Listen");
		exit(EXIT_FAILURE);
	}
}

int SocketThread::Poll(pollfd *fds, const nfds_t size, const int timeout) {
	int result = poll(fds, size, timeout);
	if (result == -1) {
		perror("Poll");
		exit(EXIT_FAILURE);
	}
	return result;
}

int SocketThread::Accept(const int fd, sockaddr* addr, socklen_t* const addr_len) {
	int result = accept(fd, addr, addr_len);
	if (result == -1) {
		perror("Accept");
		exit(EXIT_FAILURE);
	}
	return result;
}

void SocketThread::RunServer() {
	cout << "Starting thread server...\n"
			 << "Listener socket path: " << socket_path << endl;
	CleanSocket();

	cout << "Creating socket...\n";
	listener.fd = Socket(AF_UNIX, SOCK_STREAM, 0);
	listener.events = POLLRDNORM;

	cout << "Initializing socket...\n";
	sockaddr_un listener_addr;
	listener_addr.sun_family = AF_UNIX;
	memset(listener_addr.sun_path, '\0', sizeof(listener_addr.sun_path));
	strncpy(listener_addr.sun_path, socket_path.c_str(), socket_path.size());

	cout << "Binding socket...\n";
	socklen_t adrlen = sizeof(listener_addr);
	Bind(listener.fd, reinterpret_cast<sockaddr*>(&listener_addr), adrlen);

	cout << "Listening socket...\n";
	Listen(listener.fd, MAX_BACKLOG);

	cout << "Server ready." << std::endl;
	//==================================================
	bool run_server = true;
	while (run_server) {

		pollfd poll_fds[clients.size() + 1];
		// слушатель будет нулевым элементом массива
		poll_fds[0] = listener;
		// остальные будут клиенты, уже отсортированные по возрастанию
		int i = 1;
		for (const auto& it : clients) {
			poll_fds[i++] = it;
		}
		const nfds_t max_fd = poll_fds[clients.size()].fd + 1;

		int now_ready = Poll(poll_fds, max_fd, TIMEOUT);

		if (poll_fds[0].revents & POLLRDNORM) { // новое соединение с клиентом
			sockaddr_un client_addr;
			socklen_t cli_len = sizeof(client_addr);
			const int client_fd = Accept(listener.fd, reinterpret_cast<sockaddr*>(&client_addr), &cli_len);
			clients.insert( pollfd{client_fd, POLLRDNORM, POLLWRNORM} );

			if (--now_ready <= 0) continue; // больше нет готовых для чтения дескрипторов
		}

		for (size_t i = 1; i <= clients.size(); ++i) { // проверяем всех старых клиентов на наличие данных для чтения
			if (poll_fds[i].revents & (POLLRDNORM | POLLERR)) {
				const int sockfd = poll_fds[i].fd;
				char buffer[BUFFER_SIZE];
				memset(buffer, '\0', sizeof(buffer));
				const int data_size = read(sockfd, buffer, BUFFER_SIZE);
				if (data_size < 0) {
					if (errno == ECONNRESET) { // клиент переподключился
						close(sockfd);
						const auto it = clients.find(poll_fds[i]);
						clients.erase(it);
					}
					else {
						cerr << "Error: Failed to read data\n";
					}
				}
				else if (data_size == 0) { // клиент отключился
					close(sockfd);
					const auto it = clients.find(poll_fds[i]);
					clients.erase(it);
				}
				else { // все хорошо
					Handler handler = ParseRequest(buffer);
					handler.PollTheDevice(&device);
					string answer = handler.GetResult();
					cout << answer << endl;
					if (answer == "ok, close") {
						 if (clients.size() > 1) {
							 answer = "Failure: There are other clients";
						 }
						 else {
							 run_server = false;
						 }
					}
					write(sockfd, answer.c_str(), answer.size());
				}

				if (--now_ready <= 0) break; // больше не осталось готовых дескрипторов
			}
		}
	}
}

void SocketThread::CleanSocket() {
	if (access(socket_path.c_str(), F_OK) != -1) { // проверяем наличие файла сокета-слушателя
		cout << "Cleanup socket\n";
		unlink(socket_path.c_str()); // удаляем файл сокета
	}
}
