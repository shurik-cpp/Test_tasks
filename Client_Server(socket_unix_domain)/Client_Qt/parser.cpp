#include "parser.h"

using namespace std;

QString StringToQString(const std::string& str) {
	QString qstr;
	for (const auto it : str) {
		qstr.push_back(it);
	}
	return qstr;
}

vector<QString> SplitIntoWords(const string str) {
	cerr << "str.size() == " << str.size() << endl;
	vector<QString> result;
	string word;
	for (const auto it : str) {
		if (it != ' ') word.push_back(it);
		else {
			if (word.back() == ',') word.pop_back();
			cerr << word << endl;
			result.push_back(StringToQString(word));
			word.clear();
		}
	}
	result.push_back(StringToQString(word));
	return result;
}
