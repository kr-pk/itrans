#!/usr/bin/perl -w
# itrans-post.pl  ->  ITRANS post-processor
#  fix Unicode output problems, for example - input utf8, output utf8
#  currently only needed for Unicode UTF-8 Output
#---------------------------------------------------------------------
# unicode-problem
# ============
# > it could say that the
# > DEVANAGARI VOWEL SIGN CANDRA O - U-0x949 has to be used instead of
# > DEVANAGARI VOWEL SIGN CANDRA E - U - 0x945
# 
# This seems like the more likely reading. Otherwise, codepoint 0x945
# probably would've been omitted from the standard altogether.
# ============
    # CANDRA O and CANDRA E vowels,
    # aa.c and e.c should produce single Unicode characters, right now 
    # ITRANS 5.30 produces two unicode characters. [April 2008]

# Instead of changing ITRANS, which involves changes to many files, and all
# .ifm files, better to write a unicode post-processor.
# So, not following up on the changes listed below, instead creating a 
# post-processor to combine two unicode chars to 1, etc. That can be used
# to do more Unicode character processing, if needed also.
# 
# Created itrans-post.pl script to do this fixing-up.... [June 2008]
#---------------------------------------------------------------------
# Run as a filter, example:
#   itrans -U -i inputfile | itrans-post.pl -o outputfile # UTF-8
# ---
# http://www.aczoom.com/itrans/
# -------------------------------------------------------------------------

# define all the replacement patterns
my @replace_list = (
    # udvng.ifm Devanagari
    ["\x{090F}\x{0945}" => "\x{090D}"],   # ay + .c => LETTER CANDRA E
    ["\x{0947}\x{0945}" => "\x{0945}"],   # ay-matra + .c => SIGN CANDRA E
    ["\x{0906}\x{0945}" => "\x{0911}"],   # aa + .c => LETTER CANDRA O
    ["\x{093E}\x{0945}" => "\x{0949}"],   # aa-matra + .c => SIGN CANDRA O
    # uguj.ifm Gujarati
    ["\x{0A8F}\x{0AC5}" => "\x{0A8D}"],   # ay + .c => LETTER CANDRA E
    ["\x{0AC7}\x{0AC5}" => "\x{0AC5}"],   # ay-matra + .c => SIGN CANDRA E
    ["\x{0A86}\x{0AC5}" => "\x{0A91}"],   # aa + .c => LETTER CANDRA O
    ["\x{0ABE}\x{0AC5}" => "\x{0AC9}"],   # aa-matra + .c => SIGN CANDRA O
    # no CANDRA in Unicode for other languages:
    # ubeng.ifm Bengali, ugur.ifm Gurmukhi, ukan.ifm Kannada,
    # umal.ifm Malayalam, uoriya.ifm Oriya, utel.ifm Telugu, utml.ifm Tamil
);

# -------------------------------------------------------------------------
sub show_help {
    print STDERR <<EOT;
itrans-post.pl, 1.0.0
usage: $0 
   -d                           # debug, add more -d for more debug
   -i <inputfile>               # name of input file, STDIN is default
   -o <outputfile>              # name of output file, STDOUT is default
EOT
    exit(255);
}
# ---------------------------------------------------------------------
use open ':encoding(utf8)';
use Getopt::Long;

my $INPUT_FILE = "";
my $OUTPUT_FILE = ""; 
my $DEBUG = 0;

GetOptions("i=s" => \$INPUT_FILE, "o=s" => \$OUTPUT_FILE, "d+" => \$DEBUG)
    || show_help();

open(STDIN, "<", $INPUT_FILE)
    || die "Can't open input file \"$INPUT_FILE\": $!,"
    if ($INPUT_FILE);

open(STDOUT, ">", $OUTPUT_FILE)
    || die "Can't open output file \"$OUTPUT_FILE\": $!,"
    if ($OUTPUT_FILE);

binmode( STDIN,  ':utf8' );
binmode( STDOUT, ':utf8' );

# compile replacement strings/regular expressions
@replace_regexs = ();
foreach my $replace (@replace_list) {
    push(@replace_regexs, [ qr/$replace->[0]/, $replace->[1] ]);
}

# do the text substitution
while (defined($line = <STDIN>)) {
    print_chars($line, "\nInput Line: ") if ($DEBUG > 1);

    # perform replacements, pint out new line
    foreach my $replace (@replace_regexs) {
        $line =~ s/$replace->[0]/$replace->[1]/g;
    }
    print $line;

    print_chars($line, "\nOutput Line: ") if ($DEBUG > 1);
}

close(STDOUT);

exit(0);

# ---------------------------------------------------------------------
sub print_chars {
    my ($line, $tag) = @_;
    my @chars = split(//, $line);
    print STDERR $tag;
    printf STDERR " %04X", ord($_) foreach (@chars);
}
# ---------------------------------------------------------------------
