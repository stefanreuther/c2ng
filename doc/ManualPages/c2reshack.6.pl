start('c2reshack', 'PCC2 Resource Editor');

section('Synopsis',
        paragraph_synopsis('<b>c2reshack</b> [<b>-</b><i>OPTIONS</i>] [<i>FILE</i>...]'));

section('Description',
        paragraph_text('<i>c2reshack</i> is a graphical resource editor for <i>c2ng</i>.',
                       'It can edit graphics and fonts.',
                       'For a detailed description of the user interface, refer to <fn>ui/reshack/README.txt</fn> that is part of the source code.'));

section('Options',
        paragraph_default_help(';'),
        paragraph_default_log(';'),
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
