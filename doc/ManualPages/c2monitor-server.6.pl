start('c2monitor-server', 'PCC2 Monitor Server');

section('Synopsis',
        paragraph_synopsis('<b>c2monitor-server --help</b>'),
        paragraph_synopsis('<b>c2monitor-server</b> [<b>-</b><i>OPTIONS</i>]'));

section('Description',
        paragraph_text('<i>c2monitor-server</i> is a microservice that is part of the PlanetsCentral suite.',
                       'It monitors other PlanetsCentral servers, and publishes the result via HTTP.'),
        paragraph_default_server_intro('c2monitor-server'));

section('Options',
        paragraph_default_server());

section('Environment',
        paragraph_default_server_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
