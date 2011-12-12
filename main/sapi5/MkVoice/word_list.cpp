#include "stdafx.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include "word_list.h"
#include "symbol_map.h"
#include "../../src/dict_common.h"
using namespace std;

SymbolCode * MandarinList::searchSymbolCode(string &symbol)
{
	SymbolCode *pSymCode;
	pSymCode = Perfect_Hash::in_word_set(symbol.c_str(), symbol.size());

	if (!pSymCode)
		cerr << "No symbol code is found" << endl;

	return pSymCode;
}

bool MandarinList::checkSpecialPhon(list<Character> *word)
{
	bool ret = false; 
	list<Character>::iterator ci;
	for (ci = word->begin(); ci != word->end(); ci++) {
		if (ci->code < 65536) {
			Character *c = &m_charArray[ci->code];
			if (c->code == 0) {
				c->code = ci->code;
				c->symbolCode = ci->symbolCode;
				continue;
			}
			if (c->symbolCode->code != ci->symbolCode->code) {
				ret = true;
				break;
			}
		}
		else {
			list<Character>::iterator ci2;
			for (ci2 = m_extraCharList.begin(); ci2 != m_extraCharList.end(); ci2++) {
				if (ci->code == ci2->code)
					break;
			}
			if (ci->symbolCode->code != ci2->symbolCode->code) {
				ret = true;
				break;
			}
		}
	}
	return ret;
}

int MandarinList::load(const char *zhlist)
{
	string line;
	bool has_syntax_error = false;

	ifstream fs(zhlist);
	if (!fs.is_open()) {
		cerr << "Fail to open file " << zhlist << " at " << __LINE__ << endl;
		return -1;
	}

	getline(fs, line);
	int linecount = 1;
	while (!fs.eof() && !fs.fail()) {
		list<Character> char_list = Character::split(line);
		if (char_list.size() > 0) {
			list<Character>::iterator c = char_list.begin();
			list<Character> word;

			// skip space
			while (c != char_list.end() && (c->code == ' ' || c->code == '\t')) {
				c++;
			}

			// skip comment line and number definition line
			// number will be defined manually
			list<Character>::iterator c2 = c;
			c2++;
			if (c != char_list.end() && c2 != char_list.end() &&
					((c->code == '/' && c2->code == '/') || (c->code == '_'))) {
				c = char_list.end();
			}
			// check whether it's a word line
			else if (c != char_list.end() && c->code == '(' && c2 != char_list.end()) {
				c++;
				c2++;
				word.push_back(c->code);
				c++;
				c2++;

				while (c->code != ')' && c2 != char_list.end()) {
					word.push_back(c->code);
					c++;
					c2++;
				}

				if (c->code == ')' && c2 != char_list.end()) {
					// skip space
					c++;
					while (c != char_list.end() && (c->code == ' ' || c->code == '\t')) {
						c++;
					}

					list<Character>::iterator ch = word.begin();

					while (c != char_list.end()) {
						// get next phonetic symbol
						string symbol = "";
						while (c != char_list.end() && !(c->code >= '0' && c->code <= '9')) {
							symbol += c->getUtf8();
							c++;
						}

						if (c != char_list.end() && c->code >= '0' && c->code <= '9') {
							symbol += c->getUtf8();
							c++;

							// asign phonetic symbol to character
							if (ch != word.end()) {
								ch->symbolCode = searchSymbolCode(symbol);
								// TODO: we need to support extra symbol
								if (!ch->symbolCode)
									break;
								ch++;
							} else {
								cerr << "Bad list format at line" << linecount << "(symbols more than characters): " << line << endl;
								has_syntax_error = true;
								break;
							}

							// skip "|" if exists
							if (c != char_list.end() && c->code == '|') {
								c++;
							}

						} else {
							cerr << "Bad list format at line" << linecount << "(symbol not ended with number): " << line << endl;
							has_syntax_error = true;
							break;
						}
					}

					if (ch != word.end()) {
						cerr << "Bad list format at line " << linecount << "(characters more than symbols): " << line << endl;
						has_syntax_error = true;
					}

					if (!has_syntax_error) {
						if (checkSpecialPhon(&word))
							m_wordList.push_back(word);
						//else {
							//list<Character>::iterator ci;
							//cout << "No special phonetic: ";
							//for(ci = word.begin(); ci != word.end(); ci++) {
								//cout << ci->getUtf8();
							//}
							//cout << endl;
						//}
					}
				} else {
					cerr << "Bad list format at line " << linecount << "(word not quoted by ')'): " << line << endl;
				}
			}

			// a char line
			else if (c2 != char_list.end() &&
					c->code > 256 && // ignore ASCII temporary
					(c2->code == ' ' || c2->code == '\t')) {
				Character c3 = *c;

				// skip space
				c++;
				while (c != char_list.end() && (c->code == ' ' || c->code == '\t')) {
					c++;
				}

				// get next phonetic symbol
				string symbol = "";
				while (c != char_list.end() && !(c->code >= '0' && c->code <= '9')) {
					symbol += c->getUtf8();
					c++;
				}

				// asign phonetic symbol to character
				if (c != char_list.end() && c->code >= '0' && c->code <= '9') {
					symbol += c->getUtf8();
					c3.symbolCode = searchSymbolCode(symbol);
					// TODO: we need to support extra symbol
					if (!c3.symbolCode)
						break;
					if (c3.code < 65536)
						m_charArray[c3.code] = c3;
					else
						m_extraCharList.push_back(c3);
				}
			}
			// bad line
			else if (c != char_list.end()) {
				cerr << "Bad list format at line " << linecount << ": " << line << endl; // debug code
			}

		}

		getline(fs, line);
		linecount++;
	}

	fs.close();
	return 0;
}

