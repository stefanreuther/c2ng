start('c2mailout-server', 'PCC2 Mailout Server');

section('Synopsis',
        paragraph_synopsis('<b>c2mailout-server --help</b>'),
        paragraph_synopsis('<b>c2mailout-server</b> [<b>-</b><i>OPTIONS</i>]'));

section('Description',
        paragraph_text('<i>c2mailout-server</i> is a microservice that is part of the PlanetsCentral suite.',
                       'It provides a mail queue and manages mail send permissions (confirmed opt-in) for addresses.',
                       'Messages are forwarded to an SMTP server after an email address is confirmed.'),
        paragraph_default_server_intro('c2mailout-server'));

section('Options',
        paragraph_default_server('MAILOUT', ';'),
        paragraph_detail(text_option('--notx'),
                         paragraph_text('Disable mail transmitter.',
                                        'Messages are only received and queued, but not sent.')));

section('Environment',
        paragraph_default_server_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
