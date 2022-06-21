#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include "device.h"

struct Params {
	int channel;
	int value = -1;
};

std::ostream& operator<< (std::ostream& s, const Params& p);

class Handler {
public:
	Handler(const std::string& comm, const std::vector<Params>& params);

	void PollTheDevice(Device* device);
	std::string GetResult() const { return result_message; }

private:
	const std::string command;
	const std::vector<Params> parametrs;
	std::string result_message;
};
