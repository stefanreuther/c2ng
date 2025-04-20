start('c2mkturn', 'PCC2 Turn File Compiler');

section('Synopsis',
        paragraph_synopsis('<b>c2mkturn -h</b>'),
        paragraph_synopsis('<b>c2mkturn</b> [<b>-f</b>] <i>GAMEDIR</i> [<i>ROOTDIR</i>]'));

section('Description',
        paragraph_text('<i>c2mkturn</i> compiles the commands given in an unpacked VGA Planets&trade; game directory',
                       'into a turn file (<fn>playerX.trn</fn>).',
                       'This turn file can then be sent to the host.'),
        paragraph_text('Note that PCC2 does not require you to unpack RST files.',
                       'PCC2 can read RST files and write TRN files directly.'),
        paragraph_text('You cannot specify a player number.',
                       '<i>c2mkturn</i> always generates turn files for all players.',
                       "This is required by VGA Planets&trade;'s copy protection scheme:",
                       'all turn files made using the same copy of the program must be made in one go.'));

section('Options',
        paragraph_default_help(';'),
        paragraph_detail(text_option('-f'),
                         paragraph_text('<i>Force</i> creation of turn files.',
                                        'Normally, <i>c2mkturn</i> refuses to run if the specified directory contains data from multiple games',
                                        '(use <link>c2sweep(6)</link> to clean this up first).')));

section('Environment',
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
