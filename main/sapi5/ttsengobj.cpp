/*******************************************************************************
* TtsEngObj.cpp *
*---------------*
*   Description:
*       This module is the main implementation file for the CTTSEngObj class.
*-------------------------------------------------------------------------------
*  Creation Date: 03/24/99
*  Copyright (c) Microsoft Corporation. All rights reserved.
*  All Rights Reserved
*
*******************************************************************************/

//--- Additional includes
#include "stdafx.h"
#include "TtsEngObj.h"
#include "dictionary.h"
//#include "festival.h"
#define OUTPUT16BIT
//#include "sr-convert.cpp"
#include <iostream>

using namespace std;

//--- Local

/*****************************************************************************
* CTTSEngObj::FinalConstruct *
*----------------------------*
*   Description:
*       Constructor
*****************************************************************************/
HRESULT CTTSEngObj::FinalConstruct()
{
    SPDBG_FUNC( "CTTSEngObj::FinalConstruct" );
    HRESULT hr = S_OK;

	m_dict = NULL;
	mSonicStream = 0;

    return hr;
} /* CTTSEngObj::FinalConstruct */

/*****************************************************************************
* CTTSEngObj::FinalRelease *
*--------------------------*
*   Description:
*       destructor
*****************************************************************************/
void CTTSEngObj::FinalRelease()
{
    SPDBG_FUNC( "CTTSEngObj::FinalRelease" );

	delete m_dict;
	if (mSonicStream) {
		sonicDestroyStream(mSonicStream);
		mSonicStream = 0;
	}

} /* CTTSEngObj::FinalRelease */

//
//=== ISpObjectWithToken Implementation ======================================
//

/*****************************************************************************
* CTTSEngObj::SetObjectToken *
*----------------------------*
*   Description:
*       This function performs the majority of the initialization of the voice.
*   Once the object token has been provided, the filenames are read from the
*   token key and the files are mapped.
*****************************************************************************/
STDMETHODIMP CTTSEngObj::SetObjectToken(ISpObjectToken * pToken)
{
    SPDBG_FUNC( "CTTSEngObj::SetObjectToken" );
    HRESULT hr = SpGenericSetObjectToken(pToken, m_cpToken);


    //--- Map the voice data so it will be shared among all instances
    //  Note: This is a good example of how to memory map and share
    //        your voice data across instances.
    if( SUCCEEDED( hr ) )
	{
		CSpDynamicString	dstrDictPath;
		CSpDynamicString	dstrVoicePath;

		m_dict = new Dict();

		hr = m_cpToken->GetStringValue( L"Dict", &dstrDictPath );
		if( SUCCEEDED( hr ) ) {
			CHAR * path = dstrDictPath.CopyToChar();

			m_strDictFile = path;

			if (m_dict->loadDictFile(path) < 0)
				hr = E_FAIL;

			::CoTaskMemFree(path);
		}

		if( !SUCCEEDED( hr ) )
			return hr;

		hr = m_cpToken->GetStringValue( L"Voice", &dstrVoicePath );
		if( SUCCEEDED( hr ) ) {
			CHAR * path = dstrVoicePath.CopyToChar();

			m_strVoicePath = path;

			m_strTempFile = path;
			m_strTempFile += "\\tmp\\2046";

			m_dict->setVoice(path);

			::CoTaskMemFree(path);

			/* Init sonic */
			mSonicStream = sonicCreateStream(m_dict->mSfinfo.samplerate, 1);
		}

		static bool isFestivalInited = false;
		if (!isFestivalInited) {
			initFestival();
			isFestivalInited = true;
		}
	}

    return hr;
} /* CTTSEngObj::SetObjectToken */

//
//=== ISpTTSEngine Implementation ============================================
//

