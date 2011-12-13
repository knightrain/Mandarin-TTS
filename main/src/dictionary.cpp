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

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include "dictionary.h"
#include "phonetic_symbol.h"
#include "character.h"
#include "symbol_map.h"
#include "dict_common.h"
using namespace std;

static bool isDir(const char *path) {
	struct stat st;
	if (stat(path, &st) == 0 && (st.st_mode & S_IFDIR)) {
		return true;
	} else {
		return false;
	}
}

Dict::Dict(void) {
	init();
}

void Dict::init(void) {
	mFullPausePcmSize = 25000;

	mFullPausePcm = new char[mFullPausePcmSize];
	memset(mFullPausePcm, 0, mFullPausePcmSize);
	mFullPause = new PhoneticSymbol("fullpause");
	mFullPause->setPcm(mFullPausePcm, mFullPausePcmSize);

	mHalfPausePcm = new char[mFullPausePcmSize / 2];
	memset(mHalfPausePcm, 0, mFullPausePcmSize / 2);
	mHalfPause = new PhoneticSymbol("halfpause");
	mHalfPause->setPcm(mHalfPausePcm, mFullPausePcmSize / 2);

	mQuaterPausePcm = new char[mFullPausePcmSize / 4];
	memset(mQuaterPausePcm, 0, mFullPausePcmSize / 4);
	mQuaterPause = new PhoneticSymbol("quaterpause");
	mQuaterPause->setPcm(mQuaterPausePcm, mFullPausePcmSize / 4);
}

Dict::~Dict(void) {
	delete mFullPause;
	delete mHalfPause;
	delete mQuaterPause;

	map<string, PhoneticSymbol *>::iterator it;
	for(it=m_SymbolMap.begin();it != m_SymbolMap.end();it++)
	{
		PhoneticSymbol *phonSymbol = (*it).second;
		delete phonSymbol;
	}
}

int Dict::loadDictFile(const char *zhDict)
{
	// clear old dictionary
	for (int i = 0; i < 65536; ++i) {
		mDictItemArray[i].character.phonSymbol = NULL;
		mDictItemArray[i].wordList.clear();
	}
	mExtraDictItemMap.clear();

	if (this->loadDictionary(zhDict) != 0) {
		cerr << "Fail to load mandarin dictionary: " << zhDict << endl;
		return -1;
	}

	addSpecialSymbols();

	return 0;
}

void Dict::addSpecialSymbols(void)
{
	// full pauses
	addDictItem(10, mFullPause); // "\n"
	addDictItem(59, mFullPause); // ";"
	addDictItem(65307, mFullPause); // Chinese ";"
	addDictItem(12290, mFullPause); // Chinese "."

	// "." "..."
	addDictItem(46, mFullPause); // "."
	list<Character> * word = new list<Character>;
	word->push_back(Character(46, mQuaterPause));
	word->push_back(Character(46, mQuaterPause));
	word->push_back(Character(46, mQuaterPause));
	mDictItemArray[46].wordList.push_back(word);

	// half pauses
	addDictItem(44, mHalfPause); // ","
	addDictItem(65292, mHalfPause); // Chinese ","
	addDictItem(58, mHalfPause); // ":"
	addDictItem(65306, mHalfPause); // Chinese ":"
	addDictItem(8230, mHalfPause); // Chinese "..."

	// quater pauses
	addDictItem(45, mQuaterPause); // "-"
	addDictItem(8212, mQuaterPause); // Chinese "-"
	addDictItem(32, mQuaterPause); // " "
	addDictItem(12288, mQuaterPause); // Chinese " "
	addDictItem(39, mQuaterPause); // "'"
	addDictItem(8216, mQuaterPause); // Chinese "'"
	addDictItem(34, mQuaterPause); // '"'
	addDictItem(8220, mQuaterPause); // Chinese '"'
	addDictItem(40, mQuaterPause); // "("
	addDictItem(65288, mQuaterPause); // Chinese "("
	addDictItem(41, mQuaterPause); // ")"
	addDictItem(65289, mQuaterPause); // Chinese ")"
	addDictItem(12298, mQuaterPause); // Chinese "<<"
	addDictItem(12299, mQuaterPause); // Chinese ">>"
	addDictItem(12300, mQuaterPause); // Chinese "["
	addDictItem(12301, mQuaterPause); // Chinese "]"
	addDictItem(12302, mQuaterPause); // Chinese "[["
	addDictItem(12303, mQuaterPause); // Chinese "]]"
}

int Dict::setVoice(string voice) {
	if (isDir(voice.c_str())) {
		PhoneticSymbol *ps = lookup(0x7684);
		if (!ps) {
			cerr << "Fail to lookup(de)!" << endl;
			return -1;
		}
		int size;

		// set sfinfo
		ps->setPcm(0, 0);
		const char *pPcm = ps->getPcm(voice.c_str(), "wav", size, mSfinfo);
		if (pPcm) {
			mVoiceFileType = "wav";
		} else {
			pPcm = ps->getPcm(voice.c_str(), "gsm", size, mSfinfo);
			if (pPcm) {
				mVoiceFileType = "gsm";
			} else {
				pPcm = ps->getPcm(voice.c_str(), "ogg", size, mSfinfo);
				if (pPcm) {
					mVoiceFileType = "ogg";
				} else {
					cerr << "No voice data file is found" << endl;
					return -1;
				}
			}
		}

		if (size < 25000) {
			mFullPausePcmSize = size;
		}

		return 0;
	} else {
		return -1;
	}
}

