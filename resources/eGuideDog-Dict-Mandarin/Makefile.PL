use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'		    => 'eGuideDog::Dict::Mandarin',
    'VERSION_FROM'	=> 'Mandarin.pm', # finds $VERSION
    'PL_FILES'      => {'update_dict.pl' => 'no_arguments'},
    'PREREQ_PM'		=> {Encode::CNMap => 0,
        Storable => 0,
        utf8 => 0,
    }, # e.g., Module::Name => 1.1
    'PM'            => {'Mandarin.pm' => '$(INST_LIBDIR)/Mandarin.pm',
        'Mandarin.dict' => '$(INST_LIBDIR)/Mandarin/Mandarin.dict',
    },
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
        (ABSTRACT_FROM => 'Mandarin.pm', # retrieve abstract from module
            AUTHOR     => 'Cameron Wong <hgneng at yahoo.com.cn>') : ()),
);
