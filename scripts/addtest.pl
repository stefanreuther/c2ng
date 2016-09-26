#!/usr/bin/perl -w
#
#  Generate Unit Test Boilerplate
#
#  This generates the annoying unit-test boilerplate.
#  Invoke as
#    perl addtest.pl ns1::ns2::Class testOne testTwo testThree
#
#  This generates or updates
#  - the header file u/t_ns1_ns2.hpp to include an appropriately-named class, TestNs1Ns2Class,
#    with new members testOne(), testTwo(), testThree()
#  - the implementation file u/t_ns1_ns2_class.cpp with empty bodies for those members.
#
use strict;

if (@ARGV == 0) {
    print STDERR "usage: $0 ClassName [TestNames...]\n";
    exit 1;
}

my @namespace = split /::/, shift @ARGV;
my $className = pop @namespace;
my @testNames = @ARGV;
foreach (@testNames) {
    print STDERR "test '$_' does not start with 'test'\n"
        unless /^test/;
}

my $testClassName = 'Test'.join('', map {ucfirst} @namespace).$className;
my $testFileName = lc(join('_', 't', @namespace, $className)).'.cpp';
my $testHeaderName = lc(join('_', 't', @namespace)).'.hpp';
my $testGuardName = uc(join('_', 'U', 'T', @namespace, 'HPP'));
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


generateHeaderFile();
generateTestFile();

if (@newFiles) {
    print "New files: ", join(' ', @newFiles), "\n";
    system "proj9", "add-file", @newFiles;
}


sub generateHeaderFile {
    # Load existing file
    my @header;
    if (open HEADER, "< u/$testHeaderName") {
        @header = <HEADER>;
        close HEADER;
    } else {
        push @newFiles, "u/$testHeaderName";
        @header = ("/**\n",
                   "  *  \\file u/$testHeaderName\n",
                   "  *  \\brief Tests for ".join("::", @namespace)."\n",
                   "  */\n",
                   "#ifndef C2NG_$testGuardName\n",
                   "#define C2NG_$testGuardName\n",
                   "\n",
                   "#include <cxxtest/TestSuite.h>\n",
                   "\n",
                   "#endif\n");
    }

    # Build declarations
    my @testDeclarations = map {"    void $_();\n"} @testNames;
 
    # Insert new class declaration
    my $i = 0;
    while ($i < @header) {
        if ($header[$i] =~ /^class\s+\Q$testClassName\E\b/) {
            ++$i;
            while ($i < @header && $header[$i] =~ /^\s*(void\s+test|public:)/) {
                ++$i;
            }
            splice @header, $i, 0, @testDeclarations;
            last;
        } elsif ($header[$i] =~ /^\#endif/ || ($header[$i] =~ /^class\s+([0-9A-Za-z_]+)/ && $1 gt $testClassName)) {
            splice @header, $i, 0, ("class $testClassName : public CxxTest::TestSuite {\n",
                                    " public:\n",
                                    @testDeclarations,
                                    "};\n",
                                    "\n");
            last;
        } else {
            ++$i;
        }
    }

    # Write it out
    open HEADER, "> u/$testHeaderName" or die "u/$testHeaderName: $!";
    print HEADER @header;
    close HEADER;
}

sub generateTestFile {
    # Load existing file
    my @test;
    if (open TEST, "< u/$testFileName") {
        @test = <TEST>;
        close TEST;
    } else {
        push @newFiles, "u/$testFileName";
        @test = ("/**\n",
                 "  *  \\file u/$testFileName\n",
                 "  *  \\brief Test for ".join('::', @namespace, $className)."\n",
                 "  */\n",
                 "\n",
                 "#include \"$classHeaderName\"\n",
                 "\n",
                 "#include \"$testHeaderName\"\n",
                 "\n");
    }

    # Generate tests
    foreach (@testNames) {
        push @test, ($testComment,
                     "void\n",
                     $testClassName."::$_()\n",
                     "{\n",
                     $testCode,
                     "}\n",
                     "\n");
    }

    # Write it out
    open TEST, "> u/$testFileName" or die "u/$testFileName: $!";
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
