start('c2plugin', 'PCC2 Plugin Manager');

section('Synopsis',
        paragraph_synopsis('<b>c2plugin -h</b>'),
        paragraph_synopsis('<b>c2plugin ls</b> [<b>-l</b>|<b>-b</b>] [<b>-o</b>]'),
        paragraph_synopsis('<b>c2plugin add</b>|<b>install</b> [<b>-n</b>] [<b>-f</b>] <i>FILE.c2p</i>...'),
        paragraph_synopsis('<b>c2plugin remove</b>|<b>rm</b>|<b>uninstall</b> [<b>-n</b>] [<b>-f</b>] <i>ID</i>...'),
        paragraph_synopsis('<b>c2plugin test</b> [<b>-v</b>] <i>FILE.c2p</i>...'));

section('Description',
        paragraph_text('<i>c2plugin</i> manages PCC2 plugins.',
                       'PCC2 plugins can add additional functionality or artwork to PCC2.'));

section('Commands',
        paragraph_detail('<b>ls</b> (or <b>list</b>)',
                         paragraph_text('List installed plugins.',
                                        'The default output is human-readable;')),
        paragraph_detail('<b>add</b> (or <b>install</b>)',
                         paragraph_text('Install plugins.',
                                        'You can specify <fn>*.c2p</fn>, <fn>*.res</fn>, and <fn>*.q</fn> files on the command line which will be installed in order.',
                                        'If the specified file is not a <fn>*.c2p</fn> file, an appropriate <fn>*.c2p</fn> file is generated on-the-fly.'),
                         paragraph_text('You can also specify <fn>*.c2z</fn> or <fn>*.zip</fn> files.',
                                        'These are standard zip files, which must contain a <fn>*.c2p</fn> file and are unpacked on-the-fly;')),
        paragraph_detail('<b>rm</b> (or <b>remove</b>, <b>uninstall</b>)',
                         paragraph_text('Remove plugins.',
                                        'You can specify plugin identifiers on the command line which will be removed in order.',
                                        'The plugin identifier is the file name (without directory and extension) of the <fn>*.c2p</fn> file used for installing.')),
        paragraph_detail('<b>test</b>',
                         paragraph_text('Test plugins.',
                                        'This command is intended for plugin authors and verifies that the plugin files actually work.',
                                        'It does not check or modify the installed plugins.')));

section('Options',
        paragraph_default_help(),
        paragraph_detail(text_option('-l'),
                         paragraph_text('(ls) <i>Long</i> format. Lists more details for each plugin;')),
        paragraph_detail(text_option('-b'),
                         paragraph_text('(ls) <i>Brief</i> format. Lists just the plugin identifier for each plugin;')),
        paragraph_detail(text_option('-o'),
                         paragraph_text('(ls) List plugins in load <i>order</i> (default is alphabetical order.',
                                        'PCC2 loads plugins in dependency order;')),
        paragraph_detail(text_option('-n'),
                         paragraph_text('(add, rm) Do <i>not</i> change anything, just check whether it would succeed;')),
        paragraph_detail(text_option('-f'),
                         paragraph_text('(add, rm) Ignore dependencies and <i>force</i> installation/removal.',
                                        'Normally, <i>c2plugin</i> will not install plugins if some of their preconditions are missing,',
                                        'and refuse to uninstall plugins that are still required;')),
        paragraph_detail(text_option('-v'),
                         paragraph_text('(test) <i>Verbose</i> output.',
                                        'By default, only errors are logged. With <b>-v</b>, also successes are logged.')));

section('Dependencies',
        paragraph_text('PCC2 plugins support dependencies between plugins ("this plugin requires that one to be installed",',
                       '"this plugin requires that version of PCC2").',
                       'Normally, <i>c2plugin</i> enforces these dependencies.'),
        paragraph_text("<i>c2plugin</i>'s dependency manager is not very sophisticated and may sometimes get in your way.",
                       'If you know what you are doing, you can override it using <b>-f</b>.'),
        paragraph_text('PCC2 will not load plugins with missing dependencies.',
                       '<i>c2plugin ls -o</i> will tell you which plugins cannot be loaded.'));

section('Plugin Format');

subsect('Installed Format',
        paragraph_text('When installed, a plugin consists of a control file (<fn>*.c2p</fn>) in the <fn>plugins</fn> directory in your user profile.',
                       'The file name defines the name of the plugin.',
                       "The file contains a description of the plugin and names of all the plugin's files."),
        paragraph_text("The plugin's files will be stored in a subdirectory named after the plugin."));

subsect('Uninstalled Format',
        paragraph_text('If you use the <i>add</i> command to install a <fn>*.c2p</fn> file, <i>c2plugin</i> expects the plugin files in the same directory as the <fn>*.c2p</fn> file.',
                       'It will copy them all into your user profile.'),
        paragraph_text('If you install a <fn>*.c2z</fn> or <fn>*.zip</fn> file, that one must contain a <fn>*.c2p</fn> file and all other files.'),
        paragraph_text('If you install a <fn>*.q</fn> or <fn>*.res</fn> file, <i>c2plugin</i> will automatically generate a simple <fn>*.c2p</fn> file for you.'));

section('Limitations',
        paragraph_text('Plugins are always installed in your user profile.',
                       'PCC2 does not support installation of system-wide plugins.'));

section('Graphical Version',
        paragraph_text('The program <link>c2pluginw(6)</link> accepts a list of file names on the command line, and installs these as if the command <b>add</b> were used.',
                       'Output is shown in dialog boxes; options are not accepted.'),
        paragraph_text('<b>c2pluginw</b> is intended to be associated with <fn>*.c2p</fn>/<fn>*.c2z</fn> files in your graphical file manager,',
                       'so plugins can be installed by double-clicking them.',
                       'In Windows, the PCC2 installer sets this up by default.'));

section('Environment',
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
