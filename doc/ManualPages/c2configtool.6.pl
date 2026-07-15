start('c2configtool', 'PCC2 Configuration Tool');

section('Synopsis',
        paragraph_synopsis('<b>c2configtool</b> [<b>-</b><i>OPTION</i> | <i>FILES</i> ...]'));

section('Description',
        paragraph_text('<i>c2configtool</i> is a tool to manipulate VGA Planets&trade; configuration files,',
                       'such as <fn>hconfig.hst</fn> or <fn>pconfig.hst</fn>.'),
        paragraph_text('The command line consists of a sequence of Load/Modify actions that is executed from left to right,',
                       'followed by an Output Action.'));

section('Options');
subsect('General',
        paragraph_default_help(';'),
        paragraph_detail(text_option('-w'), paragraph_text('<i>Whitespace</i> is significant in values.',
                                                           'By default, <i>c2configtool</i> will trim whitespace.')));

subsect('Load/Modify Actions',
        paragraph_detail('<i>FILE</i>',
                         paragraph_text('Load text file as configuration file (e.g. <fn>pconfig.hst</fn>).',
                                        'The first loaded file defines the structure of the output.',
                                        'Further loaded files are merged; their content updates the existing values and inserts new ones;')),
        paragraph_detail('<b>--empty</b>',
                         paragraph_text('Load empty file;')),
        paragraph_detail('<b>--load-hconfig=</b><i>FILE</i>',
                         paragraph_text('Load the given <fn>hconfig.hst</fn> file into "PHost" section;')),
        paragraph_detail('<b>--load-racenames=</b><i>FILE</i>',
                         paragraph_text('Load the given <fn>race.nm</fn> file into "Racenames" section.',
                                        'Creates a <b>Long</b><i>n</i>, <b>Short</b><i>n</i>, and <b>Adj</b><i>n</i> assignment for each player;')),
        paragraph_detail('<b>--load-truehull=</b><i>FILE</i>',
                         paragraph_text('Load the given <fn>truehull.dat</fn> file into "Truehull" section.',
                                        "For each slot (race's first hull, race's second hull, etc.), ",
                                        'creates one assignment <b>slot</b><i>n</i><b>=</b><i>p1</i><b>,</b><i>p2</i>... with the hull assignments for all races in that slot',
                                        '(that is, same format as an arrayized option in PConfig);')),
        paragraph_detail('<b>-D</b><i>SEC.KEY</i><b>=</b><i>VALUE</i>',
                         paragraph_text('Set value. If the value already exists, it is replaced; otherwise, it is added;')),
        paragraph_detail('<b>-A</b><i>SEC.KEY</i><b>=</b><i>VALUE</i>',
                         paragraph_text('Add value to file.',
                                        'This can be used to add multiple copies of an assignment;')),
        paragraph_detail('<b>-U</b><i>SEC.KEY</i>',
                         paragraph_text('Remove a value;')),
        paragraph_detail('<b>--shuffle=</b><i>A</i><b>,</b><i>B</i><b>,</b>...',
                         paragraph_text('Shuffle player-specific settings.',
                                        'Rebuilds all player array options by using the <i>A</i>th value for player 1,',
                                        '<i>B</i>th value for player 2, and so on.',
                                        'This can be used for setting up <i>PlayerRace</i> games.'),
                         paragraph_text('<b>c2configtool</b> has a built-in ignorelist to ignore options known to <i>not</i> contain player-specific values.',
                                        'For example, <i>WraparoundRectangle</i> or command lines in the <i>%pcontrol</i> section will not be modified.',
                                        'Otherwise, a value containing a comma will be considered player-specific, and will be updated.',
                                        'In particular, this means a truehull file can be rewritten using this option together with <b>--load-truehull</b> and <b>--save-truehull</b>.'),
                         paragraph_text('Note that this option will not shuffle race names in a file loaded with <b>--load-racenames</b>;',
                                        'that would not be a useful operation.')));

subsect('Output Actions',
        paragraph_detail('<b>-o</b> <i>FILE</i>',
                         paragraph_text('Save result to a file;')),
        paragraph_detail('<b>--stdout</b>',
                         paragraph_text('Write result to standard output;')),
        paragraph_detail('<b>--get=</b><i>SEC.KEY</i>',
                         paragraph_text('Get option value and write it to standard output;')),
        paragraph_detail('<b>--get-bool=</b><i>SEC.KEY</i>',
                         paragraph_text('Get boolean option value and return it as exit code.',
                                        'If the option is set and has a truthy value (Yes, True), exits successfully.',
                                        'If the option is unset or has a falsy value (No, False), exits unsuccessfully;')),
        paragraph_detail('<b>--save-hconfig=</b><i>FILE</i>',
                         paragraph_text('Write content of "PHost" section to the given <fn>hconfig.hst</fn> file;')),
        paragraph_detail('<b>--save-racenames=</b><i>FILE</i>',
                         paragraph_text('Write content of "Racenames" section to the given <fn>race.nm</fn> file.',
                                        'See <b>--load-racenames</b>;')),
        paragraph_detail('<b>--save-truehull=</b><i>FILE</i>',
                         paragraph_text('Write content of "Truehull" section to the given <fn>truehull.dat</fn> file.',
                                        'See <b>--load-truehull</b>.')));

section('File Format',
        paragraph_text('A configuration file consists of assignments of the form "Key=Value", one per line.',
                       'It can be structured in sections, using dividers of the form "% Section" or "[Section]".'),
        paragraph_text('A value addressed as <i>SEC.KEY</i> will place an assignment to <i>KEY</i> in the given section, <i>SEC</i>.',
                       'Use just <i>KEY</i> to place it in the anonymous section (beginning of file, before first section divider).'),
        paragraph_text('Section names and keys are case-insensitive, case-preserving.',
                       'If a new section is created by this command, it uses the "%" syntax.'),
        paragraph_text('Lines can be commented-out using ";" or "#".'));

section('Environment', paragraph_default_environment());

section('Author', paragraph_default_author());

section('See Also', paragraph_link_list('pcc2(6)'));
