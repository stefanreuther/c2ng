#!/usr/bin/perl -w
#
#  Message extraction
#
#  This is an implementation of a xgettext-like tool to support our specific requirements.
#  We have not tagged everything with a "_" macro.
#  Instead, we translate function calls.
#
#       tx("foo")
#       x.translator()("foo")
#       p->translateString("foo")
#
#  Usage:
#
#     perl msgextract.pl $cpp_files > $po_file
#
use strict;
use Cwd;

my $oldcwd = Cwd::getcwd();
my %translations;

# File header. Emacs insists on a PO-Revision-Date, otherwise, it will create a new header.
$translations{''}{translation} = "PO-Revision-Date: YEAR-MO-DA HO:MI +ZONE\nContent-Type: text/plain; charset=UTF-8\n";
$translations{''}{comment} = ["Language file created by $0"];

while (defined(my $fn = shift @ARGV)) {
    if ($fn eq '-C') {
        my $dir = shift @ARGV;
        die "Missing argument to -C" if !defined $dir;
        chdir $dir or die "$dir: $!";
    } elsif ($fn eq '--reset-wd') {
        chdir $oldcwd or die "$oldcwd: $!";
    } elsif ($fn =~ /\.[ch]p+$/) {
        cpp_read($fn, \%translations);
    } elsif ($fn =~ /\.q$/) {
        script_read($fn, \%translations);
    } else {
        print STDERR "WARNING: skipping $fn\n";
    }
}
po_dump(\*STDOUT, \%translations);


#################################### po ###################################

sub po_dump {
    my ($file, $db) = @_;
    foreach my $k (sort keys %$db) {
        # Comment
        if ($db->{$k}{comment}) {
            foreach (@{$db->{$k}{comment}}) {
                print $file "# ", $_, "\n";
            }
        }

        # Locations
        if ($db->{$k}{location}) {
            foreach (sort @{$db->{$k}{location}}) {
                print $file "#: ", $_, "\n";
            }
        }

        # Translation
        po_print_str($file, 'msgid ', $k);
        po_print_str($file, 'msgstr ', $db->{$k}{translation});
        print $file "\n";
    }
}

sub po_print_str {
    my ($file, $prefix, $text) = @_;
    my @elems = split /\n/, $text, -1;
    my $last = @elems ? pop @elems : '';
    foreach (@elems) {
        print $file $prefix, "\"", po_quote($_) . "\\n\"\n";
        $prefix = '';
    }
    print $file $prefix . "\"" . po_quote($last) . "\"\n"
        unless @elems && $last eq '';
}

sub po_quote {
    my $k = shift;
    $k =~ s|\\|\\\\|g;
    $k =~ s|\"|\\\"|g;
    $k =~ s|\n|\\n|g;
    $k =~ s|\r|\\r|g;
    $k =~ s|\t|\\t|g;
    $k =~ s|([\0-\x1f])|sprintf("\\%03o", ord($1))|eg;
    $k;
}

################################# CCScript ################################

# Parser for CCScript.
# We accept only "Translate(...)" with a single string literal, where the function can also be "_" or "N_".
# No fancy line wrapping expected here.

