##
##  Configuration section
##

use Cwd ('abs_path');

load_module ('Configure.pl');

find_compiler();
find_compiler_options("-I".normalize_filename($V{IN}),
                      "-I".normalize_filename($V{OUT}),
                      qw(-g -O2 -ansi -pedantic -fmessage-length=0 -Wno-long-long -Wconversion -mthreads -W -Wall));
find_archiver();

# Variables
add_variable(RUN             => '');
add_variable(PERL            => $^X);
add_variable(WITH_COVERAGE   => 0);
add_variable(COVERAGE_RESULT => "coverage_report");
add_variable(RM              => 'rm -f');
add_variable(LIBS            => '');
add_variable(LDFLAGS         => '');
add_variable(LIBDEPEND       => '');

add_variable(ENABLE_GUI      => 1);
add_variable(ENABLE_SERVER   => 1);
add_variable(ENABLE_CONSOLE  => 1);
add_variable(ENABLE_DOC      => 0);
add_variable(ENABLE_TEST_APPS => 0);
add_variable(ENABLE_BUILD    => 1);

my $IN = $V{IN};

# Targets (previously maintained in P9/Settings as 'TARGETS' variable)
if (get_variable('ENABLE_BUILD')) {
    my @GUI_APPS     = qw(c2ng c2pluginw);
    my @SERVER_APPS  = qw(c2file-server c2format-server c2host-server c2talk-server c2fileclient c2mailin c2console c2dbexport c2nntp-server c2mailout-server c2monitor-server c2play-server c2router-server c2user-server c2logger);
    my @CONSOLE_APPS = qw(c2check c2configtool c2export c2mgrep c2mkturn c2plugin c2rater c2script c2simtool c2sweep c2unpack c2untrn c2gfxgen);
    my @LIBS         = qw(guilib serverlib gamelib);

    # Find afl
    find_directory('AFL_DIR',
                   files => [qw(config.mk include/afl/base/types.hpp include/cxxtest/TestController.h)],
                   guess => [glob('../afl* ../afl*/result ../../afl* ../../afl*/result')]);
    add_to_variable(CXXFLAGS => "-I$V{AFL_DIR}/include");

    my $afl_config_file = "$V{AFL_DIR}/config.mk";
    my $afl_config = load_variables($afl_config_file);
    add_to_variable(LDFLAGS => $afl_config->{CONFIG_AFL_LDFLAGS});

    # Find cxxtest
    find_directory('CXXTESTDIR',
                   files =>[qw(cxxtest/TestSuite.h cxxtestgen.pl)],
                   guess => [glob('../cxxtest* ../../cxxtest* ../../../cxxtest*')],
                   allow_missing => 1);

    # Find SDLs
    set_variable(WITH_SDL =>
                 $V{ENABLE_GUI} &&
                 find_library('WITH_SDL',
                              name    => 'SDL',
                              program => "#include <SDL.h>\n#undef main\nint main() { SDL_Init(0); }\n",
                              libs    => '-lSDL',
                              pkg     => 'sdl',
                              dir     => add_variable(SDL_DIR => '')));

    set_variable(WITH_SDL_IMAGE =>
                 $V{WITH_SDL} &&
                 find_library('WITH_SDL_IMAGE',
                              name    => 'SDL_image',
                              program => "#include <SDL_image.h>\n#undef main\nint main() { IMG_Load_RW(0, 0); }\n",
                              libs    => '-lSDL_image',
                              pkg     => 'SDL_image',
                              dir     => add_variable(SDL_IMAGE_DIR => '')));
    set_variable(WITH_SDL2 =>
                 !$V{WITH_SDL} && $V{ENABLE_GUI} &&
                 find_library('WITH_SDL2',
                              name => 'SDL2',
                              program => "#include <SDL.h>\n#include <SDL_render.h>\n#undef main\nint main() { SDL_Init(0); }\n",
                              libs => '-lSDL2',
                              pkg => 'sdl2',
                              dir => add_variable(SDL2_DIR => '')));
    set_variable(WITH_SDL2_IMAGE =>
                 $V{WITH_SDL2} &&
                 find_library('WITH_SDL2_IMAGE',
                              name => 'SDL2_image',
                              program => "#include <SDL_image.h>\n#undef main\nint main() { IMG_Load_RW(0, 0); }\n",
                              libs => '-lSDL2_image',
                              pkg => 'SDL2_image',
                              dir => add_variable(SDL2_IMAGE_DIR => '')));


    # Target
    add_variable(TARGET => 'POSIX');
    if ($V{TARGET} =~ /POSIX/i) {
        add_to_variable(CXXFLAGS => "-DTARGET_OS_POSIX");
    } elsif ($V{TARGET} =~ /Win(32|64)/i) {
        add_to_variable(CXXFLAGS => "-DTARGET_OS_WIN32");
    } else {
        die "Error: the specified target '$V{TARGET}' is not known; provide correct 'TARGET=' option";
    }

    # Generate output
    file_update(normalize_filename($V{OUT}, 'config.h'),
                ("#define HAVE_SDL 1\n"        x $V{WITH_SDL}) .
                ("#define HAVE_SDL_IMAGE 1\n"  x $V{WITH_SDL_IMAGE}) .
                ("#define HAVE_SDL2 1\n"       x $V{WITH_SDL2}) .
                ("#define HAVE_SDL2_IMAGE 1\n" x $V{WITH_SDL2_IMAGE}));

    # Options for coverage
    if ($V{WITH_COVERAGE}) {
        add_to_variable(CXXFLAGS => "-O0 -fprofile-arcs -ftest-coverage");
        add_to_variable(LDFLAGS  => "-fprofile-arcs -ftest-coverage");
    }

    ##
    ##  Build section
    ##

    load_module ('Compiler.pl');

    compile_add_prebuilt_library(afl => "$V{AFL_DIR}/lib/libafl.a",
                                 [split /\s+/, $afl_config->{CONFIG_AFL_LIBS}]);

    my $prefix = get_variable('prefix');
    my $settings = load_variables("$IN/P9/Settings");

    # Libraries
    my $opts = {
        normalize_filename($IN, "gfx/gen/spaceview.cpp") => {
            CXXFLAGS => "-ffloat-store"
        }
    };
    foreach my $lib (@LIBS) {
        compile_static_library($lib, [to_prefix_list($IN, $settings->{"FILES_$lib"})], ['afl'], $opts);
    }

    # Local copy of resources for out-of-tree execution
    if (normalize_filename("$V{OUT}/share") ne normalize_filename("$IN/share")) {
        generate_copy_tree('resources', "$V{OUT}/share", "$IN/share");
        rule_set_phony('resources');
    } else {
        generate('resources', "$IN/share");
    }

    # Build stuff
    my @apps;
    push @apps, @CONSOLE_APPS if $V{ENABLE_CONSOLE};
    push @apps, @GUI_APPS     if $V{WITH_SDL2} || $V{WITH_SDL};
    push @apps, @SERVER_APPS  if $V{ENABLE_SERVER};
    foreach my $app (@apps) {
        my $exe = compile_executable($app,
                                     [to_prefix_list($IN, $settings->{"FILES_$app"})],
                                     [split /\s+/, $settings->{"DEPEND_$app"}]);
        generate('install', generate_copy_strip("$prefix/bin/$exe", $exe));
        generate('all', $exe);
    }

    # Testsuite
    if ($V{CXXTESTDIR} ne '') {
        my @ts_src = sort(to_prefix_list($IN, $settings->{FILES_testsuite}));
        my @ts_obj = map {compile_file($_, {CXXFLAGS=>"-I$V{CXXTESTDIR} -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -O0"})} @ts_src;

        # - build test driver
        my $ts_main_src = normalize_filename($V{TMP}, "testsuite.cpp");
        my @ts_headers = <$IN/u/t_*.hpp>;
        generate($ts_main_src, [@ts_headers], "$V{PERL} $V{CXXTESTDIR}/cxxtestgen.pl --gui=TestController --have-eh --error-printer -o $ts_main_src ".join(' ', @ts_headers));
        rule_add_info($ts_main_src, 'Generating test driver');

        # - compile test driver
        #   "-g0": mingw fails with "testsuite.o: too many section"; this removes some 40% of the sections
        my @ts_main_obj = map {compile_file($_, {CXXFLAGS=>"-I$V{CXXTESTDIR} -I$V{AFL_DIR} -g0 -O0"})} $ts_main_src;

        # - compile & run
        my $exe = compile_executable('testsuite', [@ts_main_obj, @ts_obj], [@LIBS, 'afl']);
        generate('test', ['testsuite', 'resources'], "$V{RUN} ./testsuite");
        rule_add_info('test', 'Running testsuite');
        rule_set_phony('test');

        # Coverage
        if ($V{WITH_COVERAGE}) {
            # Empty coverage (baseline)
            my $base = generate_anonymous('.info', $exe,
                                          "lcov -q -c -d $V{TMP} -i -o \$@");

            # Test run
            my $result = generate_anonymous('.info', $exe,
                                            "lcov -q -z -d $V{TMP}",            # Reset counters (delete all *.gcda)
                                            "$V{RUN} ./$exe",                # Run testee
                                            "lcov -q -c -d $V{TMP} -o \$@");    # Capture coverage (*.gcda -> info)

            # Combine captured and baseline so we can see unreferenced lines
            my $combined = generate_anonymous('.info', [$base, $result], "lcov -q -a $base -a $result -o \$@");

            # Filter only source code
            my $abs_in = abs_path($V{IN});
            my $abs_tmp = abs_path($V{TMP});
            my $filtered_source = generate_anonymous('.info', [$combined], "lcov -q -e \$< '$abs_in/*' -o \$@");
            my $filtered_source2 = generate_anonymous('.info', [$filtered_source], "lcov -q -r \$< '$abs_tmp/*' -o \$@");
            my $filtered = generate_anonymous('.info', $filtered_source2, "lcov -q -r \$< '$abs_in/u/*' -o \$@");

            # Optimize
            my $script = normalize_filename($V{AFL_DIR}, 'bin/coverage_optimize.pl');
            my $optimized = generate_anonymous('.info', [$filtered, $script], "$V{PERL} $script <\$< >\$@");

            # Generate HTML
            my $rm = $V{RM};
            my $report = $V{COVERAGE_RESULT};
            generate("$report/index.html", $optimized,
                     "$rm -r $result/*",              # "/*" to keep the .mark file intact which buildsys creates for us
                     "genhtml -q --ignore-errors source -t c2ng -o $report \$<");
            generate("coverage", "$report/index.html");
            rule_set_phony('coverage');
        }
    }

    # Test apps
    if ($V{ENABLE_TEST_APPS}) {
        load_directory('testapps');
    }
}

