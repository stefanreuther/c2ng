start('c2user-server', 'PCC2 User Server');

section('Synopsis',
        paragraph_synopsis('<b>c2user-server --help</b>'),
        paragraph_synopsis('<b>c2user-server</b> [<b>-</b><i>OPTIONS</i>]'));

section('Description',
        paragraph_text('<i>c2user-server</i> is a microservice that is part of the PlanetsCentral suite.',
                       'It provides management of user accounts (passwords, tokens).'),
        paragraph_default_server_intro('c2user-server'));

section('Options',
        paragraph_default_server());

section('Environment',
        paragraph_default_server_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
