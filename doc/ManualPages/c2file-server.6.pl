start('c2file-server', 'PCC2 File Server');

section('Synopsis',
        paragraph_synopsis('<b>c2file-server --help</b>'),
        paragraph_synopsis('<b>c2file-server</b> [<b>-</b><i>OPTIONS</i>]'));

section('Description',
        paragraph_text('<i>c2file-server</i> is a microservice that is part of the PlanetsCentral suite.',
                       'It serves a file hierarchy with optional metadata and access checking.'),
        paragraph_default_server_intro('c2file-server'));

section('Options',
        paragraph_default_server(';'),
        paragraph_detail(text_option('--instance=', 'NAME'),
                         paragraph_text('Set name of the server instance (default: "file").',
                                        'The instance name selects a set of configuration parameters from the configuration file;')),
        paragraph_detail(text_option('--nogc'),
                         paragraph_text('Disable garbage collection.',
                                        'For a content-addressable (git) backend, <i>c2file-server</i> will remove unused files on startup.',
                                        'This option disables the garbage collection procecss.')));

section('Environment',
        paragraph_default_server_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)', 'c2fileclient(6)'));
