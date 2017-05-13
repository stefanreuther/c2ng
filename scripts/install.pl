#!/usr/bin/perl -w
#
#  Install script. Why not use standard `install' resp. `install-sh'?
#  This one has a bit of special treatment for vgalib (programs must
#  be setuid root, but our Makefile can't tell whether the user has a
#  SDL with VGA support). And while we are at it, add some more useful
#  features.
#
#  Though inspired by GNU/BSD install, this one's probably not
#  compatible.
#
#  February 2002 by Stefan Reuther <Streu@gmx.de>
#

use strict;
use vars qw{$opt_makedirs $opt_strip $opt_transform $opt_mode $opt_owner
            $opt_vgalib $opt_dry $opt_log $opt_ignore};
use vars qw{@arg_files @arg_targets $arg_target %made_dirs $retcode};

sub sh;

$opt_makedirs = 0;
$opt_strip = 0;
$opt_transform = "s,x,x,";
$opt_mode = "0755";
$opt_owner = "root";
$opt_vgalib = 0;
$opt_dry = 0;
$opt_log = undef;
$opt_ignore = 0;
%made_dirs = ();
$retcode = 0;

&parse_args();
&find_targets();
if (defined $opt_log) {
    if ($opt_log eq 'NONE') {
        $opt_log = undef;
    } else {
        open LOGFILE, ">> $opt_log" or do {
            print STDERR "$opt_log: $!\n";
            exit 1;
        };
    }
}
&do_install();
close LOGFILE if defined $opt_log;

exit $retcode;

########################### Actual Installation ###########################

sub do_install {
    foreach my $i (0..$#arg_files) {
        my $src = $arg_files[$i];
        my $dest = $arg_targets[$i];
        if (! stat $src) {
            print STDERR "$src: $!\n";
            $retcode = 1;
            next;
        }
        &do_makedirs ($dest) if $opt_makedirs;
        sh "rm", "-f", $dest;
        sh "cp", $src, $dest;
        sh "strip", $dest if $opt_strip;
        if ($opt_vgalib && grep /\blibvga\.so/, `ldd $src`) {
            sh "chown", "root", $dest;
            sh "chmod", "4711", $dest;
        } else {
            if (defined($opt_owner)) {
                sh "chown", $opt_owner, $dest;
            }
            sh "chmod", $opt_mode, $dest;
        }
        print LOGFILE "$dest\n" if defined $opt_log;
    }
}

################################ Utilities ################################