int MandarinList::save(const char *newlist)
{
	ofstream outfile(newlist);

	for (int i = 0; i < 65536; i++) {
		if (m_charArray[i].code != 0)
			outfile << m_charArray[i].getUtf8() << '\t' << m_charArray[i].symbolCode->name << endl;
	}

	list<list<Character>>::iterator wordItor;
	for (wordItor = m_wordList.begin(); wordItor != m_wordList.end(); wordItor++) {
		list<Character>::iterator c;
		for (c = wordItor->begin(); c != wordItor->end(); c++) {
			outfile << c->getUtf8() << '\t' << c->symbolCode->name << '\t';
		}
		outfile << endl;
	}

	outfile.close();

	return 0;
}

int MandarinList::save2Dict(const char *zhDict)
{
	
	ofstream dict(zhDict,ofstream::binary);
	if (!dict)
		return -1;

	character_code cc;
	/* Write mandarin characters first */
	cc.utf8_code = -1;
	cc.symbol_code = CHARACTER_SESSION;
	dict.write((char *)&cc, sizeof(cc));
	for (int i = 0; i < 65536; i++) {
		Character *c = &m_charArray[i];
		if (c->code != 0) {
			cc.utf8_code = c->code;
			cc.symbol_code = c->symbolCode->code;
			dict.write((char *)&cc, sizeof(cc));
		}
	}
	for (list<Character>::iterator ci = m_extraCharList.begin();
			ci != m_extraCharList.end();
			ci++)
	{
		cc.utf8_code = ci->code;
		cc.symbol_code = ci->symbolCode->code;
		dict.write((char *)&cc, sizeof(cc));
	}
	cc.utf8_code = -1;
	cc.symbol_code = END_OF_SESSION;
	dict.write((char *)&cc, sizeof(cc));
	
	/* Write word list */
	cc.utf8_code = -1;
	cc.symbol_code = WORD_SESSION;
	dict.write((char *)&cc, sizeof(cc));
	for (list<list<Character>>::iterator word = m_wordList.begin();
			word != m_wordList.end();
			word++)
	{
		for (list<Character>::iterator ci = word->begin();
				ci != word->end();
				ci++)
		{
			cc.utf8_code = ci->code;
			cc.symbol_code = ci->symbolCode->code;
			dict.write((char *)&cc, sizeof(cc));
		}
		cc.utf8_code = -1;
		cc.symbol_code = END_OF_WORD;
		dict.write((char *)&cc, sizeof(cc));
	}
	cc.utf8_code = -1;
	cc.symbol_code = END_OF_SESSION;
	dict.write((char *)&cc, sizeof(cc));

	dict.close();
	return 0;
}
