use strict;
use Storable;

my $dict = {pinyin => {},
    chars => {},
    words => {},
    word_index => {},
};
store($dict, "Mandarin.dict");
