#ifndef _WORD_LIST_H_
#define _WORD_LIST_H_

#include <map>
#include <list>
#include <iostream>
#include <fstream>
#include <string.h>
#include "utf8.h"
using namespace std;

typedef struct SymbolCode SymbolCode;

class Character {
	public:
		Character(void): code(0), symbolCode(NULL) {};
		Character(unsigned int code): code(code), symbolCode(NULL) {};
		Character(const Character &c): code(c.code), symbolCode(c.symbolCode){};

		string getUtf8(void) {
			char buf[5] = {0};
			try {
				utf8::append(code, buf);
			} catch (...) {
				cerr << "code point:" << code << endl;
			}
			return string(buf);
		};

		void setUtf8(string &utf8) {
			code = utf8::peek_next(utf8.begin(), utf8.end());
		};

		static list<Character> split(string &text) {
			list<Character> char_list;
			int c;
			bool is_finish = false;
			string::iterator it = text.begin();
			string::iterator end = text.end();

			while (!is_finish && it != end) {
				try {
					c = utf8::next(it, end);
					char_list.push_back(c);
				} catch (utf8::not_enough_room &) {
					is_finish = true;
				} catch (utf8::invalid_utf8 &) {
					cerr << "Invalid UTF8 encoding" << endl;
					is_finish = true;
				}
			}

			return char_list;
		};

	public:
		SymbolCode *symbolCode;
		unsigned int code;
};

class MandarinList {
	public:
		MandarinList(void) {
			for (int i = 0; i < 65536; i++) {
				m_charArray[i].code = 0;
				m_charArray[i].symbolCode = NULL;
			}
		};
		~MandarinList(void) {};

		SymbolCode * searchSymbolCode(string &symbol);
		bool checkSpecialPhon(list<Character> *word);

		int load(const char *zhlist);
		int save(const char *newlist);	// For debug purpose
		int save2Dict(const char *zhDict);

	private:
		Character m_charArray[65536];
		list<Character> m_extraCharList;
		list<list<Character>> m_wordList;
};

#endif //_WORD_LIST_H_