sub script_read {
    my ($fn, $db) = @_;
    my $line_nr = 0;
    open my $file, '<', $fn or die "$fn: $!";
    while (<$file>) {
        ++$line_nr;
        while (/\b(translate|_|N_)\b\s*\(\s*('[^']*'|"([^\\"]|\\.)*")\s*\)/gi) {
            my $t = $2;
            my $str;
            if ($t =~ /^'(.*)'$/) {
                $str = $1;
            } elsif ($t =~ /^"(.*)"$/) {
                $str = unquote_string($1);
            } else {
                die "$fn:$line_nr: parse error.";
            }

            $str = trim_str($str);
            $db->{$str}{translation} ||= '';
            push @{$db->{$str}{location}}, "$fn:$line_nr";
        }
    }
    close $file;
}

################################### C++ ###################################

# Parser for C++ and related languages.
# This parser needs to be rather smart because we use messages broken over multiple lines.

sub cpp_read {
    my ($fn, $db) = @_;
    my %is_translator_invocation = (
        AFL_TRANSLATE_STRING => 1,
        tx => 1,
        translate => 1,
        translateString => 1,
        'm_translator' => 1,
        translator => 1,
        N_ => 1,
        _ => 1
    );

    my $lex = lex_new($fn);
    while (my @tok = lex($lex)) {
        if ($tok[0] eq 'id' && $is_translator_invocation{$tok[1]}) {
            # Skip sequence of () and whitespace; possible further invocations
            my $lev = 0;
            my $loc;
            while (1) {
                # I am editing these files with po-mode which unfortunately misunderstands "file:line:column" syntax,
                # so we only produce "file:line".
                $loc = lex_line_label($lex);
                @tok = lex($lex);
                last if !@tok;
                if ($tok[0] eq 'wsp') {
                    # skip
                } elsif ($tok[0] eq 'open') {
                    ++$lev;
                } elsif ($tok[0] eq 'close') {
                    last if $lev == 0;
                    --$lev;
                } elsif ($tok[0] eq 'id') {
                    $lev = 0;
                    last if !$is_translator_invocation{$tok[1]};
                } else {
                    last
                }
            }

            # Read string
            my $str = cpp_read_string($lex, \@tok);
            if (@tok && $tok[0] eq 'close' && defined($str)) {
                $str = trim_str($str);
                $db->{$str}{translation} ||= '';
                push @{$db->{$str}{location}}, $loc;
            }
        }
    }
    lex_done($lex);
}

sub cpp_read_string {
    my ($lex, $ptok) = @_;
    my $result;
    while (@$ptok) {
        if ($ptok->[0] eq 'wsp') {
            # skip
        } elsif ($ptok->[0] eq 'str') {
            # string
            $result .= unquote_string($ptok->[1]);
        } elsif ($ptok->[0] eq 'punct' && $ptok->[1] eq '+') {
            # concatenation operator (Java)
        } else {
            last;
        }
        @$ptok = lex($lex);
    }
    $result;
}

################################## Lexer ##################################

my %LEX_STATES;

sub lex_new {
    my $fn = shift;
    lex_init() if !%LEX_STATES;
    open my $file, '<', $fn
        or die "$fn: $!";
    my $line = readline($file);
    my $state = 'normal';
    return {
        file => $file,
        filename => $fn,
        line => $line,
        linenr => 1,
        state => $state
    };
}

sub lex_done {
    my $self = shift;
    close $self->{file};
}

sub lex_full_label {
    my $self = shift;
    return $self->{filename}.":".$self->{linenr}.":".(pos($self->{line}) || 1)
}

sub lex_line_label {
    my $self = shift;
    return $self->{filename}.":".$self->{linenr}
}

sub lex {
    my $self = shift;
    while (1) {
        # EOF?
        if (!defined($self->{line})) {
            return ();
        }

        # Parseable?
        my @result = $LEX_STATES{$self->{state}}->($self);
        return @result if @result;

        # End of line?
        if ($self->{line} =~ /\G[ \r\n]*$/gc) {
            $self->{line} = readline($self->{file});
            ++$self->{linenr};
        } else {
            die lex_full_label($self).": parse error";
        }
    }
}

sub lex_init {
    # Normal state
    $LEX_STATES{normal} = sub {
        my $self = shift;
        if ($self->{line} =~ /\G(\s+)/gc) {
            return ('wsp', $1);
        } elsif ($self->{line} =~ /\G"((\\.|[^\\"])*)"/gc) {
            return ('str', $1);
        } elsif ($self->{line} =~ m|\G(/\*tx\*/)|gc) {
            return ('com', $1);
        } elsif ($self->{line} =~ m|\G(//[^\r\n]*)|gc) {
            return ('com', $1);
        } elsif ($self->{line} =~ m|\G(/\*+)|gc) {
            $self->{state} = 'comment';
            return ('com', $1);
        } elsif ($self->{line} =~ m|\G\(|gc) {
            return ('open', '(');
        } elsif ($self->{line} =~ m|\G\)|gc) {
            return ('close', '(');
        } elsif ($self->{line} =~ m|\G([a-zA-Z_][a-zA-Z0-9_]+)|gc) {
            return ('id', $1);
        } elsif ($self->{line} =~ m|\G([0-9]+)|gc) {
            return ('num', $1);
        } elsif ($self->{line} =~ m|\G(.)|gc) {
            return ('punct', $1);
        } else {
            return ();
        }
    };

    # Comment
    $LEX_STATES{comment} = sub {
        my $self = shift;
        if ($self->{line} =~ m#\G(([^*]|\*+[^*/])+)#gc) {
            return ('com', $1);
        } elsif ($self->{line} =~ m|\G(\*+/)|gc) {
            $self->{state} = 'normal';
            return ('com', $1);
        } else {
            return ();
        }
    };
}

################################ Utilities ################################

sub unquote_string {
    my $x = shift;
    $x =~ s#\\(x[0-9a-fA-F]{2}|[0-7]+|.)#unquote_char($1)#eg;
    $x;
}

sub unquote_char {
    my $ch = shift;
    if ($ch =~ /^x(.+)/) {
        return chr hex($1);
    } elsif ($ch =~ /^([0-7]+)/) {
        return chr oct($1);
    } elsif ($ch eq 'r') {
        return "\r";
    } elsif ($ch eq 'n') {
        return "\n";
    } elsif ($ch eq 't') {
        return "\t";
    } elsif ($ch eq 'b') {
        return "\b";
    } else {
        return $ch;
    }
}

sub trim_str {
    my $str = shift;
    $str =~ s|[\n: ]+$||sg;
    $str;
}
