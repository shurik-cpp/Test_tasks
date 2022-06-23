#include "device.h"

using namespace std;

Device::Device(const size_t number_of_channels) {
	for (size_t i = 0; i <= number_of_channels; ++i) {
		channels.push_back(Channel{});
	}
}

int Device::GetRandom(const int minValue, const int maxValue) {
	engine.seed(unsigned(time(nullptr)));
	// Создаём распределение для интов
	std::uniform_int_distribution<int> distribution(minValue, maxValue);
	// Вычисляем псевдослучайное число: вызовем распределение как функцию,
	// передав генератор произвольных целых чисел как аргумент.
	return distribution(engine);
}

float Device::GetRandom(const float minValue, const float maxValue) {
	engine.seed(unsigned(time(nullptr)));
	// Создаём распределение для флоат
	std::uniform_real_distribution<float> distribution(minValue, maxValue);
	return distribution(engine);
}

bool Device::IsChannelError(const size_t channel_number) {
	if (IsChannelEnable(channel_number)) {
		if (entry_counter++ >= GetRandom(MIN_ENTRY_FOR_RANDOMIZE, MAX_ENTRY_FOR_RANDOMIZE)) { // чтобы реже error_state или busy_state вылетали
			entry_counter = 0;
			channels[channel_number].state = static_cast<ChannelState>(GetRandom(0, 1));
			if (!channels[channel_number].state) {
				return false;
			}
		}
	}
	return true;
}

bool Device::IsChannelEnable(const size_t channel_number) const {
	return channel_number <= channels.size();
}

string Device::GetState(const size_t channel_number) const {
	if (!IsChannelEnable(channel_number)) return "";
	return CHANNEL_STATE[channels[channel_number].state];
}
string Device::GetMeasure(const size_t channel_number) {
	if (!IsChannelEnable(channel_number) || !channels[channel_number].state) return "";
	Channel& ch = channels[channel_number];
	string voltage;
	switch (ch.range) {
		case 0:
			ch.voltage = GetRandom(0.0000001f, 0.001f);
			voltage = to_string(ch.voltage);
		break;
		case 1:
			ch.voltage = GetRandom(0.001f, 1.f);
			voltage = to_string(ch.voltage);
		break;
		case 2:
			ch.voltage = GetRandom(1, 1000);
			voltage = to_string(static_cast<int>(ch.voltage));
		break;
		case 3:
			ch.voltage = GetRandom(1000, 1'000'000);
			voltage = to_string(static_cast<int>(ch.voltage));
		break;
	}
	return voltage;
}

bool Device::SetRange(const size_t channel_number, const int new_range) {
	if (!IsChannelEnable(channel_number) || new_range > MAX_RANGE) return false;
	channels[channel_number].range = new_range;
	return true;
}

bool Device::StartMeasure(const size_t channel_number) {
	if (!IsChannelEnable(channel_number)) return false;
	channels[channel_number].state = ChannelState::MEASURE;
	return true;
}

bool Device::StopMeasure(const size_t channel_number) {
	if (!IsChannelEnable(channel_number)) return false;
	channels[channel_number].state = ChannelState::IDLE;
	return true;
}
