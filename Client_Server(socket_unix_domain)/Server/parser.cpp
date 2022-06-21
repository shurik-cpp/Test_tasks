#include "parser.h"

using namespace std;

vector<string> SplitIntoWords(const string& str) {
	vector<string> result;
	auto str_begin = begin(str);
	const auto str_end = end(str);

	while (true) {
		auto it = find(str_begin, str_end, ' ');
		string s(str_begin, it);
		if (s.size() > 0 && s.back() == ',') {
			s.erase(end(s) - 1);
		}
		result.push_back(s);
		if (it == str_end) {
			break;
		} else {
			str_begin = it + 1;
		}
	}
	return result;
}

Handler ParseRequest(const string s) {
	vector<string> splitted = SplitIntoWords(s);
	string command;
	vector<Params> parametrs;
	if (!splitted.empty()) {
		command = splitted[0];
		for (size_t i = 1; i < splitted.size(); ++i) {
			string& param = splitted[i];
			Params val;
			if (param[0] == 'c') {
				auto it = find(begin(param), end(param), 'l');
				param.erase(begin(param), ++it);
				// в строке param остался только номер канала
				val.channel = stoi(param);
				// проверим следущий параметр, если range, получаем его значение
				if ((i + 1 < splitted.size()) && (splitted[i + 1][0] == 'r')) {
					param = splitted[++i];
					it = find(begin(param), end(param), 'e');
					param.erase(begin(param), ++it);
					// в строке param осталось только значение range
					val.value = stoi(param);
				}
			}
			parametrs.push_back(val);
		}
	}
	return Handler(command, parametrs);
}
