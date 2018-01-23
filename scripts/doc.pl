#!/usr/bin/perl -w
#
#  CCScript Manual Compiler
#
#  This extracts comments from source code and generates a XML manual.
#  Input syntax:
#
#  - a documentation block starts with
#       @q prototype (kind), prototype (kind)
#    The prototype shows how invoking this object looks like, with type annotations.
#    For example,
#       SetName name:Str
#       Int(n:Num):Int
#       ATan(x:Num, Optional y:Num):Num
#       Owner$:Int
#    The type can be given as 'void' to hide it from output. If a single word is
#    given as prototype, this is an ambiguity between a parameterless command and
#    a property. It is treated as a command if the kind includes the word "command",
#    or if the type is given as ":void".
#
#    There can be multiple kinds separated with ", ", or there can be multiple
#    prototype/kind elements (having multiple kinds and multiple prototypes does
#    not format well). There can be multiple @q lines to replicate a documentation
#    entry (normally, only one is generated).
#
#  - documentation can contain the following markup:
#    + %foo to set a word in fixed width
#    + {foo} or {label|target} to make a link.
#      . {foo} refers to an item "foo" if there is only one, or to a disambiguation
#      . {foo()} refers to an item "foo" that is a function, if there is only one,
#        or to a disambiguation.
#      . {foo (Kind)} refers to "foo" of the specified kind.
#      . {int:index:type:nnn} refers to a page by name.
#      . {@type n}, {@group n}, {@version n} refers to a type/group/version
#    + @argtype typeName (add backlink to given type; marking as argument type)
#    + @assignable (mark property assignable)
#    + @diff (make a "version differences" paragraph)
#    + @err desc (produces an error message list)
#    + @noproto (omit the automatic prototype)
#    + @retkey name:type description (generate a "Returned hash contains..." list)
#    + @rettype typeName (add backlink to given type; marking as return type)
#    + @returns (make a "Returns" paragraph)
#    + @retval type description (generate a "Returns" paragraph with a type reference)
#    + @see target, target (make list of links as "see also" at end of page)
#    + @since version, version (list versions at what this item was introduced)
#    + @todo (make a "todo" paragraph)
#    + @type typeName (add backlink to given type; marking as property type)
#    + @uses target, target (make list of links as "uses" at end of page, backlinks to the target pages)
#    + Otherwise, the text should be our pseudo-HTML; <p> tags are automatically
#      inserted, and a heuristic is implemented to insert &lt; and &amp; when needed.
#      Lines prefixed with "-" make a <ul>/<li> (continuations indented with space).
#      Lines prefixed with "|" are turned into <pre>; if @noproto is given, the first
#      such block is treated as a prototype (with links to type annotations).
#    Note that types/groups/versions can only be linked-to when they are actually used;
#    @retval/@retkey/@rettype/@argtype/@type will count as a use; a link will not.
#    Also see @used below.
#
#  - intro text for types and kinds can be written using
#      @type name
#      @group name
#      @version name
#    The following markup commands are supported:
#    + @name (?)
#    + @used (make this item used)
#    + @retkey, @retval, @err, @todo, @diff (but without their magical properties of
#      adding backlinks)
#
#  - generica pages can be written using
#      @page link,name
#    The following markup commands are supported:
#    + @in page (place this page within another / hierarchy)
#    + @retkey, @retval, @err, @todo, @diff (but without their magical properties of
#      adding backlinks)
#
#  Documentation is extracted from comments in C/C++ files
#     /* @q ....
#        blah */
#  and CCScript files
#     % @q ...
#     % blah
#  and end when the comment ends, or from text files where they are terminated with
#  a line full of dashes:
#     @q ...
#     blah
#     ----
#
#  Links we create:
#     int:index:types         Index of types
#     int:index:type:nnn      Index for type nnn
#     int:index:names         Index of names
#     int:index:versions      Index of versions
#     int:index:version:nnn   Index of items-by-version
#     int:index:groups        Index of kinds
#     int:index:group:nnn     Index of items-by-kind
#     int:name:nnn            Description of nnn if only one, disambiguation if multiple
#     int:name:nnn:kind[:n]   Description of nnn if multiple
#     int:index:names         All names
#
use strict;

# Format of an object:
#   baseNames => { baseName => [fullProto] }
#   kinds => { name => [fullProto] }
#   doc => []
#   since => []
#   see => []
#   uses => []
#   usedBy => []
#   link => nameOfPage
#   type => function|command|property
#   did => 0|1
#   assignable => 0|1
#   proto => 0|1

# baseName -> [list]
my %byBasename;
my %byLowercaseBasename;

# type -> relation -> baseName -> [list]
# relation is:
#   arg
#   result
#   type
# type -> LINK -> link
my %byDataType;

# version -> "items" -> baseName -> [list]
# version -> LINK -> link
my %bySince;

# kind -> "items" -> baseName -> [list]
# kind -> LINK
my %byKind;

# what -> name -> { doc => [],
#                   name => '',
#                   did => 0|1 }
# what is "type", "group", "version"
my %descriptions;

# List of pages in sequential order.
# Each page is { name => '',
#                title => '',
#                doc => [],
#                parent => '' }
my @pages;

