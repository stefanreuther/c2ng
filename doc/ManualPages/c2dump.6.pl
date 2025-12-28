start('c2dump', 'PCC2 File Dump Utility');

section('Synopsis',
        paragraph_synopsis('<b>c2dump -h</b>'),
        paragraph_synopsis('<b>c2dump</b> [<b>-t</b> <i>TYPE</i>] [<b>-C</b> <i>CHARSET</i>] <i>FILE</i> [[<b>-t</b> <i>TYPE</i>] <i>FILE</i> ...]'));

section('Description',
        paragraph_text('<i>c2dump</i> can parse some VGA Planets&trade;-related files into readable text.',
                       'It is intended for developers, hosts, and curious players.',
                       "<i>c2dump</i>'s output is intended to be readable to humans and programs."));

section('Options',
        paragraph_default_help(';'),
        paragraph_detail(text_option('-t', 'TYPE'),
                         paragraph_text('Force following files to be interpreted as the given <i>type</i> (see below).',
                                        "By default, <i>c2dump</i> derives the files' type from its name.",
                                        "This default can be restored using <b>-t auto</b>;")),
        paragraph_detail(text_option('-p'),
                         paragraph_text('Autodetect <i>player</i> files;')),
        paragraph_detail(text_option('-m'),
                         paragraph_text('Autodetect host (<i>master</i>) files;')),
        paragraph_detail(text_option('--charset', 'CS', '-C', 'CS'),
                         paragraph_text('Set game character set.')));

section('Exit Status',
        paragraph_text('<i>c2dump</i> exits successfully if all files given on the command line could be interpreted.',
                       'If a file fails autodetection, or cannot be opened, errorlevel 1 is returned.',
                       'The other files are nonetheless intepreted.'));

section('File Types',
        paragraph_text("By default, <i>c2dump</i> looks at a file's name to determine its type.",
                       "It consults the initial portion of the file name without the directory name,",
                       "which must match a particular token followed by a period or digit.",
                       "For example, <i>ship9.dat</i>, <i>ship.hst</i>, <i>ship.034</i> all match the initial portion <i>ship</i>,",
                       "whereas <i>shipxy.dat</i> and <i>ships.dat</i> do not."),
        paragraph_text("Some files cannot be reliably identified just by looking at the name.",
                       "For example, <i>genX.dat</i> and <i>gen.hst</i> have a very different file format.",
                       "In this case, you can use <b>-p</b> or <b>-m</b> to tell <i>c2dump</i> to assume player-side or host-side files, respectively."),
        paragraph_text("If your files have different names, you must use the <b>-t</b> option to specify the type explicitly."),
        paragraph_text("The following table lists all possible types for <b>-t</b> (first column), and all file name tokens (second column)."),
        paragraph_table('lll',
                        ['Type', 'Token', 'Content'],
                        0,
                        ['bdata',    'bdata',        'starbase files'],
                        ['beamspec', 'beamspec',     'beam specification file'],
                        ['engspec',  'engspec',      'engine specification file'],
                        ['gendat',   'gen (-p)',     'player/turn information'],
                        ['hullspec', 'hullspec',     'hull specification file'],
                        ['names',    'storm,planet', 'storm/planet name file'],
                        ['pdata',    'pdata',        'planet files'],
                        ['racenm',   'race',         'race name file'],
                        ['ship',     'ship',         'ship files'],
                        ['target',   'target',       'visual contacts'],
                        ['team',     'team',         'team definitions'],
                        ['torpspec', 'torpspec',     'torpedo specification file'],
                        ['truehull', 'truehull',     'hull/player assignments'],
                        ['vcr',      'vcr',          'VCR (combat) file'],
                        ['xyplan',   'xyplan',       'planet coordinate file']));

section('Output Format',
        paragraph_text('Output for each file starts with a simple header, listing the file name and type used,',
                       'enclosed in equal signs.'),
        paragraph_synopsis("=== Dump of file 'ship.hst' using type 'ship' ==="),
        paragraph_text('The file is split into records.',
                       'Each record is started with the record name in column one, followed by a colon.',
                       'Record data follows with indented <i>name=value</i> pairs.'),
        paragraph_synopsis("Ship:",
                           "  Ship_Id                        = 1",
                           "  Owner                          = 3",
                           "  FCode                          = 'mkt'",
                           "  Speed                          = 9",
                           "  Waypoint_delta                 = (0,0)"),
        paragraph_text('Values can be numbers, strings enclosed in quotes, or coordinate pairs.',
                       'Record and value names are single identifiers, using underscores instead of spaces,',
                       'and are not internationalized.'),
        paragraph_text('Some parts of files cannot be meaningfully interpreted, and are shown as hex dumps',
                       'using keyword <i>Unparsed</i>.'),
        paragraph_synopsis('Trailer:',
                           '  Unparsed = 2D 46 48 26 2C 2E 37 37 2A 34'));

section('Notes',
        paragraph_text("The <link>un-trn(6)</link> utility performs a similar operation to this one for turn files.",
                       "However, <i>un-trn</i> has additional functions to manipulate and verify turn files which this one lacks.",
                       "<i>c2dump</i> can only read files."));

section('Environment', paragraph_default_environment());

section('Author', paragraph_default_author());

section('See Also', paragraph_link_list('pcc2(6)', 'c2untrn(6)'));
