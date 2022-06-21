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
	vector<Params> result;
	vector<string> parametrs = SplitIntoWords(s);
	string command = parametrs[0];
	for (size_t i = 1; i < parametrs.size(); ++i) {
		string& param = parametrs[i];
		Params val;
		if (param[0] == 'c') {
			auto it = find(begin(param), end(param), 'l');
			param.erase(begin(param), ++it);
			// в строке param остался только номер канала
			val.channel = stoi(param);
			// проверим следущий параметр, если range, получаем его значение
			if ((i + 1 < parametrs.size()) && (parametrs[i + 1][0] == 'r')) {
				param = parametrs[++i];
				it = find(begin(param), end(param), 'e');
				param.erase(begin(param), ++it);
				// в строке param осталось только значение range
				val.value = stoi(param);
			}
		}
		result.push_back(val);
	}
	return Handler(command, result);
}
