#!/usr/bin/perl -w
#
#  Function size breakdown report
#
#  Invoke as
#      perl sizereport.pl file.o file.a program
#
#  It produces a report like this:
#
#      Total   Item Size | Name
#      -------- -------- | ----------------------------------------
#          4880          | client::tiles::SelectionHeaderTile::
#                   1938 |    SelectionHeaderTile(ui::Root&, client::widgets::KeymapWidget&) [x2]
#          1434      195 |    attach(client::ObjectObserverProxy&)
#          1239          |       ::
#           475          |          Job::
#                    180 |             handle(client::tiles::SelectionHeaderTile&)
#                    295 |             ~Job() [x3]
#
#  - "Total" lists the total size of the given item and all its children. No total is given if the item is a leaf.
#  - "Item Size" lists the size of this leaf. No size is given if the item does not allocate memory (e.g. a namespace).
#    Both total and leaf shown mean that this element takes size itself (the leaf size), but has children, as in this case,
#    a function with inner classes.
#  - "[x3]" means that the compiler created 3 copies of this element (constructors, destructors).
#
#  Symbol names are broken at namespace/class name/function name boundaries and template parameters,
#  i.e. you will see how much all namespace members consume, all class members consume, all template instances consume.
#  The parser is a little whacky.
#
#  If the given files share some symbols (e.g. template instances), those will be counted as additional copies, beware!
#
use strict;

# Each element is
#   $tree->{NAME}{size}   ==> size of this element
#   $tree->{NAME}{mult}   ==> multiplicity of this element
#   $tree->{NAME}{child}  ==> hash of children
# e.g. a::b::c of size 10 is represented as
#   $tree->{a}{child}{b}{child}{c}{size} = 10
my $tree = {};
my $calls = 0;
my $objs = 0;

foreach my $f (@ARGV) {
    if ($f eq '--calls') {
        $calls = 1;
        next;
    }
    if ($f eq '--objects') {
        $objs = 1;
        next;
    }
    open PIPE, "readelf -Ws $f | c++filt |" or die "popen($f)";
    while (<PIPE>) {
        #                                     Num        Value       Size    Type    Bind    Vis   Ndx  Name
        if (my ($size,$type,$bind,$name) = /^[\s\d]+:\s*[0-9a-f]+\s+(\d+)\s+(\S+)\s+(\S+)\s+\S+\s+\S+\s+(.+)/i) {
            if (($type eq 'FUNC' || ($objs && $type eq 'OBJECT')) && $size > 0) {
                my @parsedName = parseName($name);
                my $root = $tree;
                if (@parsedName) {
                    if ($bind eq 'WEAK') {
                        push @parsedName, ' [weak]';
                    }
                    while (@parsedName > 1) {
                        my $x = shift @parsedName;
                        if (!exists $root->{$x}{child}) {
                            $root->{$x}{child} = {};
                        }
                        $root = $root->{$x}{child};
                    }
                    $root->{$parsedName[0]}{size} += $size;
                    $root->{$parsedName[0]}{mult} += 1;
                }
            }
        }
    }
    close PIPE;

    if ($calls) {
        open PIPE, "objdump -drC $f |" or die "popen($f)";
        while (<PIPE>) {
            if (/\scallq?\s+[0-9a-fx]+\s+<(.*)>/) {
                my @parsedName = parseName($1);
                my $root = $tree;
                if (@parsedName) {
                    while (@parsedName > 1) {
                        my $x = shift @parsedName;
                        if (!exists $root->{$x}{child}) {
                            $root->{$x}{child} = {};
                        }
                        $root = $root->{$x}{child};
                    }
                    $root->{$parsedName[0]}{calls} += 1;
                }
            }
        }
        close PIPE;
    }
}

# Simplify
simplify($tree);

# Build report
if ($calls) {
    print "Total   Item Size | Calls  | Name\n";
    print "-------- -------- | ------ | ----------------------------------------\n";
} else {
    print "Total   Item Size | Name\n";
    print "-------- -------- | ----------------------------------------\n";
}
printReport($tree, '');