# Parse parameters.
sub parse_args {
    my $arg_pending = undef;
    foreach (@ARGV) {
        if (defined $arg_pending) {
            eval $arg_pending;
            $arg_pending = undef;
        } elsif (/^(-d|--directory)$/) {
            $opt_makedirs = 1;
        } elsif (/^(-m|--mode)$/) {
            $arg_pending = '$opt_mode = $_';
        } elsif (/^(-m|--mode=)(.*)/) {
            $opt_mode = $2;
        } elsif (/^(-o|--owner)$/) {
            $arg_pending = '$opt_owner = $_';
        } elsif (/^(-o|--owner=)(.*)/) {
            $opt_owner = $2;
        } elsif (/^(-O|--no-chown)$/) {
            $opt_owner = undef;
        } elsif (/^(-s|--strip)$/) {
            $opt_strip = 1;
        } elsif (/^(-v|--s?vgalib)$/) {
            $opt_vgalib = 1;
        } elsif (/^(-t|--transform)$/) {
            $arg_pending = '$opt_transform = $_';
        } elsif (/^(-t|--transform=)(.*)/) {
            $opt_transform = $2;
        } elsif (/^(-n|--just-print|--dry-run|--recon)$/) {
            $opt_dry = 1;
        } elsif (/^(-R|--recurs(iv)e(-scan)?)$/) {
            $arg_pending = '&scan_recursive ($_)';
        } elsif (/^(-R|--recurs(iv)e(-scan)?=)(.*)/) {
            &scan_recursive ($4);
        } elsif (/^(-L|--log)$/) {
            $arg_pending = '$opt_log = $_';
        } elsif (/^(-L|--log=)(.*)$/) {
            $opt_log = $2;
        } elsif (/^(-i|--ignore(-errors))$/) {
            $opt_ignore = 1;
        } elsif (/^--help$/) {
            print "$0 -- install program

usage: install [-options] FILES... DIRECTORY

  -d  --directory         Make missing directories
  -m  --mode=MODE         Set file modes [0755]
  -o  --owner=UID         Set file owner [root]
  -s  --strip             Strip installed files
  -v  --vgalib            Special treatment for programs using VGALIB
  -t  --transform=SEDCMD  Transform program names with SEDCMD
                          (Perl regexp command, actually)
  -L  --log=NAME          Append file list to log file
  -R  --recurse=NAME      Gather list of files by scanning specified directory
  -i  --ignore            Ignore errors of subprograms
  -n  --dry-run           Don't execute commands, just print them\n";
            exit 0;
        } elsif (/^-/) {
            print STDERR "$0: unknown option `$_'\n";
            exit 1;
        } else {
            push @arg_files, $_;
            s{^.*/}{};
            push @arg_targets, $_;
        }
    }
    if (defined $arg_pending) {
        print STDERR "$0: missing parameter to `$ARGV[-1]'\n";
        exit 1;
    }
    if (@arg_files < 2) {
        print STDERR "$0: need at least two file names\n";
        exit 1;
    }
    $arg_target = pop @arg_files;
    pop @arg_targets;
}

# Complete the @arg_targets array. For each N, @arg_targets[N] is the
# target name under which @arg_files[N] will be installed. The parameter
# parser already put the "basename" in @arg_targets[N] (for files on
# the command line, this is the actual basename, but not for files
# gathered with -R).
sub find_targets {
    $arg_target .= "/" unless $arg_target =~ m,/$,;
    foreach (@arg_targets) {
        eval $opt_transform;
        if ($@ ne "") {
            print STDERR "$0: transformation command `$opt_transform' is invalid: $@\n";
            exit 1;
        }
        $_ = $arg_target . $_;
    }
}

# do_makedirs ('name'). Make all directories so that 'name' can be
# created. We remember which ones we made to not clutter up -n scripts.
sub do_makedirs {
    my $name = shift;
    $name =~ s,/[^/]+$,, or return;
    return if $name eq "" || -d $name || exists $made_dirs{$name};

    &do_makedirs ($name);

    my $dirmode = sprintf "%o", oct($opt_mode) | ((oct($opt_mode) & 0444) >> 2);
    sh "mkdir", $name;
    sh "chmod", $dirmode, $name;
    $made_dirs{$name} = 1;
}

# scan_recursive(NAME): gather files recursively, like ls -R. Ignores
# hidden files, backup files and CVS directories.
sub scan_recursive {
    # breadth-first search.
    my $name = shift;
    my $strip = length $name;
    ++$strip unless $name =~ m{/$};
    my @todo = ($name);
    while (@todo) {
        my $now = shift @todo;
        opendir DIR, $now or do {
            print STDERR "$now: $!\n";
            next;
        };
        while (defined (my $f = readdir DIR)) {
            my $name = "$now/$f";
            next if -l $name || $f =~ /^\./ || $f eq 'CVS' || $f =~ /~$/;
            if (-d $name) {
                push @todo, $name;
            } else {
                push @arg_files, $name;
                push @arg_targets, substr $name, $strip;
            }
        }
        closedir DIR;
    }
}

# "wrapper" for `system'. Execute commands, or print them if dry-run
# requested. Exit when one fails.
sub sh {
    if ($opt_dry) {
        print "\t", join (' ', @_), "\n";
    } else {
        my $rv = system @_;
        if ($rv != 0) {
            print STDERR "*** error: the above command exited with status ", $rv >> 8, ".\n";
            exit 1 unless $opt_ignore;
        }
    }
}
