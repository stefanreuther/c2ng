start('c2router-server', 'PCC2 Router Server');

section('Synopsis',
        paragraph_synopsis('<b>c2router-server --help</b>'),
        paragraph_synopsis('<b>c2router-server</b> [<b>-</b><i>OPTIONS</i>]'));

section('Description',
        paragraph_text('<i>c2router-server</i> is a microservice that is part of the PlanetsCentral suite.',
                       'It manages multiple <link>c2play-server(6)</link> sessions and routes command to them.'),
        paragraph_default_server_intro('c2router-server'));

section('Options',
        paragraph_default_server());

section('Environment',
        paragraph_default_server_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)', 'c2play-server(6)'));
