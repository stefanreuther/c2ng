start('c2talk-server', 'PCC2 Talk Server');

section('Synopsis',
        paragraph_synopsis('<b>c2talk-server --help</b>'),
        paragraph_synopsis('<b>c2talk-server</b> [<b>-</b><i>OPTIONS</i>]'));

section('Description',
        paragraph_text('<i>c2talk-server</i> is a microservice that is part of the PlanetsCentral suite.',
                       'It provides a forum with public and private messaging functions.'),
        paragraph_default_server_intro('c2talk-server'));

section('Options',
        paragraph_default_server('TALK'));

section('Environment',
        paragraph_default_server_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
