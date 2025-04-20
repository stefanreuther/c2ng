#
#  Build script for manual pages
#

my @manpages = qw(
    pcc2
    c2check
    c2compiler
    c2configtool
    c2console
    c2dbexport
    c2docmanager
    c2doc-server
    c2export
    c2fileclient
    c2file-server
    c2format-server
    c2gfxcodec
    c2gfxgen
    c2host-server
    c2logger
    c2mailin
    c2mailout-server
    c2mgrep
    c2mkturn
    c2monitor-server
    c2ng
    c2nntp-server
    c2play-server
    c2plugin
    c2pluginw
    c2rater
    c2restool
    c2router-server
    c2script
    c2simtool
    c2sweep
    c2talk-server
    c2unpack
    c2untrn
    c2user-server
    );

my $prefix = get_variable('prefix');
my $generate = "$V{IN}/Generate.pl";
my $outman = "$V{IN}/OutputMan.pl";
my $outxml = "$V{IN}/OutputXML.pl";
my $perl = get_variable('PERL');
my $xsltproc = add_variable(XSLTPROC => 'xsltproc');
my $root = normalize_filename($V{IN}, '../..');

# Manual pages (*.6)
foreach (@manpages) {
    my $in = "$V{IN}/$_.6.pl";
    my $out = "$V{OUT}/$_.6";
    my $dist = "$prefix/share/man/6/$_.6";

    generate([$out], [$in, $generate, $outman], "$perl $generate $outman $in > $out");
    rule_add_info($out, "Generating manual page $_");
    generate_copy($dist, $out);
    generate('install', $dist);
}

# XML help
{
    my @in = map {normalize_filename("$V{IN}/$_.6.pl")} @manpages;
    my $out = "$V{OUT}/pcc2man.xml";
    my $dist = "$prefix/share/resource/pcc2man.xml";
    generate([$out], [@in, $generate, $outxml], "$perl $generate $outxml ".join(' ',@in)." > $out");
    rule_add_info($out, "Generating manual page help");
    generate_copy($dist, $out);
    generate('install', $dist);

    # Convert to HTML
    my $style = normalize_filename($root, 'scripts/pcc2help.xsl');
    my $html = "$V{OUT}/pcc2man.html";
    generate([$html], [$out, $style],
             "$xsltproc --path \"$root/scripts\" --param whoami \"'man'\" $style $out >\$@");
    rule_add_info($html, "Rendering manual page HTML");

    generate("all", $html);
    generate("install-doc", generate_copy("$prefix/doc/pcc2man.html", $html));
}
