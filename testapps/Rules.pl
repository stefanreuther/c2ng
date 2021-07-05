#
#  This directory contains random test applications.
#  They are NOT built by default.
#

my $target = 'all-test-apps';
my $IN = $V{IN};

build_test_app('browser',       ['gamelib', 'afl']);
build_test_app('dirbrowser',    ['gamelib', 'afl']);
build_test_app('overview',      ['gamelib', 'afl']);
build_test_app('processrunner', ['gamelib', 'afl']);
build_test_app('testvcr',       ['gamelib', 'afl']);
build_test_app('testflak',      ['gamelib', 'afl']);
build_test_app('msgparse',      ['gamelib', 'afl']);
build_test_app('ui_root',       ['guilib', 'gamelib', 'afl']);
build_test_app('threed',        ['guilib', 'gamelib', 'afl']);

rule_set_phony($target);


sub build_test_app {
    my ($name, $libs) = @_;

    # Place the executable one up ("..").
    # By default, this would generate the binaries in a testapps/ subdirectory.
    # This would cause them to start with a wrong data directory ("share/").
    generate($target, compile_executable("../$name", ["$IN/$name.cpp"], $libs));
}