PhoneticSymbol* Dict::lookup(Character &c) {
	if (c.code < 65536) {
		return mDictItemArray[c.code].character.phonSymbol;
	} else {
		DictItem *di = &mExtraDictItemMap[c.code];
		if (di) {
			return di->character.phonSymbol;
		} else {
			return 0;
		}
	}
}

void Dict::handleDelayedCharList(list<PhoneticSymbol*> *phonList,
		list<Character> *delayedCharList, char tone)
{
	DictItem *di = 0;
	list<PhoneticSymbol *> pl;
	PhoneticSymbol * phon;
	int lastCode = 0;

	if (delayedCharList->empty())
		return;

	while (!delayedCharList->empty()) {
		Character c = delayedCharList->front();
		if (c.code < 65536) {
			di = &mDictItemArray[c.code];
		} else {
			di = &mExtraDictItemMap[c.code];
		}

		if ((c.code == 0x4e0d || c.code == 0x4e00) && c.code != lastCode) {
			// "²»/Ò»" tone -> 2 when the next tone is 4
			string sym = di->character.phonSymbol->getSymbolStr();
			if (tone == '4') {
				tone = sym[sym.length() - 1] = '2';
				phon = findPhonSymbol(sym);
				pl.push_front(phon);
			} else { 
				pl.push_front(di->character.phonSymbol);
				tone = di->character.phonSymbol->getTone();
			}
		}
		else {
			/*TODO:
			 * there may be other special character, but we just put the
			 * defaut phonetic back here.
			 */
			pl.push_front(di->character.phonSymbol);
			tone = di->character.phonSymbol->getTone();
		}

		lastCode = c.code;
		delayedCharList->pop_front();
	}

	while(!pl.empty()) {
		phonList->push_back(pl.front());
		pl.pop_front();
	}
}

list<PhoneticSymbol*> Dict::lookup(list<Character> &charList)
{
	list<PhoneticSymbol*> phonList;
	list<Character>::iterator cItor = charList.begin();
	list<Character>::iterator cItor2 = charList.begin();
	list<Character> delayedCharList;
	DictItem *di = 0;

	while (cItor != charList.end()) {
		// get DictItem
		if (cItor->code < 65536) {
			di = &mDictItemArray[cItor->code];
		} else {
			di = &mExtraDictItemMap[cItor->code];
		}

		// TODO: handle english words
		if (! (di->character.phonSymbol)) {
			di->character.code = cItor->code; // needed?
			string s = di->character.getUtf8();
			const char *c = s.c_str();
			char *sym = new char[strlen(c) + 2];
			sym[0] = '\\';
			strcpy(sym + 1, c);
			PhoneticSymbol *unknownSymbol = new PhoneticSymbol(sym);
			di->character.phonSymbol = unknownSymbol;
		}

		// check word list
		bool foundMatchedWord = false;
		list< list<Character> *>::iterator matchedWordItor;
		if (!di->wordList.empty()) {
			unsigned int matchedWordLen = 0;
			list<list<Character> *>::iterator wordItor = di->wordList.begin();
			for (; wordItor != di->wordList.end(); ++wordItor) {
				if ((*wordItor)->size() > matchedWordLen) { // only check longer word
					// check whether current word is matched
					list<Character>::iterator charItor = (*wordItor)->begin();
					cItor2 = cItor;
					++cItor2;
					++charItor;
					while (charItor != (*wordItor)->end()
							&& cItor2 != charList.end()
							&& charItor->code == cItor2->code) {
						++charItor;
						++cItor2;
					}

					if (charItor == (*wordItor)->end()) {
						// found a longer matched word
						foundMatchedWord = true;
						matchedWordLen = (*wordItor)->size();
						matchedWordItor = wordItor;
					}
				}
			} // end of word matching for loop
		}

		if (foundMatchedWord) {
			cItor2 = (*matchedWordItor)->begin();
			handleDelayedCharList(&phonList, &delayedCharList,
					cItor2->phonSymbol->getTone());
			for (; cItor2 != (*matchedWordItor)->end(); ++cItor2) {
				phonList.push_back(cItor2->phonSymbol);
				++cItor;
			}
		} else if (cItor->code == 0x4e0d || cItor->code == 0x4e00) {
			delayedCharList.push_front(*cItor);
			cItor++;
			if (cItor == charList.end())
				handleDelayedCharList(&phonList, &delayedCharList, '0');
		} else {
			handleDelayedCharList(&phonList, &delayedCharList,
					di->character.phonSymbol->getTone());
			phonList.push_back(di->character.phonSymbol);
			++cItor;
		}
	}

	// tone 3 rules: 333->223, 33->23, 3333->2323
	list<PhoneticSymbol*>::reverse_iterator psIt = phonList.rbegin();
	while (psIt != phonList.rend()) {
		while (psIt != phonList.rend() &&
				(*psIt)->getTone() != '3') {
			psIt++;
		}

		if (psIt != phonList.rend()) {
			psIt++;
			if (psIt != phonList.rend() &&
					(*psIt)->getTone() == '3') {
				list<PhoneticSymbol*>::reverse_iterator psNextIt = psIt;
				psNextIt++;
				if (psNextIt != phonList.rend() &&
						(*psNextIt)->getTone() == '3') {
					string sym = (*psIt)->getSymbolStr();
					sym[sym.length() - 1] = '2';
					*psIt = findPhonSymbol(sym);
					psIt++;
				}
				string sym = (*psIt)->getSymbolStr();
				sym[sym.length() - 1] = '2';
				*psIt = findPhonSymbol(sym);
			}
		}
	}

	return phonList;
}

