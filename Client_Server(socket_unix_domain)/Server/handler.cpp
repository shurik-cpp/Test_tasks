#include "handler.h"

using namespace std;



ostream& operator<<(ostream& s, const Params& p) {
	s << "channel: " << p.channel;
	if (p.value > -1) {
		s << ", range: " << p.value;
	}
	return s;
}


Handler::Handler(const string& comm, const vector<Params>& params)
	: command(move(comm))
	, parametrs(move(params)) {

}


void Handler::PollTheDevice(Device* device) {
	bool is_global_ok = true;

	string additional_info;
	if (command == "close") result_message = "ok, " + command;
	else {
		if (parametrs.size() == 0) {
			is_global_ok = false;
			additional_info = ", params is empty";
		}
		for (auto& param : parametrs) {
			bool is_channel_ok = true;
			is_global_ok = device->IsChannelError(param.channel);

			if (command == "start_measure") {
				is_channel_ok = device->StartMeasure(param.channel);
			}
			else if (command == "set_range") {
				is_channel_ok = device->SetRange(param.channel, param.value);
			}
			else if (command == "stop_measure") {
				is_channel_ok = device->StopMeasure(param.channel);
			}
			else if (command == "get_status") {
				additional_info += ", " +device->GetState(param.channel);
			}
			else if (command == "get_result") {
				additional_info += ", ";
				size_t last_size = additional_info.size();
				additional_info += device->GetMessure(param.channel);
				if (additional_info.size() == last_size) is_channel_ok = false;
			}
			else {
				additional_info = ", unknown_command";
				is_global_ok = false;
			}

			if (!is_channel_ok) is_global_ok = false;
		}

		if (is_global_ok) {
			result_message = "ok" + additional_info;
		}
		else {
			result_message = "fail" + additional_info;
		}
	}
}

