start('c2sweep', 'PCC2 Game Directory Cleaner');

section('Synopsis',
        paragraph_synopsis('<b>c2sweep -h</b>'),
        paragraph_synopsis('<b>c2sweep</b> [<b>-</b>nlx] [<b>-a</b>|<i>PLAYERS</i>...] [<i>GAMEDIR</i> [<i>ROOTDIR</i>]]'));

section('Description',
        paragraph_text('<i>c2sweep</i> cleans up game directories.',
                       'When invoked with a list of players, it will delete all their files.',
                       'When invoked with the <b>-a</b> option, it will delete the files of all players.'),
        paragraph_text('By default, only files belonging to the current turn are deleted.',
                       'The <b>-l</b> option also deletes files usually kept several turns, such as history databases.'));

section('Options',
        paragraph_default_help(';'),
        paragraph_detail(text_option('-n'),
                         paragraph_text("Only show which files would be deleted, but don't delete them (<i>no operation</i>);")),
        paragraph_detail(text_option('-l'),
                         paragraph_text('Also delete files usually kept over several turns (<i>long-lived files</i>);')),
        paragraph_detail(text_option('-a'),
                         paragraph_text("Delete all players' data;")),
        paragraph_detail(text_option('-x'),
                         paragraph_text('Display additional status messages, i.e. list files being deleted.')));

section('History',
        paragraph_text('A <i>sweep.bat</i> utility was included with VGA Planets&trade; since at least version 2.2.'),
        paragraph_text('PCC 1.x included a <i>CCSweep</i> utility that added player selection and the options',
                       '<b>-l</b> and <b>-x</b>.'),
        paragraph_text('This version is a reimplementation and expansion of <i>CCSweep 1.06</i>.'));

section('Environment',
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