/*****************************************************************************
* CTTSEngObj::Speak *
*-------------------*
*   Description:
*       This is the primary method that SAPI calls to render text.
*-----------------------------------------------------------------------------
*   Input Parameters
*
*   pUser
*       Pointer to the current user profile object. This object contains
*       information like what languages are being used and this object
*       also gives access to resources like the SAPI master lexicon object.
*
*   dwSpeakFlags
*       This is a set of flags used to control the behavior of the
*       SAPI voice object and the associated engine.
*
*   VoiceFmtIndex
*       Zero based index specifying the output format that should
*       be used during rendering.
*
*   pTextFragList
*       A linked list of text fragments to be rendered. There is
*       one fragement per XML state change. If the input text does
*       not contain any XML markup, there will only be a single fragment.
*
*   pOutputSite
*       The interface back to SAPI where all output audio samples and events are written.
*
*   Return Values
*       S_OK - This should be returned after successful rendering or if
*              rendering was interrupted because *pfContinue changed to FALSE.
*       E_INVALIDARG 
*       E_OUTOFMEMORY
*
*****************************************************************************/
STDMETHODIMP CTTSEngObj::Speak( DWORD dwSpeakFlags,
                                REFGUID rguidFormatId,
                                const WAVEFORMATEX * pWaveFormatEx,
                                const SPVTEXTFRAG* pTextFragList,
                                ISpTTSEngineSite* pOutputSite )
{
	SPDBG_FUNC( "CTTSEngObj::Speak" );
	HRESULT hr = S_OK;

	//--- Check args
	if( SP_IS_BAD_INTERFACE_PTR( pOutputSite ) ||
		SP_IS_BAD_READ_PTR( pTextFragList )  )
	{
		hr = E_INVALIDARG;
	}
	else
	{
		//--- Init some vars
		m_pCurrFrag   = pTextFragList;
		while (m_pCurrFrag) {
			m_pNextChar   = m_pCurrFrag->pTextStart;
			m_pEndChar    = m_pNextChar + m_pCurrFrag->ulTextLen;
			m_ullAudioOff = 0;

			// skip bookmark
			if (m_pCurrFrag->State.eAction == SPVA_Bookmark) {
				m_pCurrFrag = m_pCurrFrag->pNext;
				continue;
			}

			// get pitch
			long pitch = m_pCurrFrag->State.PitchAdj.MiddleAdj; // [-10, 10]
			float pitchDelta = 0;
			if (pitch >= 0)
				pitchDelta = (float)pitch / 10;
			else
				pitchDelta = (float)pitch * 0.5 / 10;

			list<Character> char_list;
			LPCWSTR c = m_pCurrFrag->pTextStart;
			for (unsigned int i = 0; i < m_pCurrFrag->ulTextLen; i++) {
				char_list.push_back(unsigned short(*c));
				c++;
			}

			list<PhoneticSymbol*> phons = this->m_dict->lookup(char_list);
			bool has_unknown_char = false;
			float pause = 0;
			string unknown_str = "";
			const char *pPcm = NULL;


			for (list<PhoneticSymbol*>::iterator li = phons.begin(); li != phons.end() || has_unknown_char; li++) {
				if( !(pOutputSite->GetActions() & SPVES_ABORT) )
				{

					//--- Fire begin sentence event
					CSpEvent Event;
					Event.eEventId             = SPEI_SENTENCE_BOUNDARY;
					Event.elParamType          = SPET_LPARAM_IS_UNDEFINED;
					Event.ullAudioStreamOffset = m_ullAudioOff;
					Event.lParam               = (LPARAM)0;
					Event.wParam               = (WPARAM)m_pCurrFrag->ulTextLen;
					hr = pOutputSite->AddEvents( &Event, 1 );

					//--- Output
					if( SUCCEEDED( hr ) )
					{
						if (li != phons.end() && ((!*li) ||
									((*li)->isBackslashSymbol() ||
										(has_unknown_char && !(*li)->SymbolStrCmp("pause"))))) {
							has_unknown_char = true;

							if (*li && !(*li)->SymbolStrCmp("pause")) {
								unknown_str += " ";
							} else {
								if (*li && (*li)->isAlphabet()) {
									unknown_str += (*li)->getAlphabet();
								} else {
									// illegal characters for Festival
									unknown_str += " ";
								}
							}
						} else if (li != phons.end() && (*li) && !(*li)->SymbolStrCmp("fullpause")) {
							pause += 1;
						} else if (li != phons.end() && (*li) && !(*li)->SymbolStrCmp("halfpause")) {
							pause += 0.5;
						} else if (li != phons.end() && (*li) && !(*li)->SymbolStrCmp("quaterpause")) {
							pause += 0.25;
						} else {
							int size;
							if (has_unknown_char) {
								pPcm = getPcmFromFestival(unknown_str, size);
								if (! pPcm) {
									has_unknown_char = false;
									pPcm = m_dict->getFullPause()->getPcm(size);
								}
								unknown_str.clear();
								li--;
							} else if (pause > 0) {
								if (pause >= 1)
									pPcm = m_dict->getFullPause()->getPcm(size);
								else if (pause >= 0.5)
									pPcm = m_dict->getHalfPause()->getPcm(size);
								else
									pPcm = m_dict->getQuaterPause()->getPcm(size);
								pause = 0;
								li--;
							} else {
								pPcm = (*li)->getPcm(m_strVoicePath.c_str(), m_dict->mVoiceFileType.c_str(), size);
							}

							if (pPcm) {
								//--- We don't say anything for punctuation or control characters
								//    in this sample. 
								//--- Queue the event

								CSpEvent Event;
								Event.eEventId             = SPEI_WORD_BOUNDARY;
								Event.elParamType          = SPET_LPARAM_IS_UNDEFINED;
								Event.ullAudioStreamOffset = m_ullAudioOff;
								Event.lParam               = (LPARAM)0;
								Event.wParam               = (WPARAM)m_pCurrFrag->ulTextLen;
								hr = pOutputSite->AddEvents( &Event, 1 );

								//--- Queue the audio data
								long rate; // [-10,10]
								float rateDelta = 0;
								if (pOutputSite->GetRate(&rate) == S_OK) {
									if (rate >= 0)
										rateDelta = (float)rate * 3 / 10;
									else
										rateDelta = (float)rate * 0.8 / 10;
								}

								USHORT volume = 100; // [0, 100]
								pOutputSite->GetVolume(&volume);

								if (rateDelta != 0 || volume != 100 || pitch != 0) {
									sonicSetSpeed(mSonicStream, 1 + rateDelta);
									sonicSetVolume(mSonicStream, (float)volume / 100);
									sonicSetPitch(mSonicStream, 1 + pitchDelta);
									int frames = sonicWriteShortToStream(mSonicStream, (short*)pPcm,
											size / 2);
									if (frames == 0) {
										sonicFlushStream(mSonicStream);
									}

									const size_t BUFFER_SIZE = 65536;
									short buffer[BUFFER_SIZE]; 
									do {
										frames = sonicReadShortFromStream(mSonicStream, buffer,
												BUFFER_SIZE);
										hr = pOutputSite->Write((const void*)buffer, (ULONG)(frames * 2), 0);
										//--- Update the audio offset
										m_ullAudioOff += frames * 2;
									} while (frames == BUFFER_SIZE);
								} else {
									hr = pOutputSite->Write( (const void*)pPcm,
											(ULONG)size,
											NULL );
									//--- Update the audio offset
									m_ullAudioOff += size;
								}
							}

							if (has_unknown_char) {
								if (pPcm) {
									delete[] pPcm;
									pPcm = 0;
								}
								has_unknown_char = false;
							}
						}
					}
				}
			}

			m_pCurrFrag = m_pCurrFrag->pNext;
		}
	}

    return hr;
} /* CTTSEngObj::Speak */