# link -> 1 if used
my %usedLinks;

# Root namespace
my $namespace = 'int';

# Parse everything
print STDERR "Parsing...\n";
foreach (@ARGV) {
    if (/^-ns=(.*)/) {
        $namespace = $1;
        next;
    }
    if (/^--?help$/) {
        print "usage: $0 [-ns=NAMESPACE] FILE...\n";
        exit 0;
    }
    open FILE, "< $_" or die "$_: $!";
    while (defined(my $line = <FILE>)) {
        $line =~ s/[\r\n]//g;
        if (my ($prefix, $command, $args) = $line =~ /^(.*?)\@(q|type|group|page|version)\s+(.*)/) {
            # Read the comment
            my @args = ($args);
            my @lines;
            if ($prefix =~ m|%\s*$|) {
                # CCScript: % @q blah
                while (defined($line = <FILE>) && $line =~ m|^\s*%|) {
                    $line =~ s/[\r\n]//g;
                    if (length($line) > length($prefix)) {
                        push @lines, substr($line, length($prefix));
                    } else {
                        push @lines, '';
                    }
                }
            } elsif ($prefix =~ m|\#\s*$|) {
                # Perl: # @q blah
                while (defined($line = <FILE>) && $line =~ m|^\s*\#|) {
                    $line =~ s/[\r\n]//g;
                    if (length($line) > length($prefix)) {
                        push @lines, substr($line, length($prefix));
                    } else {
                        push @lines, '';
                    }
                }
            } elsif ($prefix =~ m|/\*+\s*|) {
                # C++: /* @q blah
                if ($args !~ s|\*+/||) {
                    while (defined($line = <FILE>)) {
                        $line =~ s/[\r\n]//g;
                        my $end = $line =~ s|\*+/||;
                        if (length($line) > length($prefix)) {
                            push @lines, substr($line, length($prefix));
                        } else {
                            push @lines, '';
                        }
                        last if $end;
                    }
                }
            } elsif ($prefix eq '') {
                # Text file: read until ---
                while (defined($line = <FILE>) && $line !~ m|^---+[\r\n]*$|) {
                    $line =~ s/[\r\n]//g;
                    push @lines, $line;
                }
            } else {
                # What?
                warn "WARNING: Unparsed: '$prefix'";
            }

            # Continuations
            while (@lines && $lines[0] =~ /^\s*\@\Q$command\E\s+(.*)/) {
                push @args, $1;
                shift @lines;
            }
            while (@lines && $lines[0] eq '') {
                shift @lines
            }

            # Strip the comment
            foreach my $L (@lines) {
                $L =~ s/\s+$//;
            }
            while (@lines && $lines[-1] eq '') {
                pop @lines
            }

            # Save it away
            if ($command eq 'q') {
                foreach (@args) {
                    processDocumentation($_, @lines);
                }
            } elsif ($command eq 'page') {
                foreach (@args) {
                    processPage($_, @lines);
                }
            } else {
                foreach (@args) {
                    processDescription($command, $_, @lines);
                }
            }
        }
    }
    close FILE;
}

# Assign links
print STDERR "Assigning links...\n";
foreach my $b (sort keys %byBasename) {
    push @{$byLowercaseBasename{$b}}, @{$byBasename{$b}};
}
foreach my $b (sort keys %byLowercaseBasename) {
    my @items = @{$byLowercaseBasename{$b}};
    if (@items == 1) {
        # Unique
        if (!exists $items[0]{link}) {
            $items[0]{link} = allocateLink($namespace.':name:'.$b);
        }
    } else {
        # Not unique
        foreach (@items) {
            if (!exists $_->{link}) {
                # Find kind
                my $kind = (sort {length($a)<=>length($b)} keys %{$_->{kinds}})[0];
                if (!defined($kind)) {
                    $kind = 'unk';
                } else {
                    $kind .= s|\s+||;
                }
                $_->{link} = allocateLink($namespace.':name:'.$b.':'.$kind);
            }
        }
    }
}
foreach my $t (sort keys %byDataType) {
    $byDataType{$t}{LINK} = allocateLink($namespace.':index:type:'.$t);
}
foreach my $v (sort keys %bySince) {
    $bySince{$v}{LINK} = allocateLink($namespace.':index:version:'.$v)
}
foreach my $k (sort keys %byKind) {
    $byKind{$k}{LINK} = allocateLink($namespace.':index:group:'.$k);
}
foreach ("group:$namespace:index:types", "$namespace:index:types",
         "group:$namespace:index:versions", "$namespace:index:versions",
         "group:$namespace:index:groups", "$namespace:index:groups",
         "group:$namespace:index:names", "$namespace:index:names")
  {
      if (allocateLink($_) ne $_) {
          warn "WARNING: unable to generate fixed link '$_'\n";
      }
  }
foreach (@pages) {
    if (allocateLink($_->{name}) ne $_->{name}) {
        warn "WARNING: unable to generate page link '$_'\n";
    }
}
foreach my $b (sort keys %byLowercaseBasename) {
    foreach my $obj (@{$byLowercaseBasename{$b}}) {
        if (!$obj->{didUses}) {
            $obj->{didUses} = 1;
            foreach my $link (@{$obj->{uses}}) {
                my $solved = resolveNameToObject($link);
                if (defined($solved)) {
                    push @{$solved->{usedBy}}, $obj
                      unless grep {$_ eq $obj} @{$solved->{usedBy}};
                }
            }
        }
    }
}

