start('c2unpack', 'PCC2 Result File Unpacker');

section('Synopsis',
        paragraph_synopsis('<b>c2unpack -h</b>'),
        paragraph_synopsis('<b>c2unpack</b> [<b>-</b><i>OPTIONS</i>] [<i>PLAYERS</i>...] [<i>GAMEDIR</i>]'));

section('Description',
        paragraph_text('<i>c2unpack</i> unpacks a VGA Planets&trade; result file (<fn>playerX.rst</fn>) into a set of separate files.',
                       'Many utilities need the result file in unpacked form to be able to work with it.',
                       'After playing, you have to pack the changes into a TRN file using a program such as <link>c2mkturn(6)</link>.'),
        paragraph_text('Note that PCC2 does not require you to unpack RST files.',
                       'PCC2 can read RST files and write TRN files directly.',
                       'However, <i>c2unpack</i> allows a little more control in special situations.'),
        paragraph_text("When you specify a set of player numbers, only those players' files are unpacked.",
                       "If you do not specify player numbers, <i>c2unpack</i> unpacks all files it finds in the game directory."),
        paragraph_text('Note that <i>c2unpack</i> never deletes the RST files, like traditional unpackers do.',
                       'It will, however, delete obsolete files from the game directory to avoid conflicts with',
                       'the files it creates (for example, <fn>skoreX.dat</fn> files which do not appear in every RST'),
        paragraph_text('By default, <i>c2unpack</i> checks your RST as well as your <fn>utilX.dat</fn> file for attachments.',
                       'An attachment is a file that can have a possibly longer lifetime than the regular files contained in the RST,',
                       'for example, a race-name file or a configuration file.'),
        paragraph_text('Note that this version of <i>c2unpack</i> does not require a <i>ROOTDIR</i> parameter;',
                       'a directory name given after <i>GAMEDIR</i> is ignored.'));

section('Options',
        paragraph_default_help(';'),
        paragraph_detail(text_option('-w'),
                         paragraph_text('Create Windows (3.5) file format.',
                                        'This is the default setting, and the preferred format when you play with PCC or PCC2;')),
        paragraph_detail(text_option('-d'),
                         paragraph_text('Create DOS (3.0) file format.',
                                        'Note that this setting does not prevent <i>c2unpack</i> from unpacking the version-3.5 part',
                                        'of the result file.',
                                        'Use <b>-a</b> to force <i>c2unpack</i> to ignore the version-3.5 part and behave like a "real" 3.0 unpack;')),
        paragraph_detail(text_option('-a'),
                         paragraph_text('Ignore the version-3.5 part of the result file (pretend that the file is version 3.0);')),
        paragraph_detail(text_option('-t'),
                         paragraph_text('Create <fn>targetX.ext</fn> files.',
                                        'If the result file contains more than 50 targets (no matter whether they are in the',
                                        'regular target sections or in the version-3.5 part), the excess are put in <fn>targetX.ext</fn>.',
                                        'Use this for compatibility with older programs such as <i>VPUtil</i>;')),
        paragraph_detail(text_option('-n'),
                         paragraph_text('Do not attempt to fix host-side errors.',
                                        'By default, <i>c2unpack</i> fixes a few host-side errors that confuse some programs;')),
        paragraph_detail(text_option('-f'),
                         paragraph_text('Force unpack of files with checksum errors.',
                                        'Normally, <i>c2unpack</i> refuses to unpack files that have a checksum error, as that',
                                        'is normally caused by a transmission error;')),
        paragraph_detail(text_option('-x', '', '-v', ''),
                         paragraph_text('Display additional status messages.',
                                        'By default, <i>c2unpack</i> only displays the number of ships/planets/bases for each player;')),
        paragraph_detail(text_option('-R'),
                         paragraph_text('Do not accept host-side <i>race name changes</i> (<fn>race.nm</fn> file attachments).',
                                        "Note that messages received from host will use host's changed names,",
                                        'even if you prefer to keep your local names;')),
        paragraph_detail(text_option('-K'),
                         paragraph_text('Do not accept <i>configuration file attachments</i> from the host;')),
        paragraph_detail(text_option('-A'),
                         paragraph_text('Do not accept any <i>attachments</i>.',
                                        'This disables scanning <fn>utilX.dat</fn>;')),
        paragraph_detail(text_option('-u'),
                         paragraph_text('Unpack turn files as well.',
                                        'This will produce a game directory at the state when you last ran Maketurn.',
                                        'If no turn files exist or they do not match the result files (from a different turn, maybe),',
                                        'this option will not do anything;')),
        paragraph_default_log());

section('Limitations',
        paragraph_text('<i>c2unpack</i> makes no attempt to verify that all the RST files belong to the same game.',
                       'If they do not, the result will be detected as a conflict by PCC2.'),
        paragraph_text('In case a result file contains an error, <i>c2unpack</i> might leave a partially-unpacked',
                       'game directory around.',
                       'Use <i>c2sweep</i> to clean this up.'),
        paragraph_text('<i>c2unpack</i> never deletes the RST files, like traditional unpackers do.',
                       'All environments in which <i>c2unpack</i> runs allow you to write scripts (batch files) if you want this.'));

section('Environment',
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)', 'c2mkturn(6)', 'c2sweep(6)'));
