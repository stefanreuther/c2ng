#!/usr/bin/perl -w
#
#  Documentation / Manual Pages for PCC2
#

use strict;
use lib '.';

my $YEAR = 2025;
my $page_started = 0;
my $run_started = 0;

foreach (@ARGV) {
    $page_started = 0;
    do $_ or die "$_: failed: $@";
    end_page() if $page_started;
}
end_run() if $run_started;

#
#  Common
#

sub assert {
    my ($cond, $msg) = @_;
    if (!$cond) {
        my $text = "error: $msg";
        my $i = 1;
        my $pos = "<unknown>";
        while (my ($pkg, $file, $line, $fn) = caller($i)) {
            $pos = "$file:$line";
            print STDERR "$pos: $text\n";
            $text = "  called from here";
            last if ++$i >= 10;
        }
        die "Aborted\n";
    }
}

sub start {
    assert(@_ == 2, "start() requires two parameter");
    assert(!$page_started, "start() called twice");

    begin_run() if !$run_started;
    begin_page(@_, $YEAR);

    $page_started = 1;
    $run_started = 1;
}

#
#  Macros
#

sub paragraph_default_author {
    return (paragraph_text('Stefan Reuther &lt;Streu@gmx.de&gt;'),
            paragraph_text('This program is part of the PCC2 package (c2ng).'));
}

sub paragraph_default_environment {
    my $sep = shift || ".";
    return paragraph_detail('<b>LANG</b>, <b>LC_MESSAGES</b>, <b>LC_ALL</b>', paragraph_text('Language to use'.$sep));
}

sub paragraph_default_server_environment {
    my $sep = shift || ".";
    return paragraph_detail('<b>C2CONFIG</b>', paragraph_text('Name of configuration file.',
                                                              'Defaults to <fn>c2config.txt</fn>'.$sep));
}

sub paragraph_default_server_config {
    my $sep = shift || ".";
    return (paragraph_detail('<b>-D</b><i>KEY</i><b>=</b><i>VALUE</i>',
                             paragraph_text('Override a configuration file entry;')),
            paragraph_detail(text_option('--config=', 'FILE'),
                             paragraph_text('Set name of configuration file.',
                                            'Default is <b>C2CONFIG</b> environment variable, or <fn>c2config.txt</fn> if not set'.$sep)));
}

sub paragraph_default_server {
    my $self = shift || "?";
    my $sep = shift || ".";
    return (paragraph_default_help(';'),
            paragraph_default_server_config(';'),
            paragraph_detail(text_option('--instance', 'INSTANCE'),
                             paragraph_text('Set name of the server instance (default: "'.$self.'").',
                                            'The instance name selects a set of configuration parameters from the configuration file;')),
            paragraph_default_log(';'),
            paragraph_default_proxy($sep));
}

sub paragraph_default_server_intro {
    my $name = shift;
    return paragraph_text("<i>$name</i> takes its configuration from a file",
                          'specified using the <b>--config</b> option or the <b>C2CONFIG</b> environment variable.',
                          'This configuration file specifies the port/interface to listen on,',
                          'ports/addresses of other services, and possibly more parameters.');
}

sub paragraph_default_help {
    my $sep = shift || ".";
    return paragraph_detail('<b>--help</b>, <b>-h</b>',
                            paragraph_text("Show <i>help</i> summary$sep"));
}

sub paragraph_default_optimizer {
    my $sep = shift || ".";
    return paragraph_detail(text_option('-O', 'LEVEL'),
                            paragraph_text('Set optimisation level.'),
                            paragraph_indented_list('-O0', '(no optimisation, but some standard code selection intelligence is still used);',
                                                    '-O1', '(normal optimisation, default);',
                                                    '-O2', '(enable more expensive optimisations);',
                                                    '-O3', '(enable optimisations that may change behaviour in boundary case, e.g. generate different error messages than normal);',
                                                    '-O-1', '(generate most naive code possible. This setting is not intended for normal use, but as a way out if I broke something and optimisation breaks your script.)'.$sep));
}

sub paragraph_default_command {
    my $sep = shift || ".";
    return paragraph_detail(text_option('-k'),
                            paragraph_text('If specified, the parameters on the command line are commands, not files.',
                                           'It is not possible to mix commands and file names'.$sep));
}

sub paragraph_default_log {
    my $sep = shift || ".";
    return paragraph_detail(text_option('--log=', 'CONFIG'),
                            paragraph_text('Configure log output.',
                                           'The <i>CONFIG</i> parameter has the form "LOG@LEVEL=MODE[:LOG@LEVEL=MODE...]"'.$sep));
}

sub paragraph_default_proxy {
    my $sep = shift || ".";
    return paragraph_detail(text_option('--proxy=', 'URL'),
                            paragraph_text('Set proxy for outgoing connections.',
                                           'The <i>URL</i> can point to a SOCKS4 (<b>socks4://</b><i>HOST</i>[<b>:</b><i>PORT</i>])',
                                           'or SOCKS5 (<b>socks5://</b><i>HOST</i>[<b>:</b><i>PORT</i>]) proxy'.$sep));
}

sub text_option {
    my $result = '';
    while (@_) {
        my $opt = shift;
        my $val = shift;
        $result .= ', ' if $result ne '';
        $result .= "<b>$opt</b>";
        if (defined($val) && $val ne '') {
            $result .= ' ' unless $opt =~ /=$/;
            $result .= "<i>$val</i>";
        }
    }
    return $result;
}