# Output
print STDERR "Generating...\n";
print "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n";
print "<!DOCTYPE help SYSTEM \"pcc2help.dtd\">\n";
print "<help>\n";

generatePages(1, '', ' ');
generatePages(0, '', ' ');

if (keys %byDataType) {
    print " <page id=\"group:$namespace:index:types\">\n";
    print "  <h1>Data Types</h1>\n";
    generateListOfTypes();
    foreach (sort keys %byDataType) {
        generateTypeIndex($_);
    }
    print " </page>\n";
}

if (keys %bySince) {
    print " <page id=\"group:$namespace:index:versions\">\n";
    print "  <h1>Version History</h1>\n";
    generateListOfVersions();
    foreach (sort {compareVersions($a,$b)} keys %bySince) {
        generateVersionIndex($_);
    }
    print " </page>\n";
}

if (keys %byKind) {
    print " <page id=\"group:$namespace:index:groups\">\n";
    print "  <h1>Groups</h1>\n";
    generateListOfGroups();
    foreach (sort keys %byKind) {
        generateGroupIndex($_);
    }
    print " </page>\n";
}

print " <page id=\"group:$namespace:index:names\">\n";
print "  <h1>Alphabetical Index</h1>\n";
generateAlphabeticalIndex();
foreach my $b (sort keys %byLowercaseBasename) {
    my @items = @{$byLowercaseBasename{$b}};
    if (@items > 1) {
        # Generate a disambiguation
        generateDisambiguation($b, @items);
    }
    foreach (@items) {
        if (!$_->{did}) {
            generateDocumentation($_);
            $_->{did} = 1
        }
    }
}
print " </page>\n";
print "</help>\n";


################################# Parsing #################################