/*****************************************************************************
* CTTSEngObj::GetVoiceFormat *
*----------------------------*
*   Description:
*       This method returns the output data format associated with the
*   specified format Index. Formats are in order of quality with the best
*   starting at 0.
*****************************************************************************/
STDMETHODIMP CTTSEngObj::GetOutputFormat( const GUID * pTargetFormatId, const WAVEFORMATEX * pTargetWaveFormatEx,
                                          GUID * pDesiredFormatId, WAVEFORMATEX ** ppCoMemDesiredWaveFormatEx )
{
    SPDBG_FUNC( "CTTSEngObj::GetVoiceFormat" );
    HRESULT hr = S_OK;

	if (this->m_dict->mSfinfo.samplerate == 16000) {
		hr = SpConvertStreamFormatEnum(SPSF_16kHz16BitMono, pDesiredFormatId, ppCoMemDesiredWaveFormatEx);
	} else if (this->m_dict->mSfinfo.samplerate == 44100) {
	    hr = SpConvertStreamFormatEnum(SPSF_44kHz16BitMono, pDesiredFormatId, ppCoMemDesiredWaveFormatEx);
	} else if (this->m_dict->mSfinfo.samplerate == 8000) {
		hr = SpConvertStreamFormatEnum(SPSF_8kHz16BitMono, pDesiredFormatId, ppCoMemDesiredWaveFormatEx);
	}

    return hr;
} /* CTTSEngObj::GetVoiceFormat */

