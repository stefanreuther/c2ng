start('c2fileclient', 'PCC2 File Client');

section('Synopsis',
        paragraph_synopsis('<b>c2fileclient -h</b>'),
        paragraph_synopsis('<b>c2fileclient</b> [<b>--proxy=</b><i>URL</i>] <i>COMMAND</i>...'));

section('Description',
        paragraph_text('<i>c2fileclient</i> is a client for <link>c2file-server(6)</link> and the file-access part of <link>c2host-server(6)</link>.',
                       'It provides various operations to populate and inspect a file space.'));

section('Options',
        paragraph_text('These options must be specified before a command, if any.'),
        paragraph_default_help(';'),
        paragraph_default_proxy());

section('Commands',
        paragraph_detail('<b>cp</b> [<b>-r</b>] [<b>-x</b>] <i>SOURCE</i> <i>DEST</i>',
                         paragraph_text('Copy everything from <i>SOURCE</i> to <i>DEST</i>;')),
        paragraph_detail('<b>ls</b> [<b>-r</b>] [<b>-l</b>] <i>DIR</i>...',
                         paragraph_text('List content of <i>DIR</i>;')),
        paragraph_detail('<b>sync</b> <i>SOURCE</i> <i>DEST</i>',
                         paragraph_text('Make <i>DEST</i> contain the same content as <i>SOURCE</i>, by copying and deleting recursively;')),
        paragraph_detail('<b>clear</b> <i>DIR</i>...',
                         paragraph_text('Remove content of <i>DIR</i>;')),
        paragraph_detail('<b>serve</b> <i>SOURCE</i> <i>HOST</i><b>:</b><i>PORT</i>',
                         paragraph_text('Serve the content of <i>SOURCE</i> via HTTP.',
                                        'Point browser at "http://<i>HOST</i><b>:</b><i>PORT</i>/";')),
        paragraph_detail('<b>gc</b> [<b>-n</b>] [<b>-f</b>] <i>PATH</i>',
                         paragraph_text('Garbage-collect the CA filesystem contained in path <i>PATH</i>.')));

subsect('Command Options',
        paragraph_text('Command options need to specified after the command.'),
        paragraph_detail(text_option('-f'), paragraph_text('Force garbage-collection even on error;')),
        paragraph_detail(text_option('-l'), paragraph_text('Long format;')),
        paragraph_detail(text_option('-n'), paragraph_text('Dry run (do not delete anything);')),
        paragraph_detail(text_option('-r'), paragraph_text('Recursive;')),
        paragraph_detail(text_option('-x'), paragraph_text('Expand <fn>*.tgz</fn>/<fn>*.tar.gz</fn> files while copying.')));

subsect('File Specifications',
        paragraph_text('The following formats can be used for <i>SOURCE</i>, <i>DEST</i>, <i>DIR</i>, or <i>SPEC</i> parameters.'),
        paragraph_detail('<i>PATH</i>',
                         paragraph_text('Unmanaged, normal file system;')),
        paragraph_detail('[<i>PATH</i><b>@</b>]<b>ca:</b><i>SPEC</i>',
                         paragraph_text('Unmanaged, content-addressable file system;')),
        paragraph_detail('[<i>PATH</i><b>@</b>]<b>int:</b>[<i>UNIQ</i>]',
                         paragraph_text('Internal file system in RAM (not persistent).',
                                        'If the same <i>UNIQ</i> token appears in multiple parameters in the same command, it will refer to the same instance;')),
        paragraph_detail('<b>c2file://</b>[<i>USER</i><b>@</b>]<i>HOST</i><b>:</b><i>PORT</i><b>/</b><i>PATH</i>',
                         paragraph_text('Remote, managed file system.')));

section('Environment',
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)', 'c2file-server(6)', 'c2host-server(6)'));
