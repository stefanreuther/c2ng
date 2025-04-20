#
#  Output XML
#

# Begin a run
sub begin_run {
    print "<?xml version=\"1.0\"?>\n";
    print "<!DOCTYPE help SYSTEM \"pcc2help.dtd\">\n";
    print "<help priority=\"99\">\n";
}

# Begin a page
sub begin_page {
    my ($name, $title, $year) = @_;
    print "<page id=\"man:$name\">\n";
    print "<h1>", xml_quote($name), " - ", xml_quote($title), "</h1>\n";
}

# End a page
sub end_page {
    print "</page>\n";
}

# End a run
sub end_run {
    print "</help>\n";
}

# section(name:text, paragraph, paragraph, paragraph, ...)
#   section, with heading and paragraphs
sub section {
    assert(@_ > 0, "section() needs at least one parameter");

    my ($title, @content) = @_;
    print "<h2>", xml_format_text($title), "</h2>\n";
    xml_print_section(@content);
}

# subsect(name:text, paragraph, paragraph, paragraph, ...)
#   subsect, with heading and paragraphs
sub subsect {
    assert(@_ > 0, "subsect() needs at least one parameter");

    my ($title, @content) = @_;
    print "<h3>", xml_format_text($title), "</h3>\n";
    xml_print_section(@content);
}

# paragraph_synopsis(text)
#   synopsis paragraph (monospace font)
sub paragraph_synopsis {
    my $result = '<pre>';
    my $first = 1;
    foreach (@_) {
        $result .= "\n" unless $first;
        $result .= xml_format_text($_);
        $first = 0;
    }
    $result .= '</pre>';
    $result;
}

# paragraph(text...)
#   plain paragraph; text can use tags
sub paragraph_text {
    my $result = '<p>';
    my $first = 1;
    foreach (@_) {
        $result .= "\n" unless $first;
        $result .= xml_format_text($_);
        $first = 0;
    }
    $result .= '</p>';
    $result;
}

# paragraph_indent(text...)
#   indented paragraph
sub paragraph_indent {
    paragraph_synopsis(map{"    $_"} @_);
}

# paragraph_list(text...)
#   bulleted list; one item per parameter
sub paragraph_list {
    my $result = "";
    foreach (@_) {
        $result .= "<li>";
        $result .= xml_format_text($_);
        $result .= "</li>\n";
    }
    $result;
}

# paragraph_detail(text, paragraph...)
#   detail paragraph, text can use tags
sub paragraph_detail {
    my $name = shift;
    my $result = '<li><b>'.xml_format_text($name).'</b><br />';
    if (@_ == 1 && $_[0] =~ m|^<p>(.*)</p>$|s) {
        $result .= $1;
    } else {
        $result .= join("\n", @_);
    }
    $result .= "</li>";
    $result;
}

# paragraph_link_list(link:label....)
#   paragraph containing list of links
sub paragraph_link_list {
    my $result = '';
    foreach (@_) {
        $result .= '<li>';
        $result .= xml_format_text('<link>'.$_.'</link>');
        $result .= '</li>';
    }
    "<ul>$result</ul>\n";
}

# paragraph_indented_list(key => value, key => value...)
#   intended to be used inside paragraph_detail().
sub paragraph_indented_list {
    assert(@_ % 2 == 0, 'paragraph_indented_list() must have even number of parameters');

    my $result = "<dl>\n";
    while (@_) {
        my $key = shift;
        my $value = shift;
        $result .= '<di term="'.xml_quote($key).'">'.xml_format_text($value)."</di>\n";
    }
    $result .= "</dl>\n";
    $result;
}

# paragraph_table(colspec, [row...], [row...])
#   colspec has one letter (r/l/c) per column.
#   a row can be 0 to insert a spacer.
sub paragraph_table {
    my $colspec = shift;
    my $result = "<table>\n";
    foreach my $pRow (@_) {
        if ($pRow) {
            $result .= "<tr>";
            for (my $i = 0; $i < length($colspec); ++$i) {
                $result .= "<td>";
                $result .= xml_format_text($pRow->[$i]) if defined $pRow->[$i];
                $result .= "</td>\n";
            }
            $result .= "</tr>\n";
        }
    }
    $result .= "</table>\n";
    $result;
}

# internal
sub xml_format_text {
    my $line = shift;

#    # Quote backslashes
#    $line =~ s|\\|\\\\|g;
#
    # Process tags
    $line =~ s|<i>|<em>|g;
    $line =~ s|</i>|</em>|g;
    # No need to process <b>
    $line =~ s|<cmd>|<tt>|g;
    $line =~ s|</cmd>|</tt>|g;
    $line =~ s|<fn>|<tt>|g;
    $line =~ s|</fn>|</tt>|g;

    # Links
    # TODO: internal links have form "pcc2:" (PCC2 user help), "int:" (PCC2 interpreter help), and "pcc:" (PlanetsCentral tech manual).
    $line =~ s|<link>(.*?)(\(6\))</link>|<a href="man:$1">$1</a>|g;
    $line =~ s|<link>(.*?)(\(\d*?\))</link>|<b>$1</b>$2|g;
    $line =~ s|<link>(.*?)</link>|&lt;<em>$1</em>&gt;|g;

    # Process specials
    $line =~ s|&trade;|&#x2122;|g;
    $line =~ s|&reg;|&#xae;|g;

    # PCC cannot reliably handle only-space-between-tags.
    # Replace by non-breaking space (probably makes more sense anyway).
    $line =~ s|> <|>&#160;<|g;

    $line;
}

sub xml_print_section {
    my $tag = '';
    foreach (@_) {
        my $expect = '';
        if (/^<li>/) {
            $expect = 'ul';
        } elsif (/^<dt>/) {
            $expect = 'dl';
        } else {
            $expect = '';
        }
        if ($tag ne $expect) {
            print "</$tag>\n" if $tag ne '';
            $tag = $expect;
            print "<$tag>\n" if $tag ne '';
        }
        print $_;
    }
    print "</$tag>\n" if $tag ne '';
    1;
}

sub xml_quote {
    my $line = shift;
    $line =~ s/&/&amp;/g;
    $line =~ s/"/&quot;/g;
    $line =~ s/</&lt;/g;
    $line =~ s/>/&gt;/g;
    $line;
}

1;
