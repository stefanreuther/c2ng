start('c2mgrep', 'PCC2 Message Search');

section('Synopsis',
        paragraph_synopsis('<b>c2mgrep -h</b>'),
        paragraph_synopsis('<b>c2mgrep</b> [<b>-</b><i>OPTS</i>] <i>TEXT</i> {[<b>-</b><i>TYPE</i>] <i>FILE</i>...} ...'));

section('Description',
        paragraph_text('<i>c2mgrep</i> searches VGA Planets&trade; message files for text (currently no regexps, sorry).',
                       'It supports a variety of file formats.',
                       'By default, it auto-detects the file type.',
                       'You can override the file type by specifying a file type option, which applies to all subsequent file names.'));

section('Options',
        paragraph_default_help(';'),
        paragraph_detail(text_option('-c'),
                         paragraph_text('Search <i>case-sensitive</i>.',
                                        'Default is case-insensitive;')),
        paragraph_detail(text_option('-C', 'CHARSET'),
                         paragraph_text('Set <i>character set</i> for file content.',
                                        'File content will be translated to your console character set, as per usual PCC2 operation;')),
        paragraph_detail(text_option('-r'),
                         paragraph_text('File type: <i>result</i> files (<fn>*.rst</fn>);')),
        paragraph_detail(text_option('-t'),
                         paragraph_text('File type: <i>turn</i> files (<fn>*.trn</fn>);')),
        paragraph_detail(text_option('-m'),
                         paragraph_text('File type: <i>message</i> inbox (<fn>mdata*.dat</fn>);')),
        paragraph_detail(text_option('-d'),
                         paragraph_text('File type: <i>Dosplan</i> outbox (<fn>mess*.dat</fn>);')),
        paragraph_detail(text_option('-w'),
                         paragraph_text('File type: <i>Winplan</i> outbox (<fn>mess35*.dat</fn>);')),
        paragraph_detail(text_option('-a'),
                         paragraph_text('File type: VPA database;')),
        paragraph_detail(text_option('-A'),
                         paragraph_text('<i>Automatic</i> file type detection.',
                                        'This is the default.',
                                        'Note that file type detection is based upon file content, not names.',
                                        'Thus, <i>c2mgrep</i> works equally well on renamed files (such as turn file backups, "player9.043").',
                                        'In very rare cases, <i>c2mgrep</i> may mis-detect a file; in that case, use one of the file type options;')),
        paragraph_detail(text_option('-z'),
                         paragraph_text('Search in <i>zip</i> files.',
                                        '<i>c2mgrep</i> has built-in support for standard ("PKZip 2.0") .zip files.',
                                        'With this option, it will look for files of the above types within those archive files.',
                                        'This option implies <b>-A</b>, any file type option will render <b>-z</b> ineffective;')),
        paragraph_detail(text_option('-I'),
                         paragraph_text('<i>Ignore</i> unknown files.',
                                        'Normally, <i>c2mgrep</i> warns when encountering a file it cannot identify.',
                                        'This option is automatically enabled for the content of archive files,',
                                        'as there is no way to restrict searching only to particular files within archives;')),
        paragraph_detail(text_option('-f').', '.text_option('-n'),
                         paragraph_text('Ignored. In <i>mgrep</i> (the DOS predecessor of <i>c2mgrep</i>),',
                                        'these options enable and disable output in mailbox format.',
                                        'This format is the only output format of <i>c2mgrep</i>.')));

section('Output Format',
        paragraph_text('Each message is preceded by a line'),
        paragraph_indent('--- Message 14 (/path/to/file) ---'),
        paragraph_text('When possible, each message is also preceded by a "TURN:" header containing the turn number.'),
        paragraph_text('This format is called "Mailbox format", because PCC can use this information to split <i>c2mgrep</i> output',
                       'back into individual messages for viewing',
                       '(for dividers, it triggers on three dashes and the word "Message").'));

section('Limitations',
        paragraph_text('#Although PCC2 tries to present a uniform view on message files, their content is stored in very different formats.',
                       '<i>c2mgrep</i> operates on the raw message files, and output therefore matches the file content.',
                       'In particular,'),
        paragraph_list('search covers headers for incoming messages, but not for outgoing messages',
                       'not all messages contain all headers ("&lt;&lt; header &gt;&gt;", FROM, TO, TURN)',
                       'outgoing messages to multiple players appear multiple times except in Winplan outbox files'),
        paragraph_text('This program does not support RAR archives like the DOS version of <i>mgrep</i> does.'));

section('Environment',
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
