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
#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include "character.h"
#include <map>
#include <list>
#include <sndfile.h>
using namespace std;

class DictItem {
	public:
		DictItem(){};
		~DictItem() {
			if (!wordList.empty()) {
				list<list<Character> *>::iterator it;
				for (it = wordList.begin(); it != wordList.end(); it++) {
					delete *it;
				}
				wordList.clear();
			}
		};

	public:
		Character character;
		list<list<Character> *> wordList;
};

class Dict {
	public:
		Dict(void);
		~Dict(void);

		string mVoiceFileType; // "wav" or "gsm"
		SF_INFO mSfinfo;
		const static int MAX_SYMBOL_ARRAY_SIZE = 3000;
		static PhoneticSymbol m_SymbolArray[MAX_SYMBOL_ARRAY_SIZE];

		int loadDictFile(const char *zhDict);
		int setVoice(string voice);
		PhoneticSymbol* lookup(Character &c);
		inline PhoneticSymbol* lookup(unsigned int code) {
			Character c(code);
			return lookup(c);
		};
		inline list<PhoneticSymbol*> lookup(string &text) {
			list<Character> charList = Character::split(text);
			return lookup(charList);
		}
		list<PhoneticSymbol*> lookup(list<Character> &charList);
		inline list<PhoneticSymbol*> lookup(const char *text) {
			string str(text);
			return lookup(str);
		};

		PhoneticSymbol* findPhonSymbol(string &symbol);
		inline PhoneticSymbol* findPhonSymbol(const char *symbol) {
			string str(symbol);
			return findPhonSymbol(str);
		}

		int loadCharList(const char *zhlist);
		int loadDictionary(const char *zhDict);
		int saveDictionary(const char *zhDict);

		inline PhoneticSymbol* getFullPause(void) { return mFullPause; };
		inline PhoneticSymbol* getHalfPause(void) { return mHalfPause; };
		inline PhoneticSymbol* getQuaterPause(void) { return mQuaterPause; };

	private:
		int loadDictData(const char *zhDict, char **dict_data);
		int loadCharSession(FILE *dict);
		int loadWordSession(FILE *dict);
		DictItem * getDictItem(int code);
		void handleDelayedCharList(list<PhoneticSymbol*> *phonList,
				list<Character> *delayedCharList, char tone);
		list<Character>::iterator handleSpecPhon(list<PhoneticSymbol*> *phonList,
				list<Character> &charList, list<Character>::iterator ci);

	private:
		/**
		 * Map character code to DictItem
		 */
		DictItem mDictItemArray[65536];
		map<int,DictItem> mExtraDictItemMap;
		map<string, PhoneticSymbol *> m_SymbolMap;

		int mFullPausePcmSize;
		char *mFullPausePcm;
		char *mHalfPausePcm;
		char *mQuaterPausePcm;
		PhoneticSymbol *mFullPause;
		PhoneticSymbol *mHalfPause;
		PhoneticSymbol *mQuaterPause;

		void init(void);
		string getDefaultDataPath(void);
		void addSpecialSymbols(void);

		inline void addDictItem(unsigned short code, PhoneticSymbol* phonSymbol) {
			mDictItemArray[code].character.code = code;
			mDictItemArray[code].character.phonSymbol = phonSymbol;
		}
};

#endif //_DICTIONARY_H_
