#ifndef _DICT_COMMON_H_
#define _DICT_COMMON_H_

enum dict_session {
	END_OF_SESSION = -1,
	EXTRA_SYMBOL_SESSION,
	CHARACTER_SESSION,
	WORD_SESSION,
	END_OF_WORD,
};

typedef struct _character_code {
	int utf8_code;
	int symbol_code;
} character_code;

#endif //_DICT_COMMON_H_
