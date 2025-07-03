start('c2doc-server', 'PCC2 Documentation Server');

section('Synopsis',
        paragraph_synopsis('<b>c2doc-server --help</b>'),
        paragraph_synopsis('<b>c2doc-server</b> [<b>-</b><i>OPTIONS</i>]'));

section('Description',
        paragraph_text('<i>c2doc-server</i> is a microservice that is part of the PlanetsCentral suite.',
                       'It publishes a set of documentation files using a network protocol.'),
        paragraph_default_server_intro('c2doc-server'));

section('Options',
        paragraph_default_server('DOC'));

section('Environment',
        paragraph_default_server_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
