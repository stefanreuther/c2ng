#
#  Build Rules for Documentation
#
#  Parameters:
#     XSLTPROC     path to xsltproc binary (mandatory)
#     CGI          path to c2web source (optional)
#
#  You can invoke this from the main rule file using ENABLE_DOC=1,
#  but can also use it standalone to build just the docs and nothing else.
#


# Prepare directory names
my $IN = $V{IN};
my $OUT = $V{OUT};
my $TMP = $V{TMP};
my $ROOT = normalize_filename($IN, '..');
my $PERL = add_variable(PERL => $^X);
my $XSLTPROC = add_variable(XSLTPROC => 'xsltproc');
my $prefix = get_variable('prefix');

# Prepare list of source files
my $list = load_variables("$ROOT/P9/Settings");
my @sources;
foreach (qw(FILES_serverlib FILES_gamelib FILES_guilib)) {
    push @sources, split /\s+/, $list->{$_};
}

# Prepare auxiliary files
my $script = normalize_filename($ROOT, 'scripts/doc.pl');
my $style = normalize_filename($ROOT, 'scripts/pcc2help.xsl');


#
#  Interpreter documentation
#
{
    my @int_files;
    push @int_files, sort grep {/\.txt$/} get_directory_content("$IN/InterpreterReference");
    push @int_files, sort map {normalize_filename($ROOT, $_)} grep {m!^(client/si|game/interface|interpreter)/!} @sources;
    push @int_files, sort grep {m!/core.*\.q$!} get_directory_content("$ROOT/share/resource");

    # Build XML
    my $xml = generate("$TMP/pcc2interpreter.xml",
                       [@int_files, $script],
                       "$PERL $script -ns=int >\$@ ".join(' ', @int_files));
    rule_add_info($xml, "Scanning Interpreter input files");

    # Build HTML
    my $html = generate("$OUT/pcc2interpreter.html",
                        [$xml, $style],
                        "$XSLTPROC --path \"$ROOT/scripts\" --param whoami \"'int'\" $style $xml >\$@");
    rule_add_info($html, "Rendering Interpreter HTML");

    generate("all", $html);
    generate("install-doc", generate_copy("$prefix/doc/pcc2interpreter.html", $html));

    # Also install the XML for use by the help system
    # (FIXME: does this reliably work?)
    my $copy = generate_copy("$prefix/share/resource/pcc2interpreter.xml", $xml);
    generate("resources", $copy);
    generate("install-doc", $copy);
}


#
#  PlanetsCentral documentation
#
my $CGI_PATH = add_variable(CGI => '');
if ($CGI_PATH ne '') {
    # Scan API files
    my @pcc_files;
    push @pcc_files, sort grep {/\.txt$/} get_directory_content("$IN/ServerReference");
    push @pcc_files, sort grep {/\.cgi$/} get_directory_content("$CGI_PATH/api");
    push @pcc_files, sort map {normalize_filename($ROOT, $_)} grep {m|^server/|} @sources;

    # Build XML
    my $xml = generate("$TMP/pcc2tech.xml",
                       [@pcc_files, $script],
                       "$PERL $script -ns=pcc >\$@ ".join(' ', @pcc_files));
    rule_add_info($xml, "Scanning PlanetsCentral input files");

    # Build HTML
    my $html = generate("$OUT/pcc2tech.html",
                        [$xml, $style],
                        "$XSLTPROC --path \"$ROOT/scripts\" --param whoami \"'pcc'\" $style $xml >\$@");
    rule_add_info($html, "Rendering PlanetsCentral HTML");

    generate("all", $html);
    generate("install-doc", generate_copy("$prefix/doc/pcc2tech.html", $html));
}


#
#  Help file HTML
#

# ... FIXME ...

rule_set_phony("install-doc");
generate("install", "install-doc");
