  use utf8;
  use eGuideDog::Dict::Mandarin;

  binmode(stdout, 'utf8');
  my $dict = eGuideDog::Dict::Mandarin->new();
  my @symbols = $dict->get_multi_phon("长");
  print "长(all pronunciation)：@symbols\n"; # zhang3 chang2
  my $symbol = $dict->get_pinyin("长");
  print "长(default pronunciation): $symbol\n"; # zhang3
  $symbol = $dict->get_pinyin("长江");
  print "长江的长: $symbol\n"; # chang2
  my @symbols = $dict->get_pinyin("拼音");
  print "拼音: @symbols\n"; # pin1 yin1
  my @words = $dict->get_words("长");
  print "Some words begin with 长: @words\n";
  
  print "print_phon_char_list:\n";
  $dict->print_phon_char_list();
