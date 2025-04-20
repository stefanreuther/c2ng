#
#  Output Manpages
#

# Begin a run
sub begin_run {
}

# Begin a page
sub begin_page {
    my ($name, $title, $year) = @_;
    print ".TH ", uc($name), " 6 $year \"VGA PLANETS\" \"PCC2\"\n";
    print ".SH NAME\n";
    print "$name \\- $title\n\n";
}

# End a page
sub end_page {
}

# End a run
sub end_run {
}

# section(name:text, paragraph, paragraph, paragraph, ...)
#   section, with heading and paragraphs
sub section {
    assert(@_ > 0, "section() needs at least one parameter");

    my ($title, @content) = @_;
    print ".SH \"", man_format_text(uc($title)), "\"\n";
    man_print_section(@content);
}

# subsect(name:text, paragraph, paragraph, paragraph, ...)
#   subsect, with heading and paragraphs
sub subsect {
    assert(@_ > 0, "subsect() needs at least one parameter");

    my ($title, @content) = @_;
    print ".SS \"", man_format_text($title), "\"\n";
    man_print_section(@content);
}

# paragraph_synopsis(text)
#   synopsis paragraph (monospace font)
sub paragraph_synopsis {
    paragraph_text(@_);
}

# paragraph(text...)
#   plain paragraph; text can use tags
sub paragraph_text {
    my $result = '';
    foreach (@_) {
        my $line = man_format_text($_);
        $result .= $line;
        $result .= "\n";
    }
    $result;
}

# paragraph_indent(text...)
#   indented paragraph
sub paragraph_indent {
    my $result = ".RS\n";
    foreach (@_) {
        $result .= man_format_text($_);
        $result .= "\n";
    }
    $result .= ".RE\n";
    $result;
}

# paragraph_list(text...)
#   bulleted list; one item per parameter
sub paragraph_list {
    my $result = "";
    foreach (@_) {
        $result .= ".IP \\(bu\n";
        $result .= man_format_text($_);
        $result .= "\n";
    }
    $result .= ".RE\n";
    $result;
}

# paragraph_detail(text, paragraph...)
#   detail paragraph, text can use tags
sub paragraph_detail {
    my $name = shift;
    ".IP \"" . man_format_text($name) . "\"\n" . join("\n", @_);
}

# paragraph_link_list(link:label....)
#   paragraph containing list of links
sub paragraph_link_list {
    my $result = '';
    foreach (@_) {
        $result .= ', ' if $result ne '';
        if (/^(.*)(\(.*\))$/) {
            $result .= "\\fB$1\\fR$2";
        } else {
            $result .= "\\fI$_\\fR";
        }
    }
    "$result\n";
}

# paragraph_indented_list(key => value, key => value...)
#   intended to be used inside paragraph_detail().
sub paragraph_indented_list {
    assert(@_ % 2 == 0, 'paragraph_indented_list() must have even number of parameters');

    my $result = ".RS 12\n";
    while (@_) {
        my $key = shift;
        my $value = shift;
        $result .= "\n" unless $result eq '';
        $result .= man_format_text($key).' '.man_format_text($value)."\n";
    }
    $result .= ".RE\n";
    $result;
}

# paragraph_table(colspec, [row...], [row...])
#   colspec has one letter (r/l/c) per column.
#   a row can be 0 to insert a spacer.
sub paragraph_table {
    my $colspec = shift;
    my $result = ".TS\n$colspec.\n";
    foreach my $pRow (@_) {
        if ($pRow) {
            for (my $i = 0; $i < length($colspec); ++$i) {
                $result .= "\t" if $i;
                $result .= man_format_text($pRow->[$i]) if defined $pRow->[$i];
            }
        }
        $result .= "\n";
    }
    $result .= ".TE\n";
    $result;
}

# internal
sub man_format_text {
    my $line = shift;

    # Quote backslashes
    $line =~ s|\\|\\\\|g;

    # Process tags
    $line =~ s|<i>|\\fI|g;
    $line =~ s|</i>|\\fR|g;
    $line =~ s|<b>|\\fB|g;
    $line =~ s|</b>|\\fR|g;
    $line =~ s|<cmd>|\\fB|g;
    $line =~ s|</cmd>|\\fR|g;
    $line =~ s|<fn>|\\fI|g;
    $line =~ s|</fn>|\\fR|g;

    # Links
    # TODO: internal links have form "pcc2:" (PCC2 user help), "int:" (PCC2 interpreter help), and "pcc:" (PlanetsCentral tech manual).
    $line =~ s|<link>(.*?)(\(\d*?\))</link>|\\fB$1\\fR$2|g;
    $line =~ s|<link>(.*?)</link>|<\\fI$1\\fR>|g;

    # Process specials
    $line =~ s|&lt;|<|g;
    $line =~ s|&gt;|>|g;
    $line =~ s|&quot;|"|g;
    $line =~ s|&trade;|\\*(Tm|g;
    $line =~ s|&reg;|\\*R|g;
    $line =~ s|&amp;|&|g;

    # Escape things for groff
    $line =~ s|"|\\(dq|g;             # quotes
    $line =~ s|^\.|\\&.|g;            # paragraph starting with dot

    # Remove duplicate font selection
    $line =~ s|\\f.(\\f.)|$1|g;
    $line;
}

sub man_print_section {
    my $special = 0;
    foreach (@_) {
        my $new_special = /^\./;
        if ($special && !$new_special) {
            print ".PP\n";
        }
        print $_, "\n";
        $special = $new_special;
    }
    1;
}

1;
