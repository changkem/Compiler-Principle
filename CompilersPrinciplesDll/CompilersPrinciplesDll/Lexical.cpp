#include "stdafx.h"
#include "Lexical.h"


Lexical::Lexical() {
	in.push_back("");
	index = 0;
	row = col = 0;
}

Lexical::Lexical(string str) {
	int len = (int)str.length();
	string s = "";
	for (int i = 0; i < len; ++i) {
		if (str[i] == '\n' && s != "") {
			in.push_back(s);
			s = "";
		}
		else {
			s += str[i];
		}
	}
	if (s != "") {
		in.push_back(s);
	}
	index = 0;
	row = col = 0;
	
}

string Lexical::slice(int start, int end = 0) {
	string s = in[0];
	return s.substr(start, end);
}

bool Lexical::isSpcae(char ch) {
	return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}

bool Lexical::isId(string id) {
	if (isdigit(id[0]) || id[0] == '_' /*|| isIllegalChar(c)*/)return false;
	for (auto c : id) {
		if (!isalnum(c) && c != '_' /*&& !isIllegalChar(c)*/)return false;
	}
	return true;
}

bool Lexical::isKey(string k) {
	for (auto s : Key) {
		if (s == k)return true;
	}
	return false;
}
//验证单个字符的运算符
bool Lexical::isOperator(char ch) {
	for (auto s : Operator) {
		if (s[0] == ch)return true;
	}
	return false;
}
//验证多个字符的运算符
bool Lexical::isOperator(string ch) {
	for (auto s : Operator) {
		if (s == ch)return true;
	}
	return false;
}

bool Lexical::isDelimitor(char li) {
	for (auto c : Delimitor)
		if (c[0] == li) return true;
	return false;
}


string Lexical::to_json_str(int type, string val) {
	int len = (int)val.length();
	string position = string("\",\"position\":") + "\"(" + to_string(row + 1) + ", " + to_string(col + 1 - len) + ")\"";
	if (type == 0) {
		return "{" + string("\"key\":\"") + val + position + "},";
	}
	if (type == 1) {
		return "{" + string("\"identifier\":\"") + val + position + "},";
	}
	if (type == 2) {
		return "{" + string("\"number\":\"") + val + position + "},";
	}
	if (type == 3) {
		return "{" + string("\"comment\":\"") + val + position + "},";
	}
	if (type == 4) {
		return "{" + string("\"operator\":\"") + val + position + "},";
	}
	if (type == 5) {
		return "{" + string("\"delimiter\":\"") + val + position + "},";
	}
	if (type == 6) {
		return "{" + string("\"string\":\"") + val + position + "},";
	}
	if (type == 7) {
		return "{" + string("\"error\":\"") + val + position + "},";
	}
}
string Lexical::run() {
	string result = "[";
	for (row = 0; row < in.size(); ++row) {
		int len = (int)in[row].length();
		string s = in[row];
		char ch = NULL;
		for (col = 0; col < len; ++col) {
			ch = s[col];
			//解析标识符或者关键字，字母开头后接字母或者数字或者下划线
			if (isalpha(ch)) {
				string key(1, ch);
				while (col + 1 < len && (isalnum(s[col + 1]) || s[col + 1] == '_')) {
					col = col + 1;
					ch = s[col];
					key += ch;
				}
				if (isKey(key)) {
					result += to_json_str(0, key);
				}
				else if (isId(key)) {
					result += to_json_str(1, key);
				}
				else {
					result += to_json_str(7, key);
				}
				//cout << key << "\n";
			}
			else if (isdigit(ch)) {
				string number(1, ch);
				while (col + 1 < len&&isdigit(s[col + 1])) {
					col = col + 1;
					ch = s[col];
					number += ch;
				}
				result += to_json_str(2, number);
				//cout << number << "  " << row << " " << col + 1 << "\n";
				/*				while (isNum(ch) || ch == 'e' || ch == 'E') {
					string num = "";
					int l = 0;
					if (!isNum(ch)) {
						while (isNum(ch))
						{
							num += ch;
							col = col + 1;
							ch = s[col];
						}
						l = stoi(num);
					}
					else {
						number += ch;
						col = col + 1;
						ch = s[col];
					}
					for (int i = 0; i < l; ++i) {
						number += "0";
					}
				}*/
			}
			else if (ch == '/' && col + 1 < len && s[col + 1] == '/') {
				string comment = s.substr(col + 2, len);
				col = len;
				result += to_json_str(3, comment);
				//cout << comment << "\n";
			}
			else if (isOperator(ch)) {
				string op(1, ch);
				//双字符运算符，但是匹配失败
				if (col + 1 < len && !isOperator(op + s[col + 1]) && isOperator(s[col + 1])) {
					op += s[col + 1];
					col += 1;
					result += to_json_str(7, op);
				}
				//双字符匹配成功
				else if (col + 1 < len && isOperator(op + s[col + 1])) {
					op += s[col + 1];
					col += 1;
					result += to_json_str(4, op);
				}
				//匹配单字符
				else if (col + 1 >= len || (isOperator(op) && !isOperator(s[col + 1]))) {
					result += to_json_str(4, op);
				}

				//cout << op << "\n";
			}
			else if (isDelimitor(ch)) {
				string op(1, ch);
				result += to_json_str(5, op);
			}

			/*else if (ch == '\'' || ch == '\"') {
				bool isSinglequote = ch == '\'';
				string str = "";
				if (isSinglequote) {
					while (s[col + 1] != '\''&&col + 1 < len) {
						col += 1;
						ch = s[col];
						str += ch;
					}
					if (ch != '\'') {
						result += to_json_str(7, str);
					}
					else {
						result += to_json_str(6, str);
					}
				}
				else {
					while (s[col + 1] != '\"'&&col + 1 < len) {
						col += 1;
						ch = s[col];
						str += ch;
					}
					if (ch != '\"') {
						result += to_json_str(7, str);
					}
					else {
						result += to_json_str(6, str);
					}
				}
			}*/
			else if (!isSpcae(ch)) {
				string err(1, ch);
				while (col + 1 < len && !isSpcae(s[col + 1])) {
					col += 1;
					ch = s[col];
					err += ch;
				}
				result += to_json_str(7, err);
				cout << err << "\n";
			}
			//cout << ch << '\t' << row << " " << col << "\n";
		}
	}

	return result.substr(0, result.length() - 1) + "]";
}