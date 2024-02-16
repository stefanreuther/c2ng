#!/usr/bin/perl -w
#
#  Generate Unit Test Boilerplate
#
#  This generates the annoying unit-test boilerplate.
#  Invoke as
#    perl addtest.pl ns1::ns2::Class nameOne nameTwo
#
#  This generates or updates the implementation file test/ns1/ns2/classtest with empty tests.
#
use strict;

if (@ARGV == 0) {
    print STDERR "usage: $0 ClassName [TestNames...]\n";
    exit 1;
}

my @namespace = split /::/, shift @ARGV;
my $className = pop @namespace;
my @testNames = @ARGV;

my $testFileName = lc(join('/', 'test', @namespace, $className)).'test.cpp';
my $classHeaderName = lc(join('/', @namespace, $className)).'.hpp';

#print "class: $testClassName\n";
#print "file: $testFileName\n";
#print "header: $testHeaderName\n";
#print "guard: $testGuardName\n";
#print "header: $classHeaderName\n";

my @newFiles;

# Read testee class
open CLASS, "< $classHeaderName" or die "$classHeaderName: $!\n";
my @classFile = parseFile(join "", <CLASS>);
close CLASS;
my $state = 0;
my $level = 0;
my @pureVirtuals;
foreach (@classFile) {
    if ($state == 0 && /class \Q$className\E\b.*\{/) {
        $state = 1;
    } elsif ($state == 1 && /(virtual.*\S)\s*=\s*0\s*;$/) {
        push @pureVirtuals, $1;
    } elsif ($state == 1 && /\{/) {
        ++$level;
    } elsif ($state == 1 && /\}/) {
        if ($level == 0) {
            $state = 0;
        } else {
            --$level;
        }
    }
}

# Generate test code
my $testCode;
my $testComment;
if (@pureVirtuals) {
    $testComment .= "/** Interface test. */\n";
    $testCode  = "    class Tester : public ".join('::', @namespace, $className)." {\n";
    $testCode .= "     public:\n";
    foreach (@pureVirtuals) {
        my $x = $_;
        $x =~ s!(\w+)([,\)])!/*$1*/$2!g;
        $testCode .= "        $x\n";
        if ($x =~ /^virtual (\w+\*+|u?int(\d+\w*_t)?|size_t)/) {
            $testCode .= "            { return 0; }\n";
        } elsif ($x =~ /^virtual void /) {
            $testCode .= "            { }\n";
        } elsif ($x =~ /^virtual bool /) {
            $testCode .= "            { return false; }\n";
        } elsif ($x =~ /^virtual (afl::base::)?Ptr</) {
            $testCode .= "            { return 0; }\n";
        } elsif ($x =~ /^virtual (afl::base::)?Ref</) {
            $testCode .= "            { throw std::runtime_error(\"no ref\"); }\n";
        } elsif ($x =~ /^virtual [\w\s:<>]+\*/) {
            $testCode .= "            { return 0; }\n";
        } elsif ($x =~ /^virtual ([\w:<>]+)/) {
            $testCode .= "            { return $1(); }\n";
        } else {
            $testCode .= "            { /* FIXME? */ }\n";
        }
    }
    $testCode .= "    };\n";
    $testCode .= "    Tester t;\n";
} else {
    $testComment = "";
    $testCode = "    ".join('::', @namespace, $className)." testee;\n",
}

generateTestFile();

if (@newFiles) {
    print "New files: ", join(' ', @newFiles), "\n";
    system "proj9", "add-file", @newFiles;
}

sub generateTestFile {
    # Load existing file
    my @test;
    if (open TEST, "< $testFileName") {
        @test = <TEST>;
        close TEST;
    } else {
        push @newFiles, "$testFileName";
        @test = ("/**\n",
                 "  *  \\file $testFileName\n",
                 "  *  \\brief Test for ".join('::', @namespace, $className)."\n",
                 "  */\n",
                 "\n",
                 "#include \"$classHeaderName\"\n",
                 "\n",
                 "#include \"afl/test/testrunner.hpp\"\n",
                 "\n");
    }

    # Generate tests
    foreach (@testNames) {
        my $name = join('.', @namespace, $className);
        $name .= ":$_" unless $_ eq '';

        push @test, ($testComment,
                     "AFL_TEST(\"$name\", a)\n",
                     "{\n",
                     $testCode,
                     "}\n",
                     "\n");
    }

    # Write it out
    system "mkdir", "-p", lc(join('/', 'test', @namespace));
    open TEST, "> $testFileName" or die "$testFileName: $!";
    print TEST @test;
    close TEST;
}

sub parseFile {
    my $class = shift;

    # Normalize: replace all strings, comments, preprocessor by space; replace multiple spaces by single space.
    $class =~ s!"([^\\"]|\\.)*"|'([^\\']|\\.)*'|/\*.*?\*/|//[^\n]*|#[^\n]*! !gs;
    $class =~ s|\s+| |gm;
    $class =~ s|^\s+||;
    $class =~ s|\s+$||;

    # Split at {};, but keep these delimiters
    my @result;
    foreach (split /(?<=[{};])/, $class) {
        s|^\s+||;
        s|\s+$||;
        push @result, $_;
    }
    @result;
}
