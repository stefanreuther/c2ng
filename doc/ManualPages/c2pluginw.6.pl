start('c2pluginw', 'PCC Plugin Installer');

section('Synopsis',
        paragraph_synopsis('<b>c2pluginw</b> <i>FILE</i>'));

section('Description',
        paragraph_text('<i>c2pluginw</i> is a simple graphical plugin installer for PCC2.',
                       'When invoked with a plugin file name on the command line, ',
                       'it will guide the user through plugin installation with simple dialogs',
                       '(<link>kdialog(1)</link>, <link>gxmesssage(1)</link>, <link>xmessage(1)</link>).'),
        paragraph_text('This program is intended to be associated with <fn>.c2p</fn> and <fn>.c2z</fn> files in a graphical file manager,',
                       'such that launching the plugin will install it.',
                       'For more detailed control in scripted installation, use <link>c2plugin(6)</link>.'));

section('Environment',
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
