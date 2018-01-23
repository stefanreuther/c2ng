#!/usr/bin/perl -w
#
#  Update configuration file
#
#  Usage:
#    updateconfig CONFIGFILE SECTION.PARAM=VALUE --file=CONFIG.FRAG
#
#  where
#    CONFIGFILE            The file to update
#    SECTION.PARAM=VALUE   A new value. VALUE can be empty to delete the value.
#                          This parameter can appear as often as needed.
#    CONFIG.FRAG           A config file fragment containing new values.
#                          Likewise, --file can appear as often as needed.
#                          Syntax can either be
#                              Section.Param = Value
#                          or regular
#                              % Section
#                              Param = Value
#
use strict;

if (@ARGV == 0) {
    die "Usage: $0 configfile section.param=value [section.param=value...] [--file=config.frag]\n";
}

# Parse command line
my $file = shift @ARGV;
my %values;                     # Indexed by section.param; new values
my %keys;                       # Indexed by section.param; original spelling of Section and Param
foreach (@ARGV) {
    if (/^--file=(.*)/) {
        readFragment($1);
    } elsif (/^(.*?)=(.*)/) {
        my $key = $1;
        my $value = $2;
        my $k = lc($key);
        $keys{$k} = $key;
        $values{$k} = $value;
    } else {
        die "Invalid assignment '$_'.\n";
    }
}

my %did;                        # Mark keys we have already done
foreach (keys %values) {
    $did{$_} = ($values{$_} eq '');
}


# Read and parse file
my @result;
my $hadInputFile = 0;
if (open (FILE, "< $file")) {
    my $section = '';
    $hadInputFile = 1;
    while (<FILE>) {
        s/[\r\n]+//g;
        if (m/^\s*\#/) {
            # Comment
            push @result, $_;
        } elsif (m/^\s*%\s*(\S+)/) {
            # Section
            my $newSection = lc($1);
            flushSection($section);
            $section = $newSection . ".";
            push @result, $_;
        } elsif (m/^(\s*)(\S+)(\s*=\s*)(.*)/) {
            # Assignment
            my $pre = $1;
            my $key = $2;
            my $post = $3;
            my $k = $section.lc($key);
            if (exists $keys{$k}) {
                # We want to change this
                if (!$did{$k}) {
                    push @result, "$pre$key$post$values{$k}";
                    $did{$k} = 1;
                }
            } else {
                # No change
                push @result, $_;
            }
        } else {
            # Unparsed
            push @result, $_;
        }
    }
    flushSection($section);
    close FILE;
}

# If there are still undone items, process them
flushSection("");
foreach (sort keys %keys) {
    if (!$did{$_}) {
        if (m/^(.*?)\./) {
            my $sec = $1;
            push @result, '', '% '.$sec, '';
            flushSection(lc($sec));
        }
    }
}


# Generate output. Do not generate output if we had no input file and result would be empty.
if ($hadInputFile || @result) {
    open FILE, "> $file" or die "$file: $!\n";
    foreach (@result) {
        print FILE "$_\n";
    }
    close FILE;
}
exit 0;


# flushSection($section):
#   Output all outstanding assignments for a given section
sub flushSection {
    my $section = shift;
    my $added = 0;
    if ($section ne '') {
        # We're leaving a section. Did we have all fields?
        foreach (sort keys %keys) {
            if (!$did{$_} && substr($_, 0, length($section)+1) eq $section.".") {
                my $k = $keys{$_};
                $k =~ s/^(.*?)\.//;
                push @result, "$k = $values{$_}";
                $did{$_} = 1;
                $added = 1;
            }
        }
    } else {
        # We're entering a section. Write everything that has no section.
        foreach (sort keys %keys) {
            if (!$did{$_} && !/\./) {
                my $k = $keys{$_};
                $k =~ s/^(.*?)\.//;
                push @result, "$k = $values{$_}";
                $did{$_} = 1;
                $added = 1;
            }
        }
    }
    if ($added) { push @result, '' }
}

# readFragment($fileName):
#   Read a file containing assignments
sub readFragment {
    my $file = shift;
    my $prefix = "";
    open FILE, "< $file" or die "$file: $!\n";
    while (<FILE>) {
        s/[\r\n]//g;
        if (m/^\s*\#/) {
            # Comment
        } elsif (m/^\s*%\s*(\S+)/) {
            # Section
            $prefix = $1 . ".";
        } elsif (m/^(\s*)(\S+)(\s*=\s*)(.*)/) {
            # Assignment
            my $pre = $1;
            my $key = $2;
            my $post = $3;
            my $value = $4;
            my $k = lc($prefix . $key);
            $keys{$k} = $key;
            $values{$k} = $value;
        } else {
            # Unparsed
        }
    }
}
