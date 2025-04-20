start('c2check', 'Turn Checker');

section('Synopsis',
        paragraph_synopsis('<b>c2check -h</b>'),
        paragraph_synopsis('<b>c2check</b> [<b>-</b><i>OPTION</i>] <i>PLAYER</i> [<i>GAMEDIR</i> [<i>ROOTDIR</i>]]')
    );

section('Description',
        paragraph_text('<i>c2check</i> is a turn checker.',
                       'It can check either unpacked game directories (default), or RST+TRN pairs,',
                       'and thus be used both on host and client side.'),
        paragraph_text('<i>c2check</i> will create a logfile (<fn>check.htm</fn>, <fn>check.log</fn>).'));

section('Options',
        paragraph_default_help(';'),
        paragraph_detail(text_option('-r'),
                         paragraph_text('Check <i>RST+TRN</i> pair;')),
        paragraph_detail(text_option('-H'),
                         paragraph_text('Write log file in <i>HTML</i> format (<fn>check.html</fn>). Default is plain-text (<fn>check.log</fn>);')),
        paragraph_detail(text_option('-c'),
                         paragraph_text('Validate <i>checksums</i>;')),
        paragraph_detail(text_option('-p'),
                         paragraph_text('Be extra <i>picky</i>. Reports values that are out of range even if not modified by the client, invalid crew,',
                                        'and invalid cargo transfers that are normally filtered by Maketurn;')),
        paragraph_detail(text_option('-z'),
                         paragraph_text('Do not warn about "-1" values.',
                                        'Those do not normally appear with host-generated files, but might appear with some buggy client software.')));

section('Exit Code',
        paragraph_detail('0', paragraph_text('Everything ok;')),
        paragraph_detail('1', paragraph_text('File access error (e.g. directory not found);')),
        paragraph_detail('2', paragraph_text('Problem found.')));

section('Environment',
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)', 'c2compiler(6)'));