// It's caller's responsibility to delete the returned pointer
const char* CTTSEngObj::getPcmFromFestival(string text, int& size) { 
//#ifdef ENABLE_FESTIVAL
#if 0
	EST_Wave wave;
	festival_text_to_wave(text.c_str(), wave);
	string wavfile = m_strTempFile + ".riff";
	wave.save(wavfile.c_str(), "riff");

	// change sample rate
	if (m_dict->mSfinfo.samplerate != 16000) {
		string outfile = wavfile + ".sr";
		sr_convert(wavfile.c_str(), 16000,
				outfile.c_str(), m_dict->mSfinfo.samplerate);
		wavfile = outfile;
	}

	SF_INFO sfinfo;
	char *pPcm = NULL;
	memset(&sfinfo, 0, sizeof(sfinfo));
	SNDFILE *sndfile = sf_open(wavfile.c_str(), SFM_READ, &sfinfo);
	if (!sndfile) {
		cerr << "Fail to open file " << wavfile << " at " << __LINE__ << endl;
		size = 0;
		return NULL;
	} else if (sfinfo.frames == 0) {
		size = 0;
		return NULL;
	} else {
		m_dict->mSfinfo.channels = 1; // this->sfinfo.channels is corrupted
		size_t samples = 0;

		/* sfinfo.channels has not been taken into account .... */
		switch (sfinfo.format & SF_FORMAT_SUBMASK) {
			case SF_FORMAT_PCM_16:
				size = (size_t) sfinfo.frames * 2;
				pPcm = new char[size];
				samples = (size_t) sf_readf_short(sndfile, (short*)pPcm, sfinfo.frames);
				break;
			case SF_FORMAT_PCM_S8:
			case SF_FORMAT_PCM_U8:
				size = (size_t) sfinfo.frames;
				pPcm = new char[size];
				samples = (size_t) sf_read_raw(sndfile, pPcm, sfinfo.frames);
				break;
			default:
				cerr << "Unkonwn soundfile format: " << sfinfo.format << endl;
		}

		if (samples != (size_t)sfinfo.frames) {
			cerr << "Fail to read " << wavfile << ": " << samples << " frames out of " << sfinfo.frames << " have been read." << endl;
		}

		sf_close(sndfile);
		return pPcm;
	}
#else
	return NULL;
#endif
}

int CTTSEngObj::initFestival(void) {
//#ifdef ENABLE_FESTIVAL
#if 0
	int heap_size = 2100000; // scheme heap size
	int load_init_files = 0; // don't load default festival init files
	TCHAR szPath[MAX_PATH];

	if (!GetModuleFileName(NULL, szPath, MAX_PATH)) {
		cerr << "Can not GetModuleFileName:" << GetLastError() << endl;
		return -1;
	}

	festival_initialize(load_init_files, heap_size);

	// set libdir of festival
	string path(szPath);
	path += "/festival/lib";
	siod_set_lval("libdir", strintern(path.c_str()));

	path = szPath;
	path += "/festival/lib/init.scm";
	festival_load_file(path.c_str());

#endif
	return 0;
}
