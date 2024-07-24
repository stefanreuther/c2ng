#!/usr/bin/perl -w
#
#  Compile message file into binary format
#
#  By using our own message compiler, we can define a portable file format,
#  and can tailor our format string validation to our format() routine.
#
#  Warnings:
#    Duplicate message id
#    Either string ends in "\n", the other doesn't
#    Either string starts with punctuation/space/tab/newline, the other doesn't
#    Either string starts or ends with particular Unicode characters, the other doesn't
#    Replacement contains control character (old special glyphs)
#    Entry still marked as fuzzy translation
#    Unknown escape sequence '...'
#  Warnings if --picky given:
#    Looks like format string but not marked as one
#    Either string ends in ".", "?", "!", the other doesn't
#  Warnings in format strings:
#    Translation uses more/less parameters than original
#    Translation uses different parameters than original
#    '%{' (if) not followed by condition number
#    Too many nested '%{' (if)
#    '%{' (if) without preceding output or test
#    '%}'/'%|' without '%{'
#    '%}'/'%|'/'%%' does not take arguments
#    Duplicate '%|' (else)
#    Argument pointer not known for '...'
#    n conditionals not closed at end
#
#  Input and output strings can contain "\uXXXX" for Unicode characters.
#  Because (at least my version of) msgmerge cannot handle this as a "real"
#  escape sequence, it must be actual text, i.e. the backslash must be
#  escaped!
#
#  The header (blank msgid) can contain a Content-Type header with a
#  character set name of either "utf-8" or "iso-8859-1". Those will be
#  used to write output in proper UTF-8.
#

use strict;
use vars qw{$line $lineno $beg_lineno $token $token_arg %strings $input $output $opt_picky $opt_allfmt};
use bytes;

sub usage {
    fail ("usage: $0 [--opts] input output\n   or: $0 languagecode\n\n".
          "Options:\n".
          "  --picky       display more warnings\n".
          "  --all-format  validate all strings as format strings\n");
}

$opt_picky = 0;
$opt_allfmt = 0;

foreach (@ARGV) {
    if (/^--picky$/) {
        $opt_picky = 1
    } elsif (/^--all-format$/) {
        $opt_allfmt = 1;
    } elsif (/^-/) {
        usage();
    } elsif (!defined $input) {
        $input = $_;
    } elsif (!defined $output) {
        $output = $_;
    } else {
        fail ("usage: $0 input output");
    }
}
if (defined $input && !defined $output && $input !~ /[^a-z0-9_]/i) {
    $output = "resource/$input.lang";
    $input = "po/$input.po";
}
if (!defined $output) {
    usage();
}

open INPUT, "< $input" or fail ("can't open $input: $!");

################################ Read Input ###############################

$line = "";
$lineno = 0;
$beg_lineno = 0;
%strings = ();

lex ();
while (defined $token) {
    my $args = '';
    if ($token eq '#,') {
        $args = $token_arg;
        lex ();
    }
    if ($token eq '#~') {
        lex();
        next;
    }
    expect ('msgid');
    $beg_lineno = $lineno;
    my $key = expect_str ();
    expect ('msgstr');
    my $value = expect_str ();
    warning ("Duplicate message id") if exists $strings{$key};
    if ($opt_picky && $args !~ /c-format/ && $key =~ /%[0-9]*[sd]/) {
        warning ("Looks like format string but not marked as one");
    }
    next if $value eq "" || $value eq $key;
    if ($args =~ /c-format/ || $opt_allfmt) {
        validate_format ($key, $value);
    }
    checkEnd($key, $value, "\n", "'\\n'");
    checkEnd($key, $value, " ", "space");
    checkEnd($key, $value, "\t", "tab");
    if ($opt_picky) {
        checkEnd($key, $value, '\.', "'.'");
        checkEnd($key, $value, '\?', "'?'");
        checkEnd($key, $value, '!', "'!'");
    }
    checkStart($key, $value, '[ .,;!?]', "space or punctuation");
    checkStart($key, $value, "\t", "tab");
    checkStart($key, $value, "\n", "'\\n'");
    foreach my $i (qw(2022 2023 2190 2191 2192 2193 2194 2195 2245 2248 2259 2264 2265 25B6 25C0 E140)) {
        # Those are: bullets, arrows, comparisons, triangles
        checkStart($key, $value, "\\\\u$i", "U+$i");
        checkEnd($key, $value, "\\\\u$i", "U+$i");
    }
    if ($value =~ /([\000-\010\013-\037])/) {
        warning ("Entry contains control character with code ".ord($1));
    }
    if ($value =~ /\\u(00[0189][0-9a-fA-F])/) {
        warning ("Entry contains Unicode control character U+".uc($1));
    }
    if ($args =~ /fuzzy/) {
        warning ("Entry still marked as fuzzy translation");
    }
    $strings{$key} = $value;
}

