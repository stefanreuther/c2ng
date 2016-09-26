#!/usr/bin/perl -w
#
#  Identifier name check
#
#  Checks a list of identifiers against a grammar. Use to get consistent naming.
#  Invoke as
#     perl namecheck.pl --grammar=GRAMMARFILE --root=ROOTSYMBOL
#  and pass identifiers on stdin.
#
#  Format of the grammar file:
#  - comments start with ";" or "#"
#  - rules have the form
#       <nonterminal> = <rule> | <rule...>
#    and can be broken before a '|' symbol.
#    The <nonterminal> is a sequence of letters, digits, and hyphens.
#    A <rule> is a sequence of
#    . nonterminals
#    . terminals, i.e. quoted literals ('a', "a")
#    . directives. Currently we have <lt> to match a lower-case letter even if the grammar produces an upper-case letter.
#      For example, 'rule = "x" <lt> "Abc"' will match 'xabc'.
#

use strict;

# Grammar:
#   nonterminal => [production]
#   with each production being a nonterminal or quoted word
# for example
#   var => "a" | "b" | "c" var
# is represented as
#   $grammar->{var} = [["'a"], ["'b"], ["'c", "var"]]
my $grammar = {};

my $grammarFile = 'grammar.txt';
my $root = 'root';
my $debug = 0;

foreach (@ARGV) {
    if (/^--?grammar=(.*)/) {
        $grammarFile = $1;
    } elsif (/^--?root=(.*)/) {
        $root = $1;
    } elsif (/^--?debug/) {
        $debug = 1;
    } else {
        die "$0 [--grammar=GRAMMARFILE] [--root=ROOTSYMBOL] < LIST_TO_CHECK\n";
    }
}

loadGrammar($grammar, $grammarFile);
while (<STDIN>) {
    foreach (split /\s+/) {
        if (/(.*):(.*)/) {
            checkWord($grammar, $2, $1);
        } else {
            checkWord($grammar, $root, $_);
        }
    }
}

sub checkWord {
    my ($grammar, $root, $word) = @_;
    my $result = deriveGrammar($grammar, $root, $word, length($word));
    if ($result->{state} && $result->{remain} == 0) {
        if ($debug) {
            print "$word: matches\n";
            dumpTree($result->{path}, '  ');
        }
    } else {
        print "$word: fail, state $result->{state}, remain $result->{remain}\n";
        dumpTree($result->{path}, '  ');
    }
}

sub dumpTree {
    my $p = shift;
    my $indent = shift;
    foreach (@$p) {
        if (ref) {
            print $indent, $_->{rule}, "\n";
            dumpTree($_->{content}, $indent.'  ');
        } else {
            print "$indent'$_'\n";
        }
    }
}

sub deriveGrammar {
    my ($grammar, $root, $word, $depth) = @_;

    # Result:
    # - state: 0/1
    # - remain: remaining characters in word (fewer is better)
    # - path: list of
    #     - matched literals
    #     - {rule:name, content:[path]}
    my $bestResult = {
        state=>0,
        remain=>length($word)
    };
    if ($depth <= 0) {
        # Depth limit exceeded
        return $bestResult;
    }
    foreach my $rule (@{$grammar->{$root}}) {
        my $result = checkRule($grammar, $rule, $word, $depth-1);
        if ($result->{state} > $bestResult->{state}
            || ($result->{state} = $bestResult->{state}
                && $result->{remain} < $bestResult->{remain}))
        {
            $bestResult = $result;
            last if $result->{remain} == 0;
        }
    }
    $bestResult;
}

sub checkRule {
    my ($grammar, $rule, $word, $depth) = @_;
    my $path = [];
    print "[debug] checkRule(depth=$depth, rule=[", join(' ', @$rule), "], word='$word')\n" if $debug;
    foreach (@$rule) {
        if (/^'(.*)/) {
            if (substr($word, 0, length($1)) eq $1) {
                $word = substr($word, length($1));
                push @$path, $1;
                print "[debug]   checkRule depth=$depth, word match, remaining '$word'\n" if $debug;
            } else {
                print "[debug]   checkRule depth=$depth, word mismatch\n" if $debug;
                return {state=>0, path=>$path, remain=>length($word)};
            }
        } elsif ($_ eq '<lc>') {
            my $new = ucfirst($word);
            if ($new eq $word) {
                print "[debug]   checkRule depth=$depth, <lc> mismatch at '$word'\n" if $debug;
                return {state=>0, path=>$path, remain=>length($word)};
            }
            $word = $new;
        } else {
            my $p = deriveGrammar($grammar, $_, $word, $depth);
            push @$path, {rule=>$_, content=>$p->{path}};
            $word = substr($word, length($word) - $p->{remain});
            if ($p->{state}) {
                print "[debug]   checkRule depth=$depth, subrule match, remaining '$word'\n" if $debug;
            } else {
                print "[debug]   checkRule depth=$depth, subrule mismatch\n" if $debug;
                return {state=>0, path=>$path, remain=>$p->{remain}};
            }
        }
    }
    print "[debug]   checkRule depth=$depth returns ", length($word), "\n" if $debug;
    return {state=>1, path=>$path, remain=>length($word)};
}

sub loadGrammar {
    my ($grammar, $file) = @_;
    open FILE, "< $file" or die "$file: $!";
    my $lastSymbol;
    while (<FILE>) {
        # Strip comments
        s/[#;].*//;

        # Strip space
        s/^\s+//;
        s/\s+$//;

        # Check
        if (/^$/) {
            # blank
        } elsif (/^([\w\d-]+)\s*=\s*(.*)/) {
            # Initial assignment
            $lastSymbol = $1;
            push @{$grammar->{$lastSymbol}}, parseRule($2);
        } elsif (defined($lastSymbol) && /^\|\s*(.*)/) {
            push @{$grammar->{$lastSymbol}}, parseRule($1);
        } else {
            die "$file:$.: parse error";
        }
    }
}

sub parseRule {
    my $rule = shift;
    my @result;
    foreach (split /\s*\|\s*/, $rule) {
        my $me = [];
        foreach (split /\s+/) {
            if (/^([\w\d-]+)/) {
                # Nonterminal reference
                push @$me, $1;
            } elsif (/^<(lc)>/) {
                # Directive
                push @$me, "<$1>";
            } elsif (/^'(.*)'$/ || /^"(.*)"$/) {
                # Terminal reference
                push @$me, "'$1";
            } else {
                die "parse error";
            }
        }
        push @result, $me;
    }
    @result;
}
