#include <iostream>
#include <fstream>
#include <exception>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/un.h>

using namespace std;

int Socket(const int domain, const int type, const int protocol) {
	int sockfd = socket(domain, type, protocol);
	if (sockfd == -1) {
		perror("Socket");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}

void Connect(const int sockfd, sockaddr* addr, const socklen_t addrlen) {
	if (connect(sockfd, addr, addrlen) == -1) {
		perror("Connect");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[]) {

	string socket_path;
	if (argc == 2) {
		socket_path = argv[1];
	}
	else {
		const string home_path(getenv("HOME"));
		socket_path = home_path + "/.cache/.server-socket";
	}

	cout << "Creating socket...\n";
	int sockfd = Socket(AF_UNIX, SOCK_STREAM, 0);

	cout << "Connecting to socket...\n";
	sockaddr_un serv_addr;
	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path, socket_path.c_str(), socket_path.size());

	socklen_t adrlen = sizeof(serv_addr);
	Connect(sockfd, reinterpret_cast<sockaddr*>(&serv_addr), adrlen);
	cout << "Client ready." << endl;

	int client_status = 0;
	while (true) {
//		if (access(socket_path.c_str(), F_OK) == -1) { // проверяем наличие файла сокета-слушателя
//			cerr << "Server is down.\n";
//			cout << "Enter 'r' for refresh or any symbol for exit: ";
//			string s;
//			cin >> s;
//			if (s == "r") continue;
//			else {
//				cerr << "Error: socket file is lost/n";
//				client_status = 1;
//				break;
//			}
//		}

		cout << "Enter message and press 'Enter': ";
		string request;
		getline(cin, request);
		if (request.empty()) continue;

		const int BUFFER_SIZE = 8192;
		char buffer[BUFFER_SIZE];
		memset(buffer, '\0', sizeof(buffer));
		write(sockfd, request.c_str(), request.size());

		ssize_t data_size = read(sockfd, &buffer, BUFFER_SIZE);
		if (data_size == -1) {
			cerr << "Error: Failed to read socket\n";
			continue;
		}
		string answer(buffer);
		cout << answer << endl;

		if (answer == "OK, close" || request == "exit") break;
	}

	close(sockfd);


	return client_status;
}
