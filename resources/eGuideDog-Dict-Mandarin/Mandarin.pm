package eGuideDog::Dict::Mandarin;

use strict;
no warnings;
use utf8;
use Encode::CNMap;
use Storable;

require Exporter;

our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use eGuideDog::Dict::Mandarin ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	
);

our $VERSION = '0.5';


# Preloaded methods go here.

sub new() {
  my $self = {};
  $self->{pinyin} = {}; # The most probably phonetic symbol
  $self->{chars} = {}; # all phonetic symbols (array ref)
  $self->{words} = {}; # word phonetic symbols (array ref)
  $self->{word_index} = {}; # the first char to words (array ref)
  $self->{char_rate} = {};
  bless $self, __PACKAGE__;

  # load dictionary file.
  my $dir = __FILE__;
  $dir =~ s/[.]pm$//;

  if(-e "$dir/Mandarin.dict") {
    my $dict = retrieve("$dir/Mandarin.dict");
    $self->{pinyin} = $dict->{pinyin};
    $self->{chars} = $dict->{chars};
    $self->{words} = $dict->{words};
    $self->{word_index} = $dict->{word_index};
    $self->{char_rate} = $dict->{char_rate};
    $self->{symbol_size} = $dict->{symbol_size};
  }

  return $self;
}

sub update_symbol_size {
  my ($self) = @_;
  my $all = "a ai an ang ao ba bai ban bang bao bei ben beng bi bian biao bie bin bing bo bu ca cai can cang cao ce cen ceng cha chai chan chang chao che chen cheng chi chong chou chu chua chuai chuan chuang chui chun chuo ci cong cou cu cuan cui cun cuo da dai dan dang dao de dei den deng di dia dian diao die ding diu dong dou du duan dui dun duo e ei en eng er fa fan fang fei fen feng fo fou fu ga gai gan gang gao ge gei gen geng gong gou gu gua guai guan guang gui gun guo ha hai han hang hao he hei hen heng hong hou hu hua huai huan huang hui hun huo ji jia jian jiang jiao jie jin jing jiong jiu ju juan jue jun ka kai kan kang kao ke kei ken keng kong kou ku kua kuai kuan kuang kui kun kuo la lai lan lang lao le lei leng li lia lian liang liao lie lin ling liu lo long lou lu lu: luan lu:e lun luo ma mai man mang mao me mei men meng mi mian miao mie min ming miu mo mou mu na nai nan nang nao ne nei nen neng ng ni nia nian niang niao nie nin ning niu nong nou nu nu: nuan nu:e nuo o ou pa pai pan pang pao pei pen peng pi pian piao pie pin ping po pou pu qi qia qian qiang qiao qie qin qing qiong qiu qu quan que qun ran rang rao re ren reng ri rong rou ru rua ruan rui run ruo sa sai san sang sao se sen seng sha shai shan shang shao she shei shen sheng shi shou shu shua shuai shuan shuang shui shun shuo si song sou su suan sui sun suo ta tai tan tang tao te teng ti tian tiao tie ting tong tou tu tuan tui tun tuo wa wai wan wang wei wen weng wo wu xi xia xian xiang xiao xie xin xing xiong xiu xu xuan xue xun ya yan yang yao ye yi yin ying yo yong you yu yuan yue yun za zai zan zang zao ze zei zen zeng zha zhai zhan zhang zhao zhe zhei zhen zheng zhi zhong zhou zhu zhua zhuai zhuan zhuang zhui zhun zhuo zi zong zou zu zuan zui zun zuo";
  $all =~ s/u:/v/g;
  my @all_pinyin = split(' ', $all);

  foreach my $py (@all_pinyin) {
    for (1 .. 5) {
      my $pytone = $py . $_;
      system("espeak -vzh \"$pytone\" -w /tmp/espeak_size.1");
      system("espeak -vzh \"$pytone$pytone\" -w /tmp/espeak_size.2");
      my $size = (-s '/tmp/espeak_size.2') - (-s '/tmp/espeak_size.1');
      $self->{symbol_size}->{$pytone} = $size;
    }
  }
}

