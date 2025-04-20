start('c2simtool', 'PCC2 Battle Simulation Utility');

section('Synopsis',
        paragraph_synopsis('<b>c2simtool</b> [<i>OPTIONS</i>] <i>FILE</i>...'));

section('Description',
        paragraph_text('<i>c2simtool</i> provides a command-line interface to the PCC2 battle simulator.',
                       'It will load one or more battle simulation files (<fn>.ccb</fn>), merge them into one simulation setup, and perform operations on that.'),
        paragraph_text('<fn>.ccb</fn> files can be created with PCC2, PlayVCR (Linux/Windows), CCBSim (DOS), or the PCC2 Web simulator at <link>https://planetscentral.com/play/sim.cgi</link>.'));

section('Options');

subsect('Actions',
        paragraph_text('At least one action must be specified.',
                       'If multiple actions are specified, they are executed in the order as shown here.'),
        paragraph_detail(text_option('--save', 'OUT.ccb', '-o', 'OUT.ccb'),
                         paragraph_text('Save simulation setup into a .ccb file;')),
        paragraph_detail(text_option('--report', '', '-r', ''),
                         paragraph_text('Show a report listing all ships from the simulation setup;')),
        paragraph_detail(text_option('--verify'),
                         paragraph_text('Verify that simulation setup matches the ship list.',
                                        'If verification fails, <i>c2simtool</i> exits with nonzero exit code;')),
        paragraph_detail(text_option('--run', 'N'),
                         paragraph_text('Run <i>N</i> simulations;')),
        paragraph_detail(text_option('--run-series'),
                         paragraph_text('Run a series of simulations.',
                                        'The length of the series is determined from the simulation settings.',
                                        'With <b>--seed-control</b>, a series will produce every possible result;')),
        paragraph_default_help());

subsect('Options',
        paragraph_detail(text_option('--game', 'DIR', '-G', 'DIR'),
                         paragraph_text('Set game directory.',
                                        'This directory should contain a ship list, configuration files, etc.',
                                        'Defaults to current directory;')),
        paragraph_detail(text_option('--root', 'DIR', '-R', 'DIR'),
                         paragraph_text('Set root directory.',
                                        'This directory should contain ship list and configuration files not present in the game directory.',
                                        "Defaults to PCC2's built-in defaults;")),
        paragraph_detail(text_option('--charset', 'CS', '-C', 'CS'),
                         paragraph_text('Set game character set;')),
        paragraph_detail(text_option('-q'),
                         paragraph_text('Suppress some status messages;')),
        paragraph_default_log());

subsect('Simulation Options',
        paragraph_text('These options are only relevant when simulations are run.'),
        paragraph_detail(text_option('--jobs', 'N', '-j', 'IN'),
                         paragraph_text('Set number of threads for simulation;')),
        paragraph_detail(text_option('--mode', 'MODE'),
                         paragraph_text('Set mode (host, phost2, phost3, phost4, flak, nuhost).',
                                        'Setting a mode also sets defaults for <b>--esb</b>, <b>--scotty</b>, <b>--random-sides</b>, and <b>--balance</b>;')),
        paragraph_detail(text_option('--esb', 'N'),
                         paragraph_text('Set engine-shield bonus to N%;')),
        paragraph_detail(text_option('--scotty', '', '--no-scotty', ''),
                         paragraph_text('Enable/disable scotty bonus;')),
        paragraph_detail(text_option('--random-sides', '', '--no-random-sides', ''),
                         paragraph_text('Enable/disable random left/right;')),
        paragraph_detail(text_option('--alliances', '', '--no-alliances', ''),
                         paragraph_text('Enable/disable alliance support.',
                                        'As of 20201010, this has no effect;')),
        paragraph_detail(text_option('--one', '', '--no-one', ''),
                         paragraph_text('If enabled, generate only one fight per simulation;')),
        paragraph_detail(text_option('--seed-control', '', '--no-seed-control', ''),
                         paragraph_text('Enable/disable seed control.',
                                        'Supported for <b>host</b> and <b>nuhost</b>.',
                                        'When enabled, all possible random seeds are simulated, and therefore all possible results generated.',
                                        'Implies <b>--one</b>;')),
        paragraph_detail(text_option('--random-fc', '', '--no-random-fc', ''),
                         paragraph_text('Enable/disable random friendly code on every fight;')),
        paragraph_detail(text_option('--balance', 'MODE'),
                         paragraph_text('Set balancing mode (none, 360, master);')),
        paragraph_detail(text_option('--seed', 'N'),
                         paragraph_text('Set random number seed.',
                                        'Unless seed control is used, this seeds the random number generator used to generate starting seeds for the individual fights (VCRs),',
                                        'and to generate other random effects (e.g. random left/right).',
                                        'The default seed is derived from the current time, giving possibly-different results for each invocation.',
                                        'This option does <i>not</i> set the actual VCR seed!')));

section('Simulation',
        paragraph_text('The simulator can run many simulations from one setup.',
                       'This will provide class results and unit results.'),
        paragraph_text('<b>Class results</b> report the frequencies of possible outcomes (e.g. "75%: 2 Klingon ships survive").'),
        paragraph_text('<b>Unit results</b> report the results for each individual unit.',
                       'In addition to the number of battles survived/fought/being captured, it reports average/minimum/maximum of a number of values',
                       '(e.g. "Crew Left: 96.1 (87..201)": ship survives with 87 to 201 crew; average is 96.1).'));

section('MergeCCB',
        paragraph_text('This utility replaces the "mergeccb" utility for DOS.'),
        paragraph_text('Instead of "mergeccb -r FILE.ccb ...", use "c2simtool -r FILE.ccb ...".'),
        paragraph_text('Instead of "mergeccb -c FILE.ccb ... OUT.ccb", use "c2simtool FILE.ccb ... -o OUT.ccb".'));

section('Environment',
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