##
##  Manual
##

if ($V{ENABLE_DOC}) {
    load_directory('doc');
}

##
##  Additional Stuff
##

my $tagfile = add_variable(TAGFILE => 'TAGS');
generate('tags', [], "(cd $IN && etags -f - --recurse client game gfx interpreter main server tools u ui util) > $tagfile");
rule_set_phony('tags');


##
##  Installation section
##

if (get_variable('ENABLE_BUILD')) {
    my $prefix = get_variable('prefix');
    foreach (qw(COPYING NEWS README TODO)) {
        generate('install', generate_copy("$prefix/$_.txt", "$IN/$_.txt"));
    }
    generate_copy_tree('install', "$prefix/share", "$IN/share");
    generate_copy_tree('install', "$prefix/share/server/scripts", "$IN/server/scripts");
    rule_set_phony('install');
}

# FIXME: should be in core?
sub generate_copy_tree {
    my ($rule, $out, $in) = @_;
    if (-d $in) {
        # Directory
        opendir my $dir, $in or die;
        foreach my $e (sort readdir $dir) {
            if ($e !~ /^\./ && $e ne 'CVS' && $e !~ /~$/) {
                generate_copy_tree($rule, normalize_filename($out, $e), normalize_filename($in, $e));
            }
        }
        add_input_file($in);
        # rule_rebuild_directory($out); <- FIXME: conflicts with that we build share/server from two sources
        closedir $dir;
    } else {
        # File
        generate($rule, generate_copy($out, $in));
    }
}

sub generate_copy_strip {
    my ($out, $in) = @_;
    my $strip = add_variable(STRIP => 'strip');
    generate(generate_copy($out, $in),
             [],
             "$strip \$@");
}