sub processDocumentation {
    my $header = shift;
    my $obj = { baseNames => {},
                kinds => {},
                doc => [],
                since => [],
                see => [],
                uses => [],
                usedBy => [],
                didUses => 0,
                did => 0,
                assignable => 0,
                proto => 1 };

    # Process the prototype
    my $theName;
    foreach (split /\),\s+/, $header) {
        # Value is now
        #    prototype (kind, ...)
        # where the closing paren might be missing
        my $kind = '';
        if (s|\s+\(([^\)]+)\)?$||) { $kind = $1; }
        my $proto = $_;
        if ($kind eq '') {
            warn "WARNING: missing kind for '$proto'\n";
        }

        # Function or property?
        my ($name, $args, $ret, $type);
        if (($name,$args,$ret) = $proto =~ m|^([A-Z0-9a-z\$_.]+)\s*\((.*)\):?(\S*)$|) {
            # name(arg:type...):type
            $obj->{type} = 'function';
            if ($ret eq '') {
                warn "WARNING: Missing return type in '$proto', assuming 'Any'";
                $ret = 'Any';
            }
            push @{$obj->{baseNames}{$name}}, $proto;
            push @{$byBasename{$name}}, $obj;
            if ($kind !~ /internal/i) {
                push @{$byDataType{$ret}{result}{$name}}, $obj
                  unless $ret eq 'void';
                processArgumentList($args, $name, $obj);
            }
        } elsif (($name,$args) = $proto =~ m|^(\S+)\s+([^:].*)$|) {
            # name arg:type
            $obj->{type} = 'command';
            push @{$obj->{baseNames}{$name}}, $proto;
            push @{$byBasename{$name}}, $obj;
            if ($kind !~ /internal/i) {
                processArgumentList($args, $name, $obj);
            }
        } elsif (($kind =~ /\b(command|api|service)\b/i && (($name) = $proto =~ m|(^\S+)$|)) || (($name) = $proto =~ m|(^\S+):void$|)) {
            # name
            $obj->{type} = 'command';
            push @{$obj->{baseNames}{$name}}, $proto;
            push @{$byBasename{$name}}, $obj;
        } elsif ((($name,$type) = $proto =~ m|^(\S+) : (.*)$|) || (($name,$type) = $proto =~ m|^(\S+):(.*)$|)) {
            # name : type -or- name:type
            $obj->{type} = 'property';
            $type =~ s|[.()]+$||;
            push @{$obj->{baseNames}{$name}}, $proto;
            push @{$byBasename{$name}}, $obj;
            if ($kind !~ /internal/i) {
                push @{$byDataType{$type}{type}{$name}}, $obj;
            }
        } else {
            warn "WARNING: Could not understand this prototype: '$proto'";
            next
        }

        # Kinds
        foreach (split /,\s*/, $kind) {
            push @{$obj->{kinds}{$_}}, $proto;
            push @{$byKind{$_}{items}{$name}}, $obj;
        }

        # Remember the name
        if (!defined($theName)) {
            $theName = $name;
        }
    }

    # Process the documentation
    my $needBlank = 0;
    foreach (@_) {
        if (/^@@/) {
            # comment
        } elsif (/^\s*\@see\s+(.*)/) {
            push @{$obj->{see}}, split /,\s*/, $1;
            $needBlank = 1;
        } elsif (/^\s*\@uses\s+(.*)/) {
            push @{$obj->{uses}}, split /,\s*/, $1;
            $needBlank = 1;
        } elsif (/^\s*\@type\s+(.*)/) {
            push @{$byDataType{$1}{type}{$theName}}, $obj;
            $needBlank = 1;
        } elsif (/^\s*\@argtype\s+(.*)/) {
            push @{$byDataType{$1}{arg}{$theName}}, $obj;
            $needBlank = 1;
        } elsif (/^\s*\@rettype\s+(.*)/) {
            push @{$byDataType{$1}{result}{$theName}}, $obj;
            $needBlank = 1;
        } elsif (/^\s*\@noproto/) {
            $obj->{proto} = 0;
            $needBlank = 1;
        } elsif (/^\s*\@since\s+(.*)/) {
            foreach (split /,\s*/, $1) {
                push @{$obj->{since}}, $_;
                push @{$bySince{$_}{items}{$theName}}, $obj;
            }
            $needBlank = 1;
        } elsif (/^\s*\@returns?\b(.*)/) {
            # FIXME: need this here? Better in generateMarkup below.
            if (!$needBlank) {
                push @{$obj->{doc}}, '';
            }
            push @{$obj->{doc}}, '<b>Returns:</b> '.$1;
        } elsif (/^\s*\@assignable/) {
            $obj->{assignable} = 1;
            $needBlank = 1;
            if ($obj->{type} ne 'property') {
                warn "WARNING: '\@assignable' used on '$theName' which is not a property";
            }
        } else {
            if (/^\s*\@retkey\s+\S+:([^\s\[]+)/ || /^\s*\@retval\s+([^\s\[]+)/) {
                # @retkey name:type description
                # @retval type description
                push @{$byDataType{$1}{result}{$theName}}, $obj
                  unless grep {$_ eq $obj} @{$byDataType{$1}{result}{$theName}};
            }
            if (/^\s*\@key\s+\S+:([^\s\[]+)/) {
                # @key name:type description
                push @{$byDataType{$1}{type}{$theName}}, $obj
                  unless grep {$_ eq $obj} @{$byDataType{$1}{type}{$theName}};
            }
            if ($needBlank) {
                push @{$obj->{doc}}, '';
            }
            push @{$obj->{doc}}, $_;
            $needBlank = 0;
        }
    }
}


sub processArgumentList {
    my ($args, $name, $obj) = @_;
    if (defined($args) && $args ne '') {
        my %did;
        foreach (split /[,=\s]+/, $args) {
            s|[.()]+$||;
            if (/:([^\[\]\(\)]+)/) {
                if (!exists $did{$1}) {
                    push @{$byDataType{$1}{arg}{$name}}, $obj
                      unless $1 eq 'void';
                    $did{$1} = 1;
                }
            }
        }
    }
}

sub processDescription {
    my $what = shift;
    my $name = shift;
    my $result =  { doc => [],
                    did => 0,
                    name => $name };
    foreach (@_) {
        if (/^\@name\s+(.*)/) {
            $result->{name} = $1;
        } elsif (/^\@used$/) {
            my $parent = ($what eq 'type' ? \%byDataType : $what eq 'group' ? \%byKind : \%bySince);
            if (!exists $parent->{$name}) {
                $parent->{$name} = {};
            }
        } else {
            push @{$result->{doc}}, $_;
        }
    }
    $descriptions{$what}{$name} = $result;
}

sub processPage {
    my $name = shift;
    my $title = $name;
    if ($name =~ s|,\s*(.*)||) {
        $title = $1;
    }
    my $result = { doc => [],
                   did => 0,
                   name => $name,
                   title => $title,
                   parent => '' };

    # Process the documentation
    my $needBlank = 0;
    foreach (@_) {
        if (/^\s*\@in\s+(.*)/) {
            $result->{parent} = $1;
            $needBlank = 1;
        } elsif ($_ eq '') {
            $needBlank = 1
        } else {
            if ($needBlank) {
                push @{$result->{doc}}, '';
            }
            push @{$result->{doc}}, $_;
            $needBlank = 0;
        }
    }
    push @pages, $result;
}

################################ Generators ###############################

sub generateListOfTypes {
    generateTopIndex($namespace.':index:types', 'Index of Data Types', \%byDataType, sort {lc($a) cmp lc($b)} keys %byDataType);
}

sub generateTypeIndex {
    my $n = shift;
    print "  <page id=\"".escape($byDataType{$n}{LINK})."\">\n";
    print "   <h1>Data Type <em>".escape($n)."</em></h1>\n";
    generateDescription('type', $n);
    generateTypeIndexPart($n, 'arg', 'Commands and Functions taking <em>'.escape($n).'</em> as Parameter');
    generateTypeIndexPart($n, 'result', 'Functions returning <em>'.escape($n).'</em>');
    generateTypeIndexPart($n, 'type', 'Properties of type <em>'.escape($n).'</em>');
    print "  </page>\n";
}

sub generateTypeIndexPart {
    my ($n, $key, $head) = @_;
    my @list = sort {lc($a) cmp lc($b)} keys %{$byDataType{$n}{$key}};
    if (@list) {
        print "    <h2>".$head."</h2>\n";
        generateIndex('    ', $byDataType{$n}{$key}, @list);
    }
}

sub generateListOfVersions {
    generateTopIndex($namespace.':index:versions', 'Index of Versions', \%bySince, sort {compareVersions($b,$a)} keys %bySince);
}

sub generateVersionIndex {
    my $n = shift;
    print "  <page id=\"".escape($bySince{$n}{LINK})."\">\n";
    print "   <h1>Version ".escape($n)."</h1>\n";
    generateDescription('version', $n);
    print "   <p>Items introduced in this version:</p>\n";
    generateIndex(' ', $bySince{$n}{items}, sort keys %{$bySince{$n}{items}});
    print "  </page>\n";
}

sub generateListOfGroups {
    generateTopIndex($namespace.':index:groups', 'Index of Groups', \%byKind, sort {lc($a) cmp lc($b)} keys %byKind);
}

sub generateGroupIndex {
    my $n = shift;
    print "  <page id=\"".escape($byKind{$n}{LINK})."\">\n";
    print "   <h1>".escape($n)."</h1>\n";
    generateDescription('group', $n);
    generateIndex(' ', $byKind{$n}{items}, sort {lc($a) cmp lc($b)} keys %{$byKind{$n}{items}});
    print "  </page>\n";
}

sub generateAlphabeticalIndex {
    print "  <page id=\"$namespace:index:names\">\n";
    print "   <h1>Alphabetical Index</h1>\n";
    generateIndex(' ', \%byBasename, sort {lc($a) cmp lc($b)} keys %byBasename);
    print "  </page>\n";
}

sub generatePages {
    my ($filter, $key, $indent) = @_;
    foreach my $p (@pages) {
        if (!$p->{did} && (!$filter || $p->{parent} eq $key)) {
            # Red tape
            if (!$filter) {
                warn "WARNING: page '$p->{name}' has unknown parent '$p->{parent}'";
            }
            $p->{did} = 1;

            # Generate
            print $indent."<page id=\"".escape($p->{name})."\">\n";
            print $indent." <h1>".escape($p->{title})."</h1>\n";
            generateMarkup($p->{doc}, "\@page $p->{name}");

            # Generate children
            if ($filter) {
                generatePages(1, $p->{name}, $indent.' ');
            }

            print $indent."</page>\n";
        }
    }
}

sub generateDescription {
    my $what = shift;
    my $name = shift;
    if (!exists $descriptions{$what}{$name}) {
        warn "WARNING: no description for '\@$what $name'";
    } else {
        generateMarkup($descriptions{$what}{$name}{doc}, "\@$what $name");
        $descriptions{$what}{$name}{did} = 1;
    }
}

sub generateDisambiguation {
    my $n = shift;
    print "  <page id=\"".escape($namespace.":name:".lc($n))."\">\n";
    print "   <h1>".escape(properName($_[0], $n))." - Disambiguation</h1>\n";
    print "   <p>There are multiple items with this name:</p>\n";
    print "   <ul class=\"compact\">\n";
    foreach (@_) {
        my $kind = join (', ', sort {length($a)<=>length($b)} keys %{$_->{kinds}});
        if ($kind ne '') {
            print "    <li><a href=\"".escape($_->{link})."\">".escape(properName($_, $n))."</a> (".escape($kind).")</li>\n";
        } else {
            print "    <li><a href=\"".escape($_->{link})."\">".escape(properName($_, $n))."</a></li>\n";
        }
    }
    print "   </ul>\n";
    print "  </page>\n";
}

sub generateDocumentation {
    my $p = shift;
    print "  <page id=\"".escape($p->{link})."\">\n";
    print "   <h1>".escape(join(', ', sort {lc($a) cmp lc($b)} keys %{$p->{baseNames}})). " (".escape(join(', ', sort {lc($a) cmp lc($b)} keys %{$p->{kinds}})).")</h1>\n";

    # Show the prototype
    if ($p->{proto}) {
        # print " <p>";
        # my $first = 1;
        # foreach (sort keys %{$p->{baseNames}}) {
        #     foreach (@{$p->{baseNames}{$_}}) {
        #         print "<br />\n" unless $first;
        #         print "<tt>&#160;&#160;".generatePrototype($_)."</tt>";
        #         $first = 0;
        #     }
        # }
        # print "</p>\n";

        print "   <pre class=\"ccscript\">";
        foreach (sort {lc($a) cmp lc($b)} keys %{$p->{baseNames}}) {
            foreach (@{$p->{baseNames}{$_}}) {
                my $pfx = '';
                my $proto = $_;
                $proto =~ s!\boptional\s+(([^\)]|\([^\)]*\))*)![$1]!i;
                while (length($proto) > 53 && $proto =~ s|^(.{10,53}),\s*||) {
                    print "\n".$pfx.generatePrototype($1.',');
                    $pfx = '  ';
                }
                print "\n".$pfx.generatePrototype($proto);
                if ($p->{type} eq 'property') {
                    my $n = 43 - length($pfx.$proto);
                    if ($n < 2) { $n = 2 }
                    print "<font color=\"blue\">", " " x $n, ($p->{assignable} ? "(read/write)" : "(read-only)"), "</font>";
                }
            }
        }
        print "</pre>\n";
    }

    # Documentation
    generateMarkup($p->{doc}, (sort {lc($a) cmp lc($b)} keys %{$p->{baseNames}})[0]);

    # Since
    if (@{$p->{since}}) {
        print "   <p><b>Since: </b>",
          join(', ', map{"<a href=\"".escape($bySince{$_}{LINK})."\">".escape($_)."</a>"} sort @{$p->{since}}),
            "</p>\n";
    }

    # See also
    if (@{$p->{see}}) {
        print "   <p><b>See also: </b>",
          join(', ', map {processLink($_)} @{$p->{see}}),
            "</p>\n";
    }

    # Uses
    if (@{$p->{uses}}) {
        print "   <p><b>Uses: </b>",
          join(', ', map {processLink($_)} @{$p->{uses}}),
            "</p>\n";
    }

    # Used by
    if (@{$p->{usedBy}}) {
        print "   <p><b>Used by: </b>",
          join(', ', map{'<a href="'.escape($_->{link}).'">'.escape((sort keys %{$_->{baseNames}})[0]).'</a>'} @{$p->{usedBy}}),
            "</p>\n";
    }

    print "  </page>\n";
}