sub import_symbol_size {
  my ($self, $file) = @_;
  open(SYMBOL_SIZE, '<', $file);
  while (<SYMBOL_SIZE>) {
    my @pair = split(/\s/, $_);
    $self->{symbol_size}->{$pair[0]} = $pair[1];
  }
  close(SYMBOL_SIZE);
}

sub get_symbol_size {
  my ($self, $symbol) = @_;
  if ($self->{symbol_size}->{$symbol}) {
    return $self->{symbol_size}->{$symbol};
  } else {
    warn "$symbol size not exist";
    return undef;
  }
}

sub print_symbol_size_list {
  my ($self) = @_;
  foreach (sort(keys %{$self->{symbol_size}})) {
    print $_, "\t", $self->{symbol_size}->{$_}, "\n";
  }
}

sub update_dict {
	my $self = shift;

	$self->{pinyin} = {};
	$self->{chars} = {};
	$self->{words} = {};
	$self->{word_index} = {};
	$self->{char_rate} = {};

	$self->import_unihan("HanyuPinlu.txt");
	# if a character is not exist in HanyuPinlu, it will look up in Mandarin.txt. 
	$self->import_unihan("Mandarin.txt");
	$self->import_zh_list("zh_list");
	$self->import_zh_list("zh_listx");
	$self->import_char_rate("HanyuPinlu.txt");
	$self->import_symbol_size("symbol_size_list");

	my $dict = {pinyin => $self->{pinyin},
		chars => $self->{chars},
		words => $self->{words},
		word_index => $self->{word_index},
		char_rate => $self->{char_rate},
		symbol_size => $self->{symbol_size},
	};

	open(DICT, ">../Mandarin.list");	
	foreach my $char (sort keys %{$self->{chars}}) {
		print DICT ("$char");
		my $phons = $self->{chars}->{$char};
		foreach my $phon (@{$phons}) {
			print DICT (" $phon");
		}
		print DICT ("\n");
	}	
	foreach my $word (sort keys %{$self->{words}}) {
		print DICT ("($word) ");
		my $symbols = $self->{words}->{$word};
		foreach my $phon (@{$symbols}) {
			print DICT ("$phon");
		}
		print DICT ("\n");
	}	
	close(DICT);
#	store($dict, "Mandarin.dict");
}

