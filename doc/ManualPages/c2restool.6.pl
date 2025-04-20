start('c2restool', 'PCC Resource File Manager');

section('Synopsis',
        paragraph_synopsis('<b>c2restool -h</b>'),
        paragraph_synopsis('<b>c2restool</b> <i>COMMAND...</i>'));

section('Description',
        paragraph_text('<i>c2restool</i> is a tool to build and examine PCC resource files (<fn>*.res</fn>).',
                       'A resource file contains a set of files, each identified by a 16-bit number.'));

section('Commands',
        paragraph_detail('<b>create</b> [<b>--crfl</b>] [<b>--list=</b><i>FILE</i>] [<b>-I</b> <i>DIR</i>] <i>FILE.res</i> <i>FILE.rc</i>...',
                         paragraph_text('Create a resource file from a script;')),
        paragraph_detail('<b>ls</b> <i>FILE.res</i>...',
                         paragraph_text('List content of resource file.',
                                        'Writes a list of file Ids and sizes to standard output.',
                                        'Alternate spelling: <b>list</b>;')),
        paragraph_detail('<b>extract</b> <i>FILE.res</i> <i>INDEX</i> <i>FILE.out</i>',
                         paragraph_text('Extract a single file.',
                                        'Alternate spelling: <b>rx</b>;')),
        paragraph_detail('<b>extract-all</b> <i>FILE.res</i> [<i>FILE.rc</i>]',
                         paragraph_text('Extract all files.',
                                        'If a <fn>.rc</fn> file parameter has been specified, also creates a script that can be used to recreate the file using <b>create</b>.',
                                        'Alternate spelling: <b>rxall</b>.')));

subsect('Command Options',
        paragraph_detail(text_option('--crlf'),
                         paragraph_text('Use CR/LF linefeeds in embedded text (<i>.text</i> items);')),
        paragraph_detail(text_option('--list=', 'FILE'),
                         paragraph_text('Create a list file containing the list of all aliases used in the resource script;')),
        paragraph_detail(text_option('--dep=', 'FILE'),
                         paragraph_text('Create a dependency file (<i>.d</i>) for use with <link>make(1)</link>;')),
        paragraph_detail(text_option('--list-format=', 'FMT'),
                         paragraph_text('Defines the format of the list file. Default is "#define %s %d";')),
        paragraph_detail(text_option('-L', 'DIR'),
                         paragraph_text('Add directory to search path for files.')));

section('Resource Script Format',
        paragraph_text('Each entry starts with a line of the form'),
        paragraph_indent('<i>NUM</i>[<b>=</b><i>ALIAS</i>] <i>SOURCE</i>'),
        paragraph_text('<i>NUM</i> is the Id number of the entry, and can be a decimal number or the keyword <b>next</b>',
                       '(to use the last number, plus 1).'),
        paragraph_text('<i>SOURCE</i> can be:'),
        paragraph_detail('<b>*/</b><i>FILENAME</i>',
                         paragraph_text('insert content of the given file, searched in the paths given with <b>-L</b>;')),
        paragraph_detail('<i>FILENAME</i>',
                         paragraph_text('insert content of the given file. If the file name does not contain a directory name, it is searched in the paths given with <b>-L</b>;')),
        paragraph_detail('<b>.text</b>',
                         paragraph_text('insert text given on the following lines, terminated with keyword <b>.endtext</b> on a line of its own;')),
        paragraph_detail('<b>eq</b> <i>NUM</i>',
                         paragraph_text('create a hardlink to (=re-use the content of) another item.')),
        paragraph_text('Lines starting with ";" are ignored (comment).'));

section('Environment',
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)', 'c2gfxcodec(6)'));
