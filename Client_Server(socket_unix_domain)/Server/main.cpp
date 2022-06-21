#include <iostream>
#include "server.h"

static const int DEVICE_DEFAULT_CHANNELS = 7;

int main(int argc, char *argv[]) {

	if (argc == 3) {
		std::string arg = argv[1];
		if (arg == "--path") {
			SocketThread::Instance(DEVICE_DEFAULT_CHANNELS)->SetSocketPath(argv[2]);
		}
		else if (arg == "--channels") {
			int val = atoi(argv[2]);
			if (val <= 0) {
				std::cerr << "Error: Number of channels must be positive!/n";
				return 1;
			}
			SocketThread::Instance(static_cast<size_t>(val));
		}
	}
	else {
		SocketThread::Instance(DEVICE_DEFAULT_CHANNELS);
	}

	SocketThread::Instance()->RunServer();

	SocketThread::Instance()->Stop();
	return 0;
}
