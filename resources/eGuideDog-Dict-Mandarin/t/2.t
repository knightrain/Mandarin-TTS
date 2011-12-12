use Test::More tests => 5;
use utf8;
use eGuideDog::Dict::Mandarin;

binmode(stdout, 'utf8');
ok(my $dict = eGuideDog::Dict::Mandarin->new());
ok(my $symbol = $dict->get_pinyin("长"));
print "长: $symbol\n";
ok($symbol = $dict->get_pinyin("长江"));
print "长江的长: $symbol\n";
ok(my @symbols = $dict->get_pinyin("拼音"));
print "拼音: @symbols\n";
ok(my @words = $dict->get_words("长"));
print "Some words begin with 长: @words\n";
