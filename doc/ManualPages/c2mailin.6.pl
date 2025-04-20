start('c2mailin', 'PCC2 Incoming Mail Processor');

section('Synopsis',
        paragraph_synopsis('<b>c2mailin --help</b>'),
        paragraph_synopsis('<b>c2mailin</b> [<b>-</b><i>OPTIONS</i>]'));

section('Description',
        paragraph_text('<i>c2mailin</i> reads a mail message from standard input and checks for attached turn files.',
                       'If any are found, they are submitted to <link>c2host-server(6)</link>.',
                       'The result is of the turn file submission is reported to the original sender of the mail,',
                       'via <link>c2mailout-server(6)</link>.'));

section('Options',
        paragraph_default_server(';'),
        paragraph_detail(text_option('--dump'),
                         paragraph_text('Show mail content on standard output instead of submitting it to <link>c2host-server(6)</link>.')));

section('Environment',
        paragraph_default_server_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)', 'c2host-server(6)', 'c2mailout-server(6)'));
