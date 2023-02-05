#!/usr/bin/perl -w
#
#  Usage:
#     copy_dlls.pl bindir [dlldirs...]
#
#  Scan all the binaries in bindir for DLL references.
#  If a reference can be resolved by using a DLL from dlldirs, copies that into bindir.
#
use strict;

my $bindir = shift @ARGV;
if (!defined($bindir) || $bindir =~ /^-/) {
    die "Usage: $0 bindir [dlldirs...]";
}

# Scan for DLLs
my %dlls;
foreach my $dir (@ARGV) {
    foreach (split /[ :;]/, $dir) {
        load_dlls($_);
    }
}

# Scan for executables
my @todo;
opendir BINDIR, $bindir or die "$bindir: $!";
while (defined(my $de = readdir(BINDIR))) {
    if ($de =~ /^\./) {
        # Skip
    } elsif ($de =~ /\.exe$/i) {
        push @todo, "$bindir/$de";
    } elsif ($de =~ /\.dll$/i) {
        # Local DLL
        $dlls{lc($de)} = { name => "$bindir/$de", done => 1, copy => 0 }
    } else {
        # Other
    }
}
closedir BINDIR;

# Scan all files
while (@todo) {
    process_file(shift(@todo));
}

# Copy all files
my $ex = 0;
foreach (sort keys %dlls) {
    if ($dlls{$_}{copy}) {
        my $in = $dlls{$_}{name};
        my $out = "$bindir/$_";
        print "\tCopying $in...\n";
        if (system('cp', $in, $out) != 0) {
            $ex = 1;
        }
    }
}
exit $ex;



sub load_dlls {
    my $name = shift;
    if (opendir(my $dh, $name)) {
        while (defined(my $de = readdir($dh))) {
            if ($de =~ /^\./) {
                # Skip
            } elsif ($de =~ /\.dll$/i && -f "$name/$de") {
                # Found a DLL
                my $n = lc($de);
                if ($dlls{$n}) {
                    print STDERR "Warning: '$n' exists as '$name/$de' and as '$dlls{$n}{name}'\n";
                } else {
                    $dlls{$n} = { name => "$name/$de", done => 0, copy => 0 };
                }
            } elsif (-d "$name/$de") {
                # Directory; recurse
                load_dlls("$name/$de");
            } else {
                # Skip
            }
        }
        closedir $dh;
    }
}

sub process_file {
    my $file = shift;
    print "\tChecking $file...\n";
    open my $fh, '-|', 'objdump', '-p', $file
        or die "objdump $file: $!";
    while (<$fh>) {
        if (/DLL Name:\s+(.*\.dll)/i) {
            my $name = lc($1);
            if ($dlls{$1} && !$dlls{$1}{done}) {
                $dlls{$1}{done} = 1;
                $dlls{$1}{copy} = 1;
                push @todo, $dlls{$1}{name};
            }
        }
    }
    close $fh;
}