sub import_char_rate {
  my ($self, $file) = @_;
  open(DATA_FILE, '<', $file);
  while(my $line = <DATA_FILE>) {
    chomp($line);
    my @items = split(/\s+/, $line);
    my $rate = 0;
    foreach (@items[1 .. $#items]) {
      /\((.*)\)/;
      $rate += $1;
    }
    my $char = chr(hex($items[0]));
    $self->{char_rate}->{$char} = $rate;
#    my $char_simp = utf8_to_simputf8($char);
#    if ($char_simp !~ /[?]/) {
#      $self->{char_rate}->{$char_simp} = $rate;
#    }
#    my $char_trad = utf8_to_tradutf8($char);
#    if ($char_trad !~ /[?]/) {
#      $self->{char_rate}->{$char_trad} = $rate;
#    }
  }
  close(DATA_FILE);
}

sub import_unihan {
    my ($self, $file) = @_;
    open(DATA_FILE, '<', $file);
    while(my $line = <DATA_FILE>) {
        chomp($line);
        $line = lc($line); # specific to Mandarin.txt
        my @items = split(/\s+/, $line);
        s/\(.*\)// foreach (@items); # specific to HanyuPinlu
        my $char = chr(hex($items[0]));
        my @phons = @items[1 .. $#items];
        if (not defined $self->{chars}->{$char}) {
            $self->{chars}->{$char} = \@phons;
        }
		#my $char_simp = utf8_to_simputf8($char);
		#if ($char_simp !~ /[?]/) {
		#    if (!defined $self->{chars}->{$char_simp}) {
		#        $self->{chars}->{$char_simp} = \@phons;
		#    }
		#}
		#my $char_trad = utf8_to_tradutf8($char);
		#if ($char_trad !~ /[?]/) {
		#    if (!defined $self->{chars}->{$char_trad}) {
		#        $self->{chars}->{$char_trad} = \@phons;
		#    }
		#}
    }
    close(DATA_FILE);
}

sub add_symbol {
	my ($self, $char, $symbol) = @_;

	if (not $self->{chars}->{$char}) {
		$self->{chars}->{$char} = [$symbol];
		return 1;
	} else {
		foreach (@{$self->{chars}->{$char}}) {
			if ($symbol eq $_) {
				return 0;
			}
		}
		$self->{chars}->{$char} = [@{$self->{chars}->{$char}}, $symbol];
		return 1;
	}
}

sub import_zh_list {
    my ($self, $zh_list) = @_;

    open(ZH_LIST, '<:utf8', $zh_list);
    while (my $line = <ZH_LIST>) {
        if ($line =~ /^(.)\s([^\s]*)\s$/) {
            if ($1 && $2) {
                my $ch = $1;
                my $py = $2;
                if ($py =~ /^[a-z]*[1-5]$/) {
                    $self->{pinyin}->{$ch} = $py;
                    $self->add_symbol($ch, $py);
                }
            }
        } elsif ($line =~ /^[(]([^)]*)[)]\s([^\s]*)\s$/) {
            my @chars = split(/ /, $1);
            my $phon = $2;
            my @symbols;
            if ($phon =~ /[|]/) {
                @symbols = split(/[|]/, $phon);
            } else {
                while($phon && $phon =~ /^([a-z]*[0-9])(.*)/) {
                    push(@symbols, $1);
                    $phon = $2;
                }
            }
            if ($#chars != $#symbols) {
                warn "Dictionary error:" . "@chars" . "-" . "@symbols";
                next;
            }
            my $word = join("", @chars);
            if ($self->{word_index}->{$chars[0]}) {
                push(@{$self->{word_index}->{$chars[0]}}, $word);
            } else {
                $self->{word_index}->{$chars[0]} = [$word];
            }
            $self->{words}->{$word} = \@symbols;
#            for (my $i = 0; $i <= $#chars; $i++) {
#                $self->add_symbol($chars[$i], $symbols[$i]);
#            }
        }
    }
    close(ZH_LIST);

    # add numbers
    $self->{pinyin}->{0} = 'ling2';
    $self->{pinyin}->{1} = 'yi1';
    $self->{pinyin}->{2} = 'er4';
    $self->{pinyin}->{3} = 'san1';
    $self->{pinyin}->{4} = 'si4';
    $self->{pinyin}->{5} = 'wu3';
    $self->{pinyin}->{6} = 'liu4';
    $self->{pinyin}->{7} = 'qi1';
    $self->{pinyin}->{8} = 'ba1';
    $self->{pinyin}->{9} = 'jiu3';
    my @phon0 = ('ling2');
    $self->{chars}->{0} = \@phon0;
    my @phon1 = ('yi1');
    $self->{chars}->{1} = \@phon1;
    my @phon2 = ('er4');
    $self->{chars}->{2} = \@phon2;
    my @phon3 = ('san1');
    $self->{chars}->{3} = \@phon3;
    my @phon4 = ('si4');
    $self->{chars}->{4} = \@phon4;
    my @phon5 = ('wu3');
    $self->{chars}->{5} = \@phon5;
    my @phon6 = ('liu4');
    $self->{chars}->{6} = \@phon6;
    my @phon7 = ('qi1');
    $self->{chars}->{7} = \@phon7;
    my @phon8 = ('ba1');
    $self->{chars}->{8} = \@phon8;
    my @phon9 = ('jiu3');
    $self->{chars}->{9} = \@phon9;
}

sub get_pinyin {
    my ($self, $str) = @_;

    if (not utf8::is_utf8($str)) {
        if (not utf8::decode($str)) {
            warn "$str is not in utf8 encoding.";
            return undef;
        }
    } elsif (not $str) {
        return undef;
    }

    if (wantarray) {
        my @pinyin;
        for (my $i = 0; $i < length($str); $i++) {
            my $char = substr($str, $i, 1);
            my @words = $self->get_words($char);
            my $longest_word = '';
            foreach my $word (@words) {
                if (index($str, $word) == 0) {
                    if (length($word) > length($longest_word)) {
                        $longest_word = $word;
                    }
                }
            }
            if ($longest_word) {
                push(@pinyin, @{$self->{words}->{$longest_word}});
                $i += $#{$self->{words}->{$longest_word}};
            } else {
                push(@pinyin, $self->{pinyin}->{$char});
            }
        }
        return @pinyin;
    } else {
        my $char = substr($str, 0, 1);
        my @words = $self->get_words($char);
        my $longest_word = '';
        foreach my $word (@words) {
            if (index($str, $word) == 0) {
                if (length($word) > length($longest_word)) {
                    $longest_word = $word;
                }
            }
        }
        if ($longest_word) {
            return $self->{words}->{$longest_word}->[0];
        } else {
            return $self->{pinyin}->{$char};
        }
    }
}

sub get_words {
  my ($self, $char) = @_;

  if ($self->{word_index}->{$char}) {
    return @{$self->{word_index}->{$char}};
  } else {
    return ();
  }
}

sub is_multi_phon {
  my ($self, $char) = @_;
  return $#{$self->{chars}->{$char}};
}

sub get_multi_phon {
  my ($self, $char) = @_;
  if ($self->{chars}->{$char}) {
    return @{$self->{chars}->{$char}};
  } else {
    return undef;
  }
}

sub print_phon_char_list {
  my ($self, $char) = @_;
  my $all = "a ai an ang ao ba bai ban bang bao bei ben beng bi bian biao bie bin bing bo bu ca cai can cang cao ce cen ceng cha chai chan chang chao che chen cheng chi chong chou chu chua chuai chuan chuang chui chun chuo ci cong cou cu cuan cui cun cuo da dai dan dang dao de dei den deng di dia dian diao die ding diu dong dou du duan dui dun duo e ei en eng er fa fan fang fei fen feng fo fou fu ga gai gan gang gao ge gei gen geng gong gou gu gua guai guan guang gui gun guo ha hai han hang hao he hei hen heng hong hou hu hua huai huan huang hui hun huo ji jia jian jiang jiao jie jin jing jiong jiu ju juan jue jun ka kai kan kang kao ke kei ken keng kong kou ku kua kuai kuan kuang kui kun kuo la lai lan lang lao le lei leng li lia lian liang liao lie lin ling liu lo long lou lu lu: luan lu:e lun luo ma mai man mang mao me mei men meng mi mian miao mie min ming miu mo mou mu na nai nan nang nao ne nei nen neng ng ni nia nian niang niao nie nin ning niu nong nou nu nu: nuan nu:e nuo o ou pa pai pan pang pao pei pen peng pi pian piao pie pin ping po pou pu qi qia qian qiang qiao qie qin qing qiong qiu qu quan que qun ran rang rao re ren reng ri rong rou ru rua ruan rui run ruo sa sai san sang sao se sen seng sha shai shan shang shao she shei shen sheng shi shou shu shua shuai shuan shuang shui shun shuo si song sou su suan sui sun suo ta tai tan tang tao te teng ti tian tiao tie ting tong tou tu tuan tui tun tuo wa wai wan wang wei wen weng wo wu xi xia xian xiang xiao xie xin xing xiong xiu xu xuan xue xun ya yan yang yao ye yi yin ying yo yong you yu yuan yue yun za zai zan zang zao ze zei zen zeng zha zhai zhan zhang zhao zhe zhei zhen zheng zhi zhong zhou zhu zhua zhuai zhuan zhuang zhui zhun zhuo zi zong zou zu zuan zui zun zuo";
  $all =~ s/u:/v/g;
  my @all_pinyin = split(' ', $all);

  my %phonh;
  foreach my $char (keys %{$self->{chars}}) {
    my $phons = $self->{chars}->{$char};
    foreach my $phon (@{$phons}) {
      if ($phonh{$phon}) {
        push(@{$phonh{$phon}}, $char);
      } else {
        my @p = ($char);
        $phonh{$phon} = \@p;
      }
    }
  }
  foreach my $py (@all_pinyin) {
    for (1 .. 5) {
      my $pytone = $py . $_;
      if ($phonh{$pytone}) {
        my @p1 = @{$phonh{$pytone}};
        my @p2 = sort {($self->{char_rate}->{$b} || 0) <=> ($self->{char_rate}->{$a} || 0)} @p1;
        print "$pytone: @p2\n";
#        foreach (@p2) {
#          print $_, "(", $self->{char_rate}->{$_} || 0, ") ";
#        }
#        print "\n";
      } else {
        print "$pytone:\n";
      }
    }
  }
}

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=encoding utf8

=head1 NAME

eGuideDog::Dict::Mandarin - an informal Pinyin dictionary.

=head1 SYNOPSIS

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

=head1 DESCRIPTION

This module is for looking up Pinyin of Mandarin characters or words. The dictionary is from Mandarin dictionary of espeak (http://espeak.sf.net).

The Mandarin pronunciation dictionary included with eSpeak is a compact summary of data from CEDICT and Unihan, with some corrections. Rather than include every word in the language, it includes only words that are pronounced differently from the default pronunciations of their component characters (which are also included).

=head2 EXPORT

None by default.

=head1 METHODS

=head2 new()

Initialize dictionary.

=head2 get_pinyin($str)

Return an array of Pinyin phonetic symbols of all characters in $str if it is in an array context.

Return a string of Pinyin phonetic symbol of the first character if it is not in an array context.

=head2 get_words($char)

Return an array of words which are begun with $char.

=head2 is_multi_phon($char)

Return non-zero if $char is a multi-phonetic-symbol character. The returned value plus 1 is the number of phonetic symbols the character has.

Return 0 if $char is single-phonetic-symbol character.

=head2 get_multi_phon($char)

Return an array of phonetic symbols of $char.

=head2 print_phon_char_list()

Return a list of all Pinyin phonetic symbols with all corresponding characters.

=head1 SEE ALSO

L<eGuideDog::Dict::Cantonese>, L<http://e-guidedog.sf.net>

=head1 AUTHOR

Cameron Wong, E<lt>hgneng at yahoo.com.cnE<gt>

=head1 ACKNOWLEDGMENT

Thanks to Silas S. Brown (http://people.pwf.cam.ac.uk/ssb22/) for maintaining the Mandarin dictionary file of espeak.

=head1 COPYRIGHT AND LICENSE

=over 2

=item of the module

Copyright 2008 by Cameron Wong

This library is free software; you can redistribute it and/or modify it under the same terms as Perl itself.

=item of the dictionary data

Unihan and CC-CEDICT are used in the dictionary data.

About Unihan: Copyright (c) 1996-2006 Unicode, Inc. All Rights reserved.

  Name: Unihan database
  Unicode version: 5.0.0
  Table version: 1.1
  Date: 7 July 2006

CC-CEDICT is a continuation of the CEDICT project started by Paul Denisowski in 1997 with the aim to provide a complete downloadable Chinese to English dictionary with pronunciation in pinyin for the Chinese characters. It is licensed under a Creative Commons Attribution-Share Alike 3.0 License.  http://www.mdbg.net/chindict/chindict.php?page=cedict

=back

=cut
