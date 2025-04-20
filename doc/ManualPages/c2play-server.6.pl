start('c2play-server', 'PCC2 Play Server');

section('Synopsis',
        paragraph_synopsis('<b>c2play-server -h</b>'),
        paragraph_synopsis('<b>c2play-server</b> [<b>-</b><i>OPTIONS</i>] <i>PLAYER</i> <i>GAMEDIR</i> [<i>ROOTDIR</i>]'));

section('Description',
        paragraph_text('<i>c2play-server</i> provides script-based access to VGA Planets&trade; game data.',
                       'It takes commands on standard input, and produces responses on standard output.'),
        paragraph_text('The game directory can be either a file system directory name, or an URI of the form',
                       '"<b>c2file://</b><i>USER</i><b>@</b><i>HOST</i><b>:</b><i>PORT</i><b>/</b><i>DIR</i>"',
                       'to access a game directory stored in a <link>c2file-server(6)</link> instance.'),
        paragraph_text('<i>c2play-server</i> is usually run within session multiplexer <link>c2router-server(6)</link>,',
                       'and operates as back-end to PCC2 Web.'));

section('Options',
        paragraph_default_help(';'),
        paragraph_detail('<b>-C</b><i>CHARSET</i>',
                         paragraph_text('Set <i>game character set</i>;')),
        paragraph_detail('<b>-R</b><i>KEY</i>, <b>-W</b><i>KEY</i>',
                         paragraph_text('Ignored. <link>c2router-server(6)</link> uses these for detection of conflicting sessions;')),
        paragraph_detail('<b>-D</b><i>KEY</i><b>=</b><i>VALUE</i>',
                         paragraph_text('Define a property.',
                                        'This is a way to pass information to the web application;',
                                        'properties are published as attributes of obj/main;')),
        paragraph_detail(text_option('--language=', 'CODE'),
                         paragraph_text('Select language to use for game.',
                                        'If <i>c2play-server</i> prepares text to show to the user (e.g. query/istatX.X),',
                                        'it will use this language.')));

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)', 'c2router-server(6)'));
