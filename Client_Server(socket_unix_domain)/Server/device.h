#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <ctime>
#include <random>
#include <cassert>

enum ChannelState {
	ERROR = 0,
	BUSY,
	IDLE,
	MEASURE,
	END
};

struct Channel {
	int channel;
	int range = 1;
	float voltage;

	ChannelState state = IDLE;
};

class Device {
public:
	Device(const size_t number_of_channels);
	std::string GetState(const size_t channel_number) const;
	std::string GetMessure(const size_t channel_number);
	bool SetRange(const size_t channel_number, const int new_range);
	bool StartMeasure(const size_t channel_number);
	bool StopMeasure(const size_t channel_number);
	bool IsChannelError(const size_t channel_number);

private:
	std::vector<Channel> channels;
	const std::vector<std::string> CHANNEL_STATE = {
		"error_state",
		"busy_state",
		"idle_state",
		"measure_state"
	};

	bool IsChannelEnable(const size_t channel_number) const;

	const int MAX_RANGE = 3;

	std::mt19937 engine;
	int GetRandom(const int minValue, const int maxValue);
	float GetRandom(const float minValue, const float maxValue);
	const int MIN_ENTRY_FOR_RANDOMIZE = 10;
	const int MAX_ENTRY_FOR_RANDOMIZE = 50;
	int entry_counter = 0;
};



