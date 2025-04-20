start('c2format-server', 'PCC2 Format Server');

section('Synopsis',
        paragraph_synopsis('<b>c2format-server --help</b>'),
        paragraph_synopsis('<b>c2format-server</b> [<b>-</b><i>OPTIONS</i>]'));

section('Description',
        paragraph_text('<i>c2format-server</i> is a microservice that is part of the PlanetsCentral suite.',
                       'It provides file parsing services using a network protocol.',
                       'Clients can send a binary file and receive the parsed version back, or vice versa.'),
        paragraph_default_server_intro('c2format-server'));

section('Options',
        paragraph_default_server());

section('Environment',
        paragraph_default_server_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