int GetFileLength(const char *filename)
{
	struct stat statbuf;
	
	if(stat(filename,&statbuf) != 0)
		return(0);
	
	if((statbuf.st_mode & S_IFMT) == S_IFDIR)
		return(-2);  // a directory
	
	return(statbuf.st_size);
}

int Dict::loadDictData(const char *zhDict, char **dict_data)
{
	int length;
	char * buffer;

	ifstream is;
	is.open(zhDict, ios::binary);
	if (is.fail())
		return 0;

	// get length of file:
	is.seekg(0, ios::end);
	length = is.tellg();
	is.seekg(0, ios::beg);

	// allocate memory:
	buffer = new char [length];

	// read data as a block:
	is.read (buffer,length);
	if (is.fail() || is.eof())
		goto out;

	*dict_data = buffer;
out:
	is.close();
	return length;
}

DictItem * Dict::getDictItem(int code)
{
	DictItem *di = NULL;
	if (code < 65536) {
		di = &mDictItemArray[code];
	} else {
		di = &mExtraDictItemMap[code];
	}

	return di;
}

int Dict::loadDictionary(const char *zhDict)
{
	int size, code_count;
	char *dict_data = NULL;
	character_code *code_array;
	int rv = 0;
	int i;

	size = loadDictData(zhDict, &dict_data);
	if (size <= 0) {
		cerr << "Failed to load dictionary data." << endl;
		return -1;
	}

	clock_t begin_clock = clock();

	code_count = size/sizeof(character_code);
	code_array = (character_code *)dict_data;
	for (i = 0; i < code_count; i++) {
		character_code *cc = &code_array[i];

		if (cc->utf8_code == -1) {
			int len = 0;
			switch(cc->symbol_code) {
				case CHARACTER_SESSION:
					for (i++; i < code_count; i++) {
						cc = &code_array[i];
						if (cc->utf8_code == -1) {
							if (cc->symbol_code == END_OF_SESSION)
								break;
							rv = -1;
							goto out;
						}

						// add DictItem
						DictItem *di = getDictItem(cc->utf8_code);
						di->character.code = cc->utf8_code;
						di->character.phonSymbol = &m_SymbolArray[cc->symbol_code]; //TODO: maybe we need to handle extra symbol code
					}
					break;
				case WORD_SESSION:
				{
					Character c;
					list<Character> *word = NULL;
					DictItem * di = NULL;
					for (i++; i < code_count; i++) {
						cc = &code_array[i];
						if (cc->utf8_code == -1) {
							if (cc->symbol_code == END_OF_WORD) {
								if (!di || !word)
									return -1;
								di->wordList.push_back(word);
								di = NULL;
								word = NULL;
								continue;
							}
							else if (cc->symbol_code == END_OF_SESSION) {
								if (word) {
									di->wordList.push_back(word);
									di = NULL;
									word = NULL;
								}
								break;
							}
							else
								return -1;
						}

						c.code = cc->utf8_code;
						c.phonSymbol = &m_SymbolArray[cc->symbol_code];
						if (!word) {
							di = getDictItem(c.code);
							word = new list<Character>;
						}
						word->push_back(c);
					}
					break;
				}
				default:
					cerr << "Wrong dictionary format." << endl;
					return -1;
			}
		}
	}

	clock_t end_clock = clock();
	cout << "clocks: " << end_clock - begin_clock <<
		", CLOCKS_PER_SEC=" << CLOCKS_PER_SEC << endl;

out:
	delete [] dict_data;
	return rv;
}

PhoneticSymbol* Dict::findPhonSymbol(string &symbol)
{
	SymbolCode *pSymCode;
	PhoneticSymbol * pPhonSym;
	pSymCode = Perfect_Hash::in_word_set(symbol.c_str(), symbol.size());
	if (pSymCode)
	{
		pPhonSym = &m_SymbolArray[pSymCode->code];
	}
	else 
	{
		if (m_SymbolMap.find(symbol) == m_SymbolMap.end()) {
			PhoneticSymbol *phonSymbol = new PhoneticSymbol(symbol);
			m_SymbolMap[symbol] = phonSymbol;
		}
		pPhonSym = m_SymbolMap[symbol];
	}
	return pPhonSym;
};