########################### Generator Utilities ###########################

sub generateTopIndex {
    my $name = shift;
    my $head = shift;
    my $pHash = shift;
    print "  <page id=\"".escape($name)."\">\n";
    print "   <h1>".escape($head)."</h1>\n";
    if ($name eq 'int:index:versions') {
        # Do not refer to $namespace here because the text is specific to the PCC2 interpreter index.
        print "   <p>This lists all versions where the interpreter or a related function changed.\n";
        print "    The PCC 1.x version notes have been copied from the PCC 1.x scripting manual.\n";
        print "    Note that not all PCC2 1.99.x versions implicitly support all features of the\n";
        print "    lower-numbered 1.x versions, i.e. there can be things an 1.x version can do that\n";
        print "    a 1.99.x version can not.</p>\n";
    }
    print "   <ul class=\"compact\">\n";
    foreach (@_) {
        print "    <li><a href=\"".escape($pHash->{$_}{LINK})."\">".escape($_)."</a></li>\n";
    }
    print "   </ul>\n";
    print "  </page>\n";
}

sub generateIndex {
    my $pfx = shift;
    my $hash = shift;
    print $pfx."<ul class=\"compact\">\n";
    foreach my $e (@_) {
        my @elems = @{$hash->{$e}};
        if (@elems == 1) {
            print $pfx." <li><a href=\"".escape($elems[0]{link})."\">".escape($e)."</a></li>\n";
        } else {
            print $pfx." <li>".escape($e)."\n";
            print $pfx."  <ul>\n";
            foreach (@elems) {
                my $kind = join (', ', sort {length($a)<=>length($b)} keys %{$_->{kinds}});
                if ($kind ne '') {
                    print $pfx."   <li><a href=\"".escape($_->{link})."\">".escape($e)."</a> (".escape($kind).")</li>\n";
                } else {
                    print $pfx."   <li><a href=\"".escape($_->{link})."\">".escape($_->{baseNames}{$e}[0])."</a></li>\n";
                }
            }
            print $pfx."  </ul>\n";
            print $pfx." </li>\n";
        }
    }
    print $pfx."</ul>\n";
}