############################## Character Set ##############################

my $translateFlag = 0;
if (exists($strings{""}) && $strings{""} =~ /Content-Type:.*charset=(.*)/) {
    my $cs = lc($1);
    if ($cs eq 'iso-8859-1') {
        # translate
        $translateFlag = 1;
    } elsif ($cs eq 'utf-8' || $cs eq 'utf8') {
        # ok
    } else {
        warning ("Unrecognized character set '$1'");
    }
} else {
    warning ("No character set specified");
}

############################# Generate Output #############################

my $in     = "";
my $out    = "";
my $inptr  = "";
my $outptr = "";
foreach (sort keys %strings) {
    my ($instr, $outstr) = ("$_\0", "$strings{$_}\0");
    $instr = charsetTranslate($instr, $translateFlag);
    $outstr = charsetTranslate($outstr, $translateFlag);
    $inptr .= pack 'VV', length ($in), length ($instr);
    $outptr .= pack 'VV', length ($out), length ($outstr);
    $in .= $instr;
    $out .= $outstr;
}
open OUT, "> $output" or &fail ("can't create $output: $!");
binmode OUT;
print OUT "CClang0\032",
    pack ("VVVVVVV",
          length ($inptr) / 8,  # number of entries
          36,                   # input ptr pos
          36 + length ($inptr), # output ptr pos
          36 + length ($inptr) + length ($outptr),    # input data pos
          length ($in),         # input data length
          36 + length ($inptr) + length ($outptr) + length ($in),
          length ($out));
print OUT $inptr, $outptr, $in, $out;
close OUT;
exit 0;

############################### Subroutines ###############################

sub charsetTranslate {
    my ($str, $flag) = @_;
    # Translate ISO-8859-1
    if ($flag) {
        $str =~ s/([\x80-\xFF])/chr((ord($1)>>6) | 0xC0) . chr((ord($1) & 63) | 0x80)/eg;
    }
    # Translate Unicode escapes
    $str =~ s/\\u([0-9A-Fa-f]{4})/utf8(hex($1))/eg;
    $str;
}

sub utf8 {
    my $code = shift;
    if ($code < 0x80) {
        return chr($code);
    } elsif ($code < 0x800) {
        return chr(0xC0 + ($code >> 6)) . chr(0x80 + ($code & 63));
    } else {
        return chr(0xE0 + ($code >> 12)) . chr(0x80 + (($code >> 6) & 63)) . chr(0x80 + ($code & 63));
    }
}

sub checkEnd {
    my ($orig, $new, $rx, $print) = @_;
    if ($orig ne '' && $new ne '') {
        if ($orig =~ /$rx$/) {
            if ($new !~ /$rx$/) {
                warning ("Original ends in $print, translation doesn't");
            }
        } else {
            if ($new =~ /$rx$/) {
                warning ("Translation ends in $print, original doesn't");
            }
        }
    }
}

sub checkStart {
    my ($orig, $new, $rx, $print) = @_;
    if ($orig ne '' && $new ne '') {
        if ($orig =~ /^$rx/) {
            if ($new !~ /^$rx/) {
                warning ("Original starts with $print, translation doesn't");
            }
        } else {
            if ($new =~ /^$rx/) {
                warning ("Translation starts with $print, original doesn't");
            }
        }
    }
}

sub validate_format {
    my ($orig, $new) = @_;
    my $orig_phs = parse_format ('original', $orig);
    my $new_phs = parse_format ('translation', $new);

    my $count = @$orig_phs;
    if (@$orig_phs < @$new_phs) {
        warning ("Translation uses more parameters than original");
    } elsif (@$orig_phs > @$new_phs) {
        warning ("Translation does not use all parameters of original");
        $count = @$new_phs;
    }

    for (my $i = 0; $i < $count; ++$i) {
        my $o = defined $orig_phs->[$i] ? join(',', sort keys %{$orig_phs->[$i]}) : 'none';
        my $t = defined $new_phs->[$i]  ? join(',', sort keys %{$new_phs->[$i]})  : 'none';
        if ($o ne $t) {
            warning ("Translation uses different format specifiers: '$o' -> '$t'");
        }
    }
}

