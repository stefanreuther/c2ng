start('c2script', 'PCC2 Script Engine');

section('Synopsis',
        paragraph_synopsis('<b>c2script</b> [<i>ACTION</i>] [<i>OPTIONS</i>] <i>FILE</i>...'),
        paragraph_synopsis('<b>c2script</b> [<i>ACTION</i>] [<i>OPTIONS</i>] <b>-k</b> <i>COMMAND</i>...'));

section('Description',
        paragraph_text('<i>c2script</i> is the PCC2 script engine.',
                       'It can run scripts with input and output to the console, without graphical output.'));

section('Options',
        paragraph_default_help(';'),
        paragraph_detail(text_option('--game', 'DIR', '-G', 'DIR'),
                         paragraph_text('Set game directory;')),
        paragraph_detail(text_option('--root', 'DIR', '-R', 'DIR'),
                         paragraph_text('Set root directory;')),
        paragraph_detail(text_option('--player', 'NUM', '-P', 'NUM'),
                         paragraph_text('Set player number. Not required if game directory contains only one player;')),
        paragraph_detail(text_option('--readonly'),
                         paragraph_text('Read-only operation; game data is not saved after executing the script;')),
        paragraph_detail(text_option('--nostdlib'),
                         paragraph_text('Do not load standard library (core.q), do not add standard path to search path;')),,
        paragraph_detail(text_option('-I', 'PATH'),
                         paragraph_text('Add load path.',
                                        'The <b>Load\fR and \fBTryLoad</b> statements are resolved against this path.',
                                        'This option can be specified multiple times to look in multiple directories;')),
        paragraph_detail(text_option('--charset', 'CS', '-C', 'CS'),
                         paragraph_text('Set game character set;')),
        paragraph_detail(text_option('--coverage', 'FILE.info'),
                         paragraph_text('Record coverage, and produce a report in lcov <fn>.info</fn> format.',
                                        'You can use <link>lcov(1)</link> to manipulate the coverage report,',
                                        'and <link>genhtml(1)</link> to convert it onto a user-friendly report;')),
        paragraph_detail(text_option('--coverage-test-name', 'NAME'),
                         paragraph_text('Set the test name ("TN:" tag) to write into the coverage report;')),
        paragraph_default_optimizer(';'),
        paragraph_default_command(';'),
        paragraph_default_log(';'),
        paragraph_detail(text_option('-q'),
                         paragraph_text('Configure log output to show script output only, no status messages.')));

section('Environment',
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)', 'c2compiler(6)'));
