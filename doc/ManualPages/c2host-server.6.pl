start('c2host-server', 'PCC2 Host Server');

section('Synopsis',
        paragraph_synopsis('<b>c2host-server --help</b>'),
        paragraph_synopsis('<b>c2host-server</b> [<b>-</b><i>OPTIONS</i>]'));

section('Description',
        paragraph_text('<i>c2host-server</i> is a microservice that is part of the PlanetsCentral suite.',
                       'It provides management of a set of VGA Planets&trade; games, including metadata,',
                       'user subscriptions, and host scheduler.'),
        paragraph_default_server_intro('c2host-server'));

section('Options',
        paragraph_default_server(';'),
        paragraph_detail(text_option('--nocron'),
                         paragraph_text('Disable scheduler.',
                                        '<i>c2host-server</i> will accept commands normally, but not run host.')));

section('Environment',
        paragraph_default_server_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
