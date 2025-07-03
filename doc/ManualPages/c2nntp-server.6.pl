start('c2nntp-server', 'PCC2 NNTP Server');

section('Synopsis',
        paragraph_synopsis('<b>c2nntp-server --help</b>'),
        paragraph_synopsis('<b>c2nntp-server</b> [<b>-</b><i>OPTIONS</i>]'));

section('Description',
        paragraph_text('<i>c2nntp-server</i> is a microservice that is part of the PlanetsCentral suite.',
                       'It provides a NNTP front-end to a <link>c2talk-server(6)</link> forum.',
                       'Users are authenticated against <link>c2user-server(6)</link>.'),
        paragraph_default_server_intro('c2nntp-server'));

section('Options',
        paragraph_default_server('NNTP'));

section('Environment',
        paragraph_default_server_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)', 'c2talk-server(6)', 'c2user-server(6)'));
