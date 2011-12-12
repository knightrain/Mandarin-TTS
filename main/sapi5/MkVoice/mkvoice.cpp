/******************************************************************************
* mkvoice.cpp *
*-------------*
*   This application assembles a simple voice font for the sample TTS engine.
*Copyright (c) Microsoft Corporation. All rights reserved.
*
******************************************************************************/
#include "stdafx.h"
#include <ttseng_i.c>
#include <direct.h>
#include "word_list.h"

int wmain(int argc, WCHAR* argv[])
{
    USES_CONVERSION;
    static const DWORD dwVersion = { 1 };
    ULONG ulNumWords = 0;
    HRESULT hr = S_OK;

    //--- Check args
    if( argc != 5 )
    {
        printf( "%s", "Usage: > mkvoice [voice name] [word list] [dict file] [voice directory]\n" );
        hr = E_INVALIDARG;
    }
    else
    {
		MandarinList ml;
		WCHAR wszVoiceName[MAX_PATH];

		ml.load(W2A(argv[2]));
		ml.save2Dict(W2A(argv[3]));

		_snwprintf(wszVoiceName, MAX_PATH, L"RMTTS_%s", argv[1]);

        ::CoInitialize( NULL );

        //--- Register the new voice file
        //    The section below shows how to programatically create a token for
        //    the new voice and set its attributes.
        if( SUCCEEDED( hr ) )
        {
            CComPtr<ISpObjectToken> cpToken;
            CComPtr<ISpDataKey> cpDataKeyAttribs;
            hr = SpCreateNewTokenEx(
                    SPCAT_VOICES, 
                    wszVoiceName, 
                    &CLSID_SampleTTSEngine, 
                    argv[1], 
                    0x804, 
                    argv[1], 
                    &cpToken,
                    &cpDataKeyAttribs);

            //--- Set additional attributes for searching and the path to the
            //    voice data file we just created.
            if (SUCCEEDED(hr))
            {
                hr = cpDataKeyAttribs->SetStringValue(L"Gender", L"Male");
                if (SUCCEEDED(hr))
                {
                    hr = cpDataKeyAttribs->SetStringValue(L"Name", argv[1]);
                }
                if (SUCCEEDED(hr))
                {
                    hr = cpDataKeyAttribs->SetStringValue(L"Language", L"804");
                }
                if (SUCCEEDED(hr))
                {
                    hr = cpDataKeyAttribs->SetStringValue(L"Age", L"Adult");
                }
                if (SUCCEEDED(hr))
                {
                    hr = cpDataKeyAttribs->SetStringValue(L"Vendor", L"Rain Wang");
                }

                if (SUCCEEDED(hr))
                {
                    USES_CONVERSION;
					hr = cpToken->SetStringValue(L"Dict", argv[3]);
                    hr = cpToken->SetStringValue(L"Voice", argv[4]);
                }
            }
        }

        ::CoUninitialize();
    }
    return FAILED( hr );
}