sub generatePrototype {
    my $p = shift;
    my $result = '';
    #foreach (split /(:[^\s.,\[\]\(\)]+)/, $p) {
    #    if (/^:void(.*)/) {
    #        $result .= escape($1);
    #    } elsif (/^:(.*)/ && exists $byDataType{$1}) {
    #        $result .= '<font color="dim">:<a href="'.escape($byDataType{$1}{LINK}).'">'.escape($1).'</a></font>';
    #    } else {
    #        $result .= escape($_);
    #    }
    #}
    #$result =~ s!([\[\]]+|\.\.+)!<font color="dim">$1</font>!g;

    pos($p) = 0;
    while (1) {
        if ($p =~ m|\G( : )(\w+)((\(\)?)?)|sgc || $p =~ m|\G(:)(\w+)((\(\)?)?)|sgc) {
            # Type
            $result .= '<font color="dim">'.$1;
            if (exists $byDataType{$2}) {
                $result .= '<a href="'.escape($byDataType{$2}{LINK}).'">'.escape($2).'</a>';
            } else {
                $result .= escape($2);
            }
            $result .= escape($3);
            $result .= '</font>';
        } elsif ($p =~ m!\G(\.\.+|[\[\]])!sgc) {
            # Meta
            $result .= '<font color="dim">'.escape($1).'</font>';
        } elsif ($p =~ m|\G([\w.\$:]+)(?= : )|sgc) {
            # Non-reserved word containing ":"
            $result .= escape($1);
        } elsif ($p =~ m|\G([A-Z][\w.\$]*)|sgc) {
            # Reserved word
            $result .= '<b>'.escape($1).'</b>';
        } elsif ($p =~ m|\G([a-z][\w.\$]*)|sgc) {
            # Non-reserved word
            $result .= escape($1);
        } elsif ($p =~ m|\G(.)|sgc) {
            $result .= escape($1);
        } else {
            last;
        }
    }
    $result;
}

