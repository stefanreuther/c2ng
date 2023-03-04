#!/usr/bin/perl -w
#
#  Build zipfile for Windows
#
#  Invoke as 'build_win32_zip.pl <path-to-source> <path-to-install-directory>
#
#  Determines the file name form the configured version number;
#  builds a zip with ad-hoc filtering.
#

use strict;

my @binaries = qw(
    c2check.exe
    c2compiler.exe
    c2configtool.exe
    c2export.exe
    c2gfxgen.exe
    c2mgrep.exe
    c2mkturn.exe
    c2ng.exe
    c2plugin.exe
    c2pluginw.exe
    c2rater.exe
    c2script.exe
    c2simtool.exe
    c2sweep.exe
    c2unpack.exe
    c2untrn.exe
);

# Read version number
my $in = shift @ARGV or die;
my $version;
open FILE, '<', "$in/version.hpp" or die "$in/version.hpp: $!";
while (<FILE>) {
    if (/^\s*#\s*define\s+PCC2_VERSION\s+"([^\s\\"]+)/) {
        $version = $1;
    }
}
close FILE;
die "Version not found" if !defined $version;

# Build directory
my $dir = shift @ARGV or die;
chdir $dir or die "$dir: $!";

my $fn = "c2ng-$version-win32.zip";
unlink $fn;

exit system "zip", "-r9", $fn, sort (find_files(".", ""));

# Generate list of file names
sub find_files {
    my $dir = shift;
    my $pfx = shift;
    my @result;
    opendir my $dh, $dir or die "$dir: $!";
    while (defined(my $e = readdir($dh))) {
        my $subdir = "$dir/$e";
        my $subpfx = "$pfx$e";

        if ($e =~ /^\./) {
            # Skip file system/build system artifacts
        } elsif ($e =~ /core\.qs/) {
            # Skip personal artifacts
        } elsif ($e =~ /\.exe/) {
            # Whitelist for binaries
            push @result, $subpfx
                if grep {$_ eq $e} @binaries;
        } elsif ($subpfx =~ /^share\/server/) {
            # Skip server data
        } elsif (-d $subdir) {
            # Recurse into subdir
            push @result, find_files($subdir, "$subpfx/");
        } else {
            # Keep file
            push @result, $subpfx;
        }
    }

    @result;
}
