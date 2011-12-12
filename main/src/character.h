/***************************************************************************
 * Copyright (C) 2008-2010 by Cameron Wong                                 *
 * name in passport: HUANG GUANNENG                                        *
 * email: hgneng at yahoo.com.cn                                           *
 * website: http://www.eguidedog.net                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the Creative Commons GNU GPL.                     *
 *                                                                         *
 * To get a Human-Readable description of this license,                    *
 * please refer to http://creativecommons.org/licenses/GPL/2.0/            *
 *                                                                         *
 * To get Commons Deed Lawyer-Readable description of this license,        *
 * please refer to http://www.gnu.org/licenses/old-licenses/gpl-2.0.html   *
 *                                                                         *
 **************************************************************************/
#ifndef CHARACTER
#define CHARACTER

#include <string>
#include <list>
#include "phonetic_symbol.h"
#include "utf8.h"
using namespace std;

class Character {
	public:
		Character(void): code(0), phonSymbol(0) { };
		Character(unsigned int code): code(code), phonSymbol(0) {
		};
		Character(const Character &c): code(c.code), phonSymbol(c.phonSymbol) {
		};
		Character(unsigned int code, PhoneticSymbol *symbol):
			code(code),
			phonSymbol(symbol) {};
		Character(string &utf8, PhoneticSymbol *symbol):
			phonSymbol(symbol) {
				setUtf8(utf8);
			};

		unsigned int code;
		PhoneticSymbol *phonSymbol;

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
};

#endif
