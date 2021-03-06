#include "stdafx.h"
#include "LL.h"


bool Grammar::is_end(char c) {
    return !(c >= 'A'&&c <= 'Z') && c != '|'&& c != '>';
}

bool Grammar::is_grammar(char c) {
    return c >= 'A'&&c <= 'Z';
}

void Grammar::insert_follow(char c, set<char> s, map<char, set<char>> &follow) {
    auto it = follow.find(c);
    if (it != follow.end()) {
        s.insert(it->second.begin(), it->second.end());
        follow.erase(c);
    }
    follow.insert(pair < char, set<char>>(c, s));
}

bool Grammar::has(char c, set<char> &s) const {
    auto it = s.find(c);
    if (it != s.end())return true;
    return false;
}

set<char> Grammar::get_set(char c, map<char, set<char>> &fi) const {
    auto it = fi.find(c);
    return it->second;
}

void Grammar::get_first(map<char, set<char>>&first) {
    set<char> s;
    for (auto g : grammar) {
        char prefix = g[0];
        for (int i = 0; i < (int)g.length(); ++i) {
            char c = g[i];
            if (c == '>' || c == '|') {//防止i+1越界
                char nc = g[i + 1];
                //找到first
                if (is_end(nc)) {
                    s.insert(nc);
                }
                //否则找下一个文法的first
                else {
                    vector<char> v(1, nc);
                    for (int index = 0; index < v.size(); ++index) {
                        char cc = v[index];
                        while (true) {
                            for (auto g : grammar) {
                                if (g[0] == cc) {
                                    for (int i = 1; i < (int)g.length(); ++i) {
                                        if (g[i] == '>' || g[i] == '|') {
                                            if (is_end(g[i + 1])) {
                                                s.insert(g[i + 1]);
                                            }
                                            else {
                                                v.push_back(g[i + 1]);
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        }
                    }
                    //清空对文法g的推导集合
                    v.clear();
                }
            }
        }
        pair<char, set<char>> p(prefix, s);
        //合并已存在的文法
        for (auto f : first) {
            if (f.first == prefix) {
                first.erase(f.first);
                p.second.insert(f.second.begin(), f.second.end());
                //需要及时退出，否则会报迭代器不可increable
                break;
            }
        }
        first.insert(p);
        s.clear();
    }
}

int Grammar::get_follow_size(map<char, set<char>> &follow) {
    int size = 0;
    for (auto f : follow) {
        size += (int)f.second.size();
    }
    return size;
}

void Grammar::get_follow(const map<char, set<char>> &first, map<char, set<char>> &follow) {
    set<char> s;
    char prefix = NULL;
    int size = 0;
    do {
        size = get_follow_size(follow);
        for (auto g : grammar) {
            int len = (int)g.size();
            //每个文法开头都把#加入进去
            s.insert('#');
            insert_follow(g[0], s, follow);
            for (int i = 1; i < (int)g.size(); ++i) {
                char c = g[i];
                char nc = g[i + 1];
                if (is_grammar(c) && is_grammar(nc)) {
                    //把first(空除外)加入follow c中
                    auto it = first.find(nc);
                    s.insert(it->second.begin(), it->second.end());
                    s.erase('$');
                    insert_follow(c, s, follow);
                    //c的推导式也应该把first加进去
                    //这里的推导只需一次，所以不需要while循环了
                    for (auto gg : grammar) {
                        int len = (int)gg.length() - 1;
                        if (c == gg[0] && is_grammar(gg[len])) {
                            insert_follow(gg[len], s, follow);
                        }
                    }
                }
                if (c == '>' && !is_grammar(nc) && is_grammar(g[i + 2])) {
                    s.clear();
                    if (!is_grammar(g[i + 3])) {
                        s.insert(g[i + 3]);
                    }
                    insert_follow(g[i + 2], s, follow);
                }
                int l = len - 1;
                while (is_grammar(g[l])) {
                    if (g[l] == g[0]) break;
                    //follow g[0]
                    s = follow.find(g[0])->second;
                    auto it = first.find(g[l])->second;
                    //加入follow g[l]
                    insert_follow(g[l], s, follow);
                    //空集表示可以继续规约
                    if (it.find('$') != it.end()) {
                        l -= 1;
                    }
                    else {
                        break;
                    }
                }
            }
            s.clear();
        }

    } while (get_follow_size(follow) != size);
}


void LL::get_table(const map<char, set<char>> &first, map<char, set<char>> &follow) {
    for (auto f : first) {
        //终结符集
        char A = f.first;
        for (auto a : f.second) {
            if (a == '$') {
                set<char> s = get_set(A, follow);
                for (auto a : s) {
                    string g(1, A);
                    g += "->$";
                    insert_table(A, a, g);
                    //cout << A << ": " << a << " " << g << endl;
                }
            }
            else {
                for (auto g : grammar) {
                    if (g[0] == A) {
                        int arrow = (int)g.find('>');
                        int vline = (int)g.find('|');
                        if (is_end(g[arrow + 1])) {
                            if (a == g[arrow + 1]) {
                                if (vline != g.npos) {
                                    insert_table(A, a, g.substr(0, 6));
                                }
                                else {
                                    insert_table(A, a, g);
                                }
                            }
                            if (vline != g.npos&&a == g[vline + 1]) {
                                insert_table(A, a, string(1, A) + "->" + g.substr(vline + 1, (int)g.length() - 1));
                            }
                        }
                        else {
                            insert_table(A, a, g);
                        }
                    }
                }
            }

        }
        cout << endl;
    }
}

LL::LL(string t) {
    text = t;
}

string LL::run() {
    int i = 0;
    string st = "#E";
    bool flag = true;
    string result = "[{" + string("\"stack\":\"") + st + "\",\"input\":\"" + text.substr(i, (int)text.length()) + "\",\"prod\":\"\",\"action\":\"inital\"},";
    g.get_first(first);
    g.get_follow(first, follow);
    get_table(first, follow);
    show();
    while (flag) {
        int len = (int)st.length();
        char x = st[len - 1];
        char a = text[i];
        if (is_end(x) && x != '#') {
            if (x == a) {
                i++;
                result += "{" + string("\"stack\":\"") + st + "\",\"input\":\"" + text.substr(i, (int)text.length()) + "\",\"prod\":\"\",\"action\":\"getnext\"},";//,\"action\":\"pop,push("+ text[i-1]+ ")\"
                st = st.substr(0, len - 1);
            }
            else {
                cout << "1error" << endl;
            }
        }
        else if (x == '#') {
            if (x == a) {
                result += "{" + string("\"stack\":\"") + st + "\",\"input\":\"" + text.substr(i, (int)text.length()) + "\",\"prod\":\"\"},";
                cout << "success" << endl;
                flag = false;
            }
            else {
                cout << "2error" << endl;
            }
        }
        else if (table_find(x, a) != "") {
            string g = table_find(x, a);
            reverse(g.begin(), g.end());
            st = st.substr(0, len - 1);
            string l = "";
            for (auto s : g) {
                if (s == '>' || s == '$') break;
                st += s;
                l += s;
            }
            reverse(g.begin(), g.end());
            result += "{" + string("\"stack\":\"") + st + "\",\"input\":\"" + text.substr(i, (int)text.length()) + "\",\"prod\":\"" + g + "\",\"action\":\"pop" + (l != "" ? ", push(" + l + ")" : "") + "\"},";
        }
        else {
            cout << "3error" << endl;
        }

    }
    set<char> se;
    for (auto A : table) {
        for (auto f : A.second) {
            se.insert(f.first);
        }
    }
    string endlist = "[";
    for (auto s : se) {
        endlist += +"\"" + string(1, s) + "\",";
    }
    endlist = endlist.substr(0, endlist.length() - 1) + "]";

    string table_str = "[";
    for (auto A : table) {
        string st = "[";

        for (auto s : se) {
            auto it = A.second.find(s);
            if (it != A.second.end()) {
                st += "\"" + it->second + "\",";
            }
            else {
                st += "\"\",";
            }
        }
        table_str += st.substr(0, st.length() - 1) + "],";
    }
    table_str = table_str.substr(0, table_str.length() - 1) + "]";
    return "{\"0\":" + result.substr(0, (int)result.length() - 1) + "],\"1\":" + endlist + ",\"2\":" + table_str + "}";
}

void LL::insert_table(char A, char a, string s) {
    map<char, string> m;
    m.insert(pair<char, string>(a, s));
    //保证推导式不会被覆盖，而是合并，之前没考虑，，，，导致值都被覆盖了
    auto it = table.find(A);
    if (it != table.end()) {
        m.insert(it->second.begin(), it->second.end());
        table.erase(A);
    }
    table.insert(pair<char, map<char, string>>(A, m));
}

string LL::table_find(char A, char a) {
    auto it = table.find(A);
    if (it != table.end()) {
        auto mp = it->second;
        auto it_ = mp.find(a);
        if (it_ != mp.end()) {
            return it_->second;
        }
    }
    return "";
}

void LL::show() {
    //for (auto g : grammar) {
    //	cout << g << endl;
    //}
    for (auto f : first) {
        cout << f.first << ": ( ";
        for (auto c : f.second) {
            cout << c << ", ";
        }
        cout << ")" << endl;
    }
    cout << endl;
    for (auto f : follow) {
        cout << f.first << ": ( ";
        for (auto c : f.second) {
            cout << c << ", ";
        }
        cout << ")" << endl;
    }
}