sub parse_format {
    my ($prefix, $str) = @_;
    my $iflevel = 0;
    my $conditions_had_else = 0;
    my @condition_argptr_exit;
    my @condition_argptr_start;
    my $arg_index = 0;
    my @arg_types;
    my $last_test;
    while ($str =~ m{%([0-9\$\- \+\'!\.]*)(.)}g) {
        my ($p, $c) = ($1, $2);
        if ($c eq '{') {
            if ($p !~ m|^!?\d+$|) { warning ("$prefix: '%{' (if) not followed by condition number"); }
            if (++$iflevel > 32) { warning ("$prefix: Too many nested '%{' (if)"); }
            if (!defined $last_test) {
                warning ("$prefix: '%{' (if) without preceding output or test");
            } else {
                $arg_types[$last_test]{cond} = 1
                  if $opt_picky;
            }
            $conditions_had_else <<= 1;
            push @condition_argptr_start, $arg_index;
            push @condition_argptr_exit, undef;
        } elsif ($c eq '}') {
            if (!$iflevel) { warning ("$prefix: '%}' (endif) without '%{'") } else { --$iflevel; }
            if ($p ne '') { warning ("$prefix: '%}' (endif) does not take arguments"); }
            if ($conditions_had_else & 1) {
                # We had an 'else', so validate that positions match
                if ($condition_argptr_exit[-1] != $arg_index) { $arg_index = undef }
            }
            $conditions_had_else >>= 1;
            pop @condition_argptr_exit;
            pop @condition_argptr_start;
        } elsif ($c eq '|') {
            if ($iflevel == 0) { warning ("$prefix: '%|' (else) without '%{'") }
            if ($p ne '') { warning ("$prefix: '%|' (else) does not take arguments"); }
            if ($conditions_had_else & 1) {
                warning ("$prefix: Duplicate '%|' (else)");
            } else {
                $conditions_had_else |= 1;
                $condition_argptr_exit[-1] = $arg_index;
                $arg_index = $condition_argptr_start[-1];
            }
        } elsif ($c eq '%') {
            if ($p ne "") { warning ("$prefix: '%%' does not take arguments"); }
        } else {
            if ($p =~ s{^([0-9]+)\$}{}) {
                $arg_index = $1;
            }
            if (!defined $arg_index) {
                warning ("$prefix: Argument pointer not known for '%$p$c'");
            } else {
                $last_test = $arg_index++;
                while (@arg_types < $arg_index) {
                    push @arg_types, {};
                }
                $arg_types[$last_test]{$c} = 1;
            }
        }
    }
    if ($iflevel != 0) {
        warning ("$prefix: $iflevel conditionals not closed at end");
    }
    \@arg_types;
}

sub expect_str {
    my $rv;
    while (defined $token && $token eq '"') {
        $rv .= $token_arg;
        lex ();
    }
    $rv;
}

sub expect {
    my $rv;
    foreach (@_) {
        $rv = $token_arg;
        if ($token ne $_) {
            fail ("expected `$_'");
        }
        lex ();
    }
    $rv;
}

sub lex {
    while (1) {
        if ($line eq "") {
            $line = <INPUT>;
            return ($token = undef) if !defined $line;
            ++$lineno;
            chomp $line;
            next;
        }
        if ($line =~ s/^\s+//) {
            next;
        } elsif ($line =~ s/^\#,\s*(.*)//) {
            ($token, $token_arg) = ("#,", $1);
            return $token;
        } elsif ($line =~ s/^\#~\s*(.*)//) {
            ($token, $token_arg) = ("#~", $1);
            return $token;
        } elsif ($line =~ s/^\#.*//) {
            next;
        } elsif ($line =~ s/^(\w+)//) {
            ($token, $token_arg) = ($1, "");
            return $token;
        } elsif ($line =~ s/^\"(([^\"\\]|\\.)*)\"//) {
            my $text = $1;
            $text =~ s/\\([0-7]{1,3}|.)/unescape ($1)/eg;
            ($token, $token_arg) = ('"', $text);
            return $token;
        } else {
            fail ("syntax error (unable to parse)");
        }
    }
}

sub unescape {
    my $what = shift;
    $what =~ /^[0-7]*$/ and return chr(oct($what));
    $what eq "n" and return "\n";
    $what eq "a" and return "\7";
    $what eq "t" and return "\t";
    $what eq "\"" and return "\"";
    $what eq "\'" and return "\'";
    $what eq "\\" and return "\\";
    warning ("Unknown escape sequence `\\$what'");
    return "?";
}

sub warning {
    print STDERR "$input:$beg_lineno: warning: @_\n";
}

sub fail {
    if (defined $lineno) {
        print STDERR "$input:$lineno: @_\n";
    } else {
        print STDERR "$0:@_\n";
    }
    exit 1;
}
