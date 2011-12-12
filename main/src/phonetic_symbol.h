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
#ifndef PHONETIC_SYMBOL
#define PHONETIC_SYMBOL

#include <iostream>
#include <sndfile.h>
#include <string.h>

using namespace std;

typedef struct SymbolCode SymbolCode;

class PhoneticSymbol {
	public:
		PhoneticSymbol(void): mPcm(0), mSize(0) {};
		PhoneticSymbol(string &sym):
			m_Symbol(sym), mPcm(0), mSize(0) {};
		PhoneticSymbol(const char *sym):
			m_Symbol(sym), mPcm(0), mSize(0) {};

		~PhoneticSymbol(void) {
			if (mPcm) {
				delete[] mPcm;
				mPcm = 0;
			}
		};

		void setPcm(char *pcm, const int size) {
			if (mPcm) {
				delete mPcm;
			}
			mPcm = pcm;
			mSize = size;
		};

		inline const char* getPcm(int &size) {
			return getPcm("", "wav", size);
		};

		inline const char* getPcm(const char *wavDir, const char *postfix, int &size) {
			SF_INFO sfinfo;
			return getPcm(wavDir, postfix, size, sfinfo);
		};

		const char* getPcm(const char *wavDir, const char *postfix, int &size, SF_INFO &sfinfo) {
			if (!mPcm) {
				memset(&sfinfo, 0, sizeof(SF_INFO));

				string wav_file = wavDir;
				wav_file += "/";
				wav_file += this->m_Symbol;
				wav_file += ".";
				wav_file += postfix;

				SNDFILE *sndfile = sf_open(wav_file.c_str(), SFM_READ, &sfinfo);
				if (!sndfile) {
					//            cerr << "Fail to open file " << wav_file << " at " << __LINE__ <<
					//              " of " << __FILE__ << endl;                
				} else {
					sfinfo.channels = 1; // this->sfinfo.channels is corrupted
					int samples = 0;

					/* sfinfo.channels has not been taken into account .... */
					switch (sfinfo.format & SF_FORMAT_SUBMASK) {
						case SF_FORMAT_VORBIS:
						case SF_FORMAT_GSM610:
						case SF_FORMAT_PCM_16:
							mSize = (int)sfinfo.frames * 2;
							mPcm = new char[mSize];
							samples = (int)sf_readf_short(sndfile, (short int*)mPcm, sfinfo.frames);
							break;
						case SF_FORMAT_PCM_S8:
						case SF_FORMAT_PCM_U8:
						default:
							cerr << "Unknown soundfile format: " << (sfinfo.format & SF_FORMAT_SUBMASK) << endl;
					}

					if (samples != sfinfo.frames) {
						cerr << "Fail to read " << wav_file.c_str() << ": " << samples <<
							" frames out of " << sfinfo.frames << " have been read." << endl;
					}

					sf_close(sndfile);
				}
			}

			size = mSize;
			return mPcm;
		};
		string & getSymbolStr()
		{
			return m_Symbol;
		}

		char getTone()
		{
			string::reverse_iterator rit;
			rit = m_Symbol.rbegin();
			return *rit;
		}

		bool isBackslashSymbol()
		{
			return !m_Symbol.empty() && m_Symbol[0] == '\\';
		}

		bool isAlphabet()
		{
			return m_Symbol.length() == 2 &&
				m_Symbol[1] >= 65 &&
				m_Symbol[1] <=122;
		}

		char getAlphabet()
		{
			return m_Symbol[1];
		}

		int SymbolStrCmp(const char * str)
		{
			return m_Symbol.compare(str);
		}

	private:
		char *mPcm;
		int mSize;
		string m_Symbol;
};
#endif
