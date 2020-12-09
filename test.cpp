#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>

#include "TrieTree.h"
#include "skiplist.h"
#include "ClockTime.h"
using namespace std;

constexpr int MODE = 1;

void test1() {
    // ac自动机 多模式串匹配
    AC_automaton<char, MODE> aca;
    vector<string> vs = {"aaa", "abcab"};
    vector<string> ss{"bcd", "ce", "ce", "ababa", "abce"};

    aca.buildTrieTree(ss);
    aca.buildAC_automaton();
    aca.start_ac_automaton("abcexbcdxxxcebcdbceabcece");
}

void test2() {
    // 整数序列
    TrieTree<int, MODE, 0> trie;
    trie.insert({1, 2, 4, 7, 6, 6, 1});
    trie.insert({200, 10, 21, 1293, 1, 53, 2});
    trie.insert({1, 2, 4, 5, 6, 6, 1});
    trie.insert({1, 3, 4, 1, 6, 6, 1});
    for (auto i : trie) {
        for (auto j : i) cout << j << " ";
        cout << endl;
    }
    cout << trie.count({1, 2, 4, 5, 6, 6, 1}) << endl;
    trie.erase({1, 2, 4, 5, 6, 6, 1});
    auto y = trie.prefixWords({1, 2, 4});
    for (auto yy : y) {
        copy(yy.begin(), yy.end(), ostream_iterator<int>(cout, " "));
    }
}

auto regex_matche_word(const string& s) {
    regex rgx("(\\w+)");
    sregex_iterator first = sregex_iterator{s.begin(), s.end(), rgx};
    sregex_iterator last;
    vector<string> words;
    while (first != last) {
        words.push_back(first->str(1));
        first++;
    }
    return words;
}

void test3() {
    // 词频统计
    ifstream ifs("text.txt");
    string s;
    char c;
    while (~(c = ifs.get())) s += c;
    ifs.close();

    ClockTime::start_timeclock();

    TrieTree<char, MODE> trie;
    auto words = regex_matche_word(s);
    unordered_map<string, int> cnt;

    trie.insert({'1', '2', '3'});
    for (auto word : words) trie.insert(word);

    for (auto word : words) cnt[word] = trie.count(word);

    for (auto x : cnt) cout << x.first << ":" << x.second << endl;

    for (auto word : words) trie.erase(word);

    cout << trie.count("the") << endl;
    ClockTime::stop_timeclock();
    cout << ClockTime::time_duration() << endl;

    // for (auto it = trie.begin(); it != trie.end(); ++it) cout << *it << " ";
}

void test4() {
    TrieTree<wchar_t, 1> t;
    t.insert(L"你好世界");
    t.insert(L"你好吗");
    t.insert(L"linux大法好");
    for (auto x : t) cout << x << endl;
    t.clear();
    t.insert(L"你好");
    for (auto x : t) cout << x << endl;
}

int main() {
    test1();
    cout << endl;
    test2();
    cout << endl;

    test3();
    test4();
    return 0;
}
