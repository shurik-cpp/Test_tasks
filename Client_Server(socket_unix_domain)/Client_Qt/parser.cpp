#include "parser.h"

using namespace std;

vector<QString> SplitIntoWords(const string str) {
	vector<QString> result;
	auto str_begin = begin(str);
	const auto str_end = end(str);

	while (true) {
		auto it = find(str_begin, str_end, ' ');
		string s(str_begin, it);
		if (s.size() > 0 && s.back() == ',') {
			s.erase(end(s) - 1);
		}
		cerr << s << endl;
		QChar qc[s.size()];
		memset(qc, '\0', sizeof(QChar));
		for (size_t i = 0; i < s.size(); ++i) {
			qc[i] = s[i];
		}
		result.push_back(QString(qc, s.size()));
		if (it == str_end) {
			break;
		} else {
			str_begin = it + 1;
		}
	}
	return result;
}
