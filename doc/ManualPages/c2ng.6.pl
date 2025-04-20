start('c2ng', 'PCC2 Client');

section('Synopsis',
        paragraph_synopsis('<b>c2ng</b> [<b>-</b><i>OPTIONS</i>] [<b>-dir</b>] [<i>GAMEDIR</i>] [<i>PLAYER</i>]'));

section('Description',
        paragraph_text('<i>c2ng</i> is a graphical, interactive VGA Planets&trade; client.'),
        paragraph_text('This is the "next generation" version, replacing the previous <i>pcc-v2</i> program.',
                       'It supports local VGAP3 games,',
                       'games hosted on <link>https://planetscentral.com/</link>,',
                       'and games hosted on <link>https://planets.nu/</link>.'),
        paragraph_text('When invoked with the <b>-dir</b> option and an optional <i>GAMEDIR</i> argument,',
                       'or without any argument at all, it starts with a game chooser viewing that directory (or the current directory).'),
        paragraph_text('When invoked with <i>GAMEDIR</i> and <i>PLAYER</i> arguments,',
                       'it starts playing the specified player in that directory.'),
        paragraph_text('With just a <i>GAMEDIR</i> argument, <i>c2ng</i> starts playing if there is only one player,',
                       'otherwise it starts with the game selector.'));

section('Options',
        paragraph_default_help(';'),
        paragraph_detail(text_option('-dir'),
                         paragraph_text('Open directory browser;')),
        paragraph_default_log(';'),
        paragraph_detail(text_option('-password=', 'PASS'),
                         paragraph_text('Set result file password (NOT online password!);')),
        paragraph_default_proxy(';'),
        paragraph_detail(text_option('-resource=', 'NAME'),
                         paragraph_text('Add resource provider (e.g. <fn>*.res</fn> file);')),
        paragraph_detail(text_option('-fullscreen'),
                         paragraph_text('Run full-screen;')),
        paragraph_detail(text_option('-windowed'),
                         paragraph_text('Run in a window;')),
        paragraph_detail(text_option('-bpp', 'N'),
                         paragraph_text('Use color depth of N bits per pixel;')),
        paragraph_detail('<b>-size=</b><i>W</i>[<b>x</b><i>H</i>]',
                         paragraph_text('Set window size/screen resolution;')),
        paragraph_detail(text_option('-nomousegrab'),
                         paragraph_text("Don't grab (lock into window) mouse pointer.")));

section('Environment',
        paragraph_default_server_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