sub parseName {
    my $name = shift;
    my @result;

    $name =~ s/std::basic_string<char,\s*std::char_traits<char>,\s*std::allocator<char>\s*>/std::string/g;
    pos($name) = 0;
    while (1) {
        if ($name =~ /\G([0-9a-z_.]+|\(anonymous namespace\))::/sgci) {
            push @result, $1, '::';
        } elsif ($name =~ /\G::/sgci) {
            push @result, '::';
        } elsif ($name =~ /\G([0-9a-z_.]+)($|(?=[*&@]))/sgci) {
            push @result, $1;
        } elsif ($name =~ /\G(vtable|typeinfo name|typeinfo|VTT|guard variable) for (.*)/sgci) {
            my $what = $1;
            push @result, parseName($2), '::', $what;
        } elsif ($name =~ /\G(non-virtual thunk) to (.*)/sgci) {
            my $what = $1;
            push @result, parseName($2), '::', $what;
        } elsif ($name =~ /\G(operator[^\(]+|operator\(\)|[^<:(]+|)\(/sgci) {
            push @result, $1
                unless $1 eq '';
            my $arg = '(';
            my $n = 1;
            while ($n > 0) {
                if ($name =~ /\G([^()]+)/sgci) {
                    $arg .= $1;
                } elsif ($name =~ /\G\(/sgci) {
                    $arg .= '(';
                    ++$n;
                } elsif ($name =~ /\G\)/sgci) {
                    $arg .= ')';
                    --$n;
                } else {
                    print STDERR "WARN: unable to parse name '$name', missing ')'\n";
                    $n = 0;
                }
            }
            while ($name =~ /\G( const| volatile)/sgci) {
                $arg .= $1;
            }
            push @result, $arg;
        } elsif ($name =~ /\G([0-9a-z_]+\s*)</sgci) {
            push @result, $1;
            my $arg = '<';
            my $n = 1;
            while ($n > 0) {
                if ($name =~ /\G([^<>]+)/sgci) {
                    $arg .= $1;
                } elsif ($name =~ /\G</sgci) {
                    $arg .= '<';
                    ++$n;
                } elsif ($name =~ /\G>/sgci) {
                    $arg .= '>';
                    --$n;
                } else {
                    print STDERR "WARN: unable to parse name '$name', missing '>'\n";
                    $n = 0;
                }
            }
            push @result, $arg;
        } elsif ($name =~ /\G$/sgci) {
            last;
        } elsif ($name =~ /\G(@\S+)\s*/sgci) {
            push @result, $1;
        } elsif ($name =~ /\G\s*(\[clone.*?\])/sgci) {
            push @result, $1;
        } elsif ($name =~ /\G\s*[&*]\s*/sgci) {
            # Means we saw a return type until here; discard everything we saw to far
            @result = ();
        } elsif ($name =~ /\G\s*(unsigned |signed |)(int|short|char|long|void|bool|string|double|float)[*&]*\s*/sgci) {
            # Return type, ignore
            @result = ();
        } else {
            my $err = substr($name, pos($name));
            print STDERR "WARN: unable to parse name '$name',\nWARN:  fails at '...$err'\n";
            #print STDERR map{"WARN:    $_\n"} @result;
            last;
        }
    }
    @result;
}


sub simplify {
    my $tree = shift;

    # Simplify all children
    foreach (keys %$tree) {
        if ($tree->{$_}{child}) {
            simplify($tree->{$_}{child});
        }
    }

    # Simplify ourselves
    my @keys = keys %$tree;
    foreach (@keys) {
        if (!$tree->{$_}{size} &&
            $tree->{$_}{child} &&
            (keys(%{$tree->{$_}{child}}) == 1))
        {
            # It has no size, and one child
            my $childName = (keys(%{$tree->{$_}{child}}))[0];
            my $newName = $_.$childName;
            if (exists $tree->{$newName}) {
                print STDERR "WARN: cannot simplify '$_' -> '$newName' because that already exists\n";
            } elsif ($newName eq $_) {
                print STDERR "WARN: cannot simplify '$_' -> '$newName' because wtf?\n";
            } else {
                $tree->{$newName} = $tree->{$_}{child}{$childName};
                delete $tree->{$_};
            }
        }
    }
}


sub printReport {
    my $tree = shift;
    my $indent = shift;
    foreach (sort keys %$tree) {
        my $selfSize = exists $tree->{$_}{size} ? $tree->{$_}{size} : "";
        my $totalSize = '';
        if (exists $tree->{$_}{child}) {
            $totalSize = $tree->{$_}{size} || 0;
            $totalSize += sumSize($tree->{$_}{child});
        }
        my $mult = exists $tree->{$_}{mult} && $tree->{$_}{mult} > 1 ? sprintf(" [x%d]", $tree->{$_}{mult}) : '';
        if ($calls) {
            printf "%8s %8s | %6s | %s%s%s\n", $totalSize, $selfSize, $tree->{$_}{calls} || '', $indent, $_, $mult;
        } else {
            printf "%8s %8s | %s%s%s\n", $totalSize, $selfSize, $indent, $_, $mult;
        }
        if (exists $tree->{$_}{child}) {
            printReport($tree->{$_}{child}, $indent.'   ');
        }
    }
}

sub sumSize {
    my $tree = shift;
    my $sum = 0;
    foreach (keys %$tree) {
        if (exists $tree->{$_}{size}) {
            $sum += $tree->{$_}{size};
        }
        if (exists $tree->{$_}{child}) {
            $sum += sumSize($tree->{$_}{child});
        }
    }
    $sum;
}