sub generateMarkup {
    my $p = shift;
    my $name = shift;
    # 0 = outside;                  no tags open,  cursor at beginning of line
    # 1 = inside implicit <p>,      <p> open,      cursor at end of line
    # 2 = inside explicit tag,      <$tag> open,   cursor at end of line
    # 3 = inside implicit <ul>,     <ul><li> open, cursor at end of line
    # 4 = inside implicit <pre>,    <pre> open,    cursor at end of line
    # 5 = inside list-of-@retkey,   <ul><li> open, cursor at end of line
    # 6 = inside list-of-@err,      <ul><li> open, cursor at end of line
    # 7 = inside list-of-@key,      <ul><li> open, cursor at end of line
    my $state = 0;
    my $tag;
    my $firstPre = 1;
    my @closer = ("",
                  "</p>\n",
                  "can't happen",
                  "</li>\n    </ul>\n",
                  "</pre>\n",
                  "</li>\n    </ul>\n",
                  "</li>\n    </ul>\n",
                  "</li>\n    </ul>\n");
    foreach (@$p) {
        if (/^\@c(\W|$)/) {
            # comment, ignore
            next;
        } elsif ($state == 2) {
            # Explicit <$tag>
            if (m|^</$tag|) {
                print processMarkup($_)."\n";
                $state = 0;
            } else {
                print "\n".processMarkup($_);
            }
        } elsif (/^<(pre|ul|ol|dl|table|p|h[1-6])/) {
            # Starting explicit tag
            print $closer[$state];
            $state = 2;
            $tag = $1;
            print "    ", processMarkup($_);
        } elsif (/^\s*\@retkey\s+(\S+):([^\s\[]+)([\[\]]*)(.*)/) {
            # Start or continue @retkey list
            if ($state != 5) {
                print $closer[$state];
                print "    <p><b>Returned hash:</b></p>\n";
                print "    <ul>\n";
                $state = 5;
            } else {
                print "</li>\n";
            }
            generateRetKeyLine($1, $2, $3, $4);
        } elsif (/^\s*\@key\s+(\S+):([^\s\[]+)([\[\]]*)(.*)/) {
            # Start or continue @key list
            if ($state != 7) {
                print $closer[$state];
                print "    <p><b>Hash keys:</b></p>\n";
                print "    <ul>\n";
                $state = 7;
            } else {
                print "</li>\n";
            }
            generateRetKeyLine($1, $2, $3, $4);
        } elsif (/^\s*\@retval\s+([^\s\[]+)([\[\]]*)(.*)/) {
            # Return value; implicit start of a paragraph
            print $closer[$state];
            print "    <p><b>Return value</b> (", processTypeLink($1), $2, "):", processMarkup($3);
            $state = 1;
        } elsif (/^\s*\@err\s*(.*)/) {
            # Start or continue @error list
            if ($state != 6) {
                print $closer[$state];
                print "    <p><b>Errors:</b></p>\n";
                print "    <ul>\n";
                $state = 6;
            } else {
                print "</li>\n";
            }
            print "     <li>", processMarkup($1);
        } elsif (/^-\s+(.*)/) {
            # Start or continue implicit <ul> list
            if ($state != 3) {
                print $closer[$state];
                print "    <ul>\n";
                $state = 3;
            } else {
                print "</li>\n";
            }
            print "     <li>", processMarkup($1);
        } elsif ($closer[$state] =~ m|</li>| && /^\s+(.*)/) {
            # Continue implicit list
            print "\n     ", processMarkup($1);
        } elsif (/^\|(.*)/) {
            # Start or continue implicit <pre>
            if ($state != 4) {
                print $closer[$state];
                print "    <pre class=\"ccscript\">";
                $state = 4;
            } else {
                print "\n";
            }
            if ($firstPre) {
                print generatePrototype($1);
            } else {
                print processMarkup($1);
            }
        } elsif ($_ ne '') {
            # Start or continue implicit <p>
            if ($state != 1) {
                print $closer[$state];
                $state = 1;
                print "    <p>";
            } else {
                print "\n    ";
            }
            print processMarkup($_);
        } else {
            print $closer[$state];
            $state = 0;
        }
        if ($state != 0 && $state != 4) {
            $firstPre = 0;
        }
        if ($state == 2 && m|\S\s*</$tag>|) {
            print "\n";
            $state = 0;
        }
    }

    if ($state == 2) {
        warn "WARNING: '<$tag>' not closed at end of documentation for '$name'\n";
        print "</$tag>\n";
    } else {
        print $closer[$state];
    }
}

sub generateRetKeyLine {
    my ($name, $type, $array, $rest) = @_;
    print "     <li><tt>", escape($name), '<font color="dim">:', processTypeLink($type), escape($array), "</font></tt>", processMarkup($rest);
}

sub processTypeLink {
    my $type = shift;
    if (exists $byDataType{$type}) {
        return '<a href="'.escape($byDataType{$type}{LINK}).'">'.escape($type).'</a>';
    } else {
        return escape($type);
    }
}

sub processMarkup {
    my $t = shift;
    $t =~ s|\@todo|<b>TODO:</b>|g;
    $t =~ s|\@diff|<b>Version Differences:</b>|g;
    $t =~ s!%(([A-Za-z0-9_\$]|\.[A-Za-z0-9_\$])+(\(\))?)!<tt>$1</tt>!g;
    $t =~ s|<(?=[^/!A-Za-z])|&lt;|g;
    $t =~ s|&(?=[^\#A-Za-z])|&amp;|g;
    $t =~ s|\{(.*?)\}|processLink($1)|eg;
    $t =~ s|\bEMPTY\b|<font color="dim">EMPTY</font>|g;
    $t;
}

sub processLink {
    # Parse link
    my $orig = shift;
    my $link = $orig;
    my $text = $orig;
    my $kind = '';
    my $func = 0;
    my $hadText = 0;
    if ($link =~ s!\|(.*)!!) {
        $text = $1;
        $hadText = 1;
    }
    if ($link =~ s!\s+\((.+)\)$!!) {
        $kind = $1;
    }
    if ($link =~ s|\(\)$||) {
        $func = 1;
    }

    # Resolve link
    if ($link =~ /^@(type|group|version)\s+(.*)/) {
        if (!$hadText) {
            $text = $2
        }
        my $obj = ($1 eq 'type' ? $byDataType{$2} : $1 eq 'group' ? $byKind{$2} : $bySince{$2});
        if (!defined $obj) {
            warn "WARNING: Cannot resolve link '$orig', not found";
            return escape($text);
        } else {
            return "<a href=\"".escape($obj->{LINK})."\">".escape($text)."</a>";
        }
    } elsif (exists $byBasename{$link}) {
        my @candidates = grep { ($kind eq '' || exists $_->{kinds}{$kind})
                                  && ($func == 0 || $_->{type} eq 'function') } @{$byBasename{$link}};
        if (@candidates == 0) {
            # Error case
            warn "WARNING: Cannot resolve link '$orig', '$link' exists, but not with kind '$kind'";
            return escape($text);
        } elsif (@candidates == 1) {
            # Success case
            return "<a href=\"".escape($candidates[0]{link})."\">".escape($text)."</a>";
        } else {
            # Ambiguous
            return "<a href=\"".escape($namespace.":name:".lc($link))."\">".escape($text)."</a>";
        }
    } elsif ($link =~ m|^\S+:\S+$|) {
        if (!exists $usedLinks{$link}) {
            warn "WARNING: no page named '$link' (proceeding anyway)\n";
        }
        return "<a href=\"".escape($link)."\">".escape($text)."</a>";
    } else {
        warn "WARNING: Cannot resolve link '$orig', not found";
        return escape($text);
    }
}

# Resolve a name to an object.
# This does NOT support link formats "{@type foo}", "{name|text}", or "foo:bar:baz",
# so what remains are formats "name", "name (kind)" and "name()".
sub resolveNameToObject {
    # Parse link
    my $orig = shift;
    my $link = $orig;
    my $kind = '';
    my $func = 0;
    if ($link =~ s!\s+\((.+)\)$!!) {
        $kind = $1;
    }
    if ($link =~ s|\(\)$||) {
        $func = 1;
    }

    # Resolve link
    if (exists $byBasename{$link}) {
        my @candidates = grep { ($kind eq '' || exists $_->{kinds}{$kind})
                                  && ($func == 0 || $_->{type} eq 'function') } @{$byBasename{$link}};
        if (@candidates != 1) {
            # Ambiguous or type mismatch
            warn "WARNING: Cannot resolve link '$orig', no unique match'";
            return undef;
        } else {
            # Success case
            return $candidates[0];
        }
    } else {
        warn "WARNING: Cannot resolve link '$orig', not found";
        return undef;
    }
}

################################ Utilities ################################

sub allocateLink {
    my $pfx = shift; $pfx = lc($pfx); $pfx =~ s|\s+||g;
    my $hyp = $pfx;
    my $n = 0;
    while (exists $usedLinks{$hyp}) {
        $hyp = $pfx.':'.++$n;
    }
    $usedLinks{$hyp} = 1;
    $hyp;
}

sub escape {
    my $x = shift;
    $x =~ s|&|&amp;|g;
    $x =~ s|<|&lt;|g;
    $x =~ s|<|&gt;|g;
    $x =~ s|\"|&quot;|g;
    $x;
}

sub compareVersions {
    my @as = split /(\d+|.)/, shift;
    my @bs = split /(\d+|.)/, shift;
    my $i = 0;
    while ($i < @as && $i < @bs && $as[$i] eq $bs[$i]) { ++$i }
    return ($i >= @as || $i >= @bs ? @as <=> @bs
            : $as[$i] =~ /\d/ && $bs[$i] =~ /\d/ ? $as[$i] <=> $bs[$i]
            : $as[$i] cmp $bs[$i]);
}

sub properName {
    my $obj = shift;
    my $n = shift;
    foreach (keys %{$obj->{baseNames}}) {
        if (lc($_) eq $n) {
            return $_;
        }
    }
    return $n;
}
