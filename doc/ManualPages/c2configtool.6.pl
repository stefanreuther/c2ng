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
                         paragraph_text('Load <fn>hconfig.hst</fn> file into "PHost" section;')),
        paragraph_detail('<b>-D</b><i>SEC.KEY</i><b>=</b><i>VALUE</i>',
                         paragraph_text('Set value. If the value already exists, it is replaced; otherwise, it is added;')),
        paragraph_detail('<b>-A</b><i>SEC.KEY</i><b>=</b><i>VALUE</i>',
                         paragraph_text('Add value to file.',
                                        'This can be used to add multiple copies of an assignment;')),
        paragraph_detail('<b>-U</b><i>SEC.KEY</i>',
                         paragraph_text('Remove a value.')));

subsect('Output Actions',
        paragraph_detail('<b>-o</b> <i>FILE</i>',
                         paragraph_text('Save result to a file;')),
        paragraph_detail('<b>--stdout</b>',
                         paragraph_text('Write result to standard output;')),
        paragraph_detail('<b>--get=</b><i>SEC.KEY</i>',
                         paragraph_text('Get option value and write it to standard output;')),
        paragraph_detail('<b>--save-hconfig=</b><i>FILE</i>',
                         paragraph_text('Write content of "PHost" section to <fn>hconfig.hst</fn> file.')));

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
