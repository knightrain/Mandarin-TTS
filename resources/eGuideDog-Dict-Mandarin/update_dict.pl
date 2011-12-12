use strict;
use Storable;

# init dictionary
`rm -f Mandarin.dict`;
my $dict = {pinyin => {},
    chars => {},
    words => {},
    word_index => {},
};
store($dict, "Mandarin.dict");

use eGuideDog::Dict::Mandarin;
my $dict = {};
bless $dict, 'eGuideDog::Dict::Mandarin';
eGuideDog::Dict::Mandarin::update_dict($dict);

