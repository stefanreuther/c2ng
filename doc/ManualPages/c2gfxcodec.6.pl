start('c2gfxcodec', 'PCC2 Graphics Codec Application');

section('Synopsis',
        paragraph_synopsis('<b>c2gfxcodec -h</b>'),
        paragraph_synopsis('<b>c2gfxcodec</b> <i>COMMAND</i> [<b>-</b><i>OPTIONS</i>]'));

section('Description',
        paragraph_text("<i>c2gfxcodec</i> converts images to and from PCC's custom format."),
        paragraph_text("PCC 1.x and PCC2 use a custom format with 8 and 4 bits per pixel,",
                       "conventionally having the file extension <i>.cd</i> and <i>.cc</i>.",
                       "The primary benefit of that codec is (was) use of PCC 1.x's fixed color palette.",
                       "The files can optionally be compressed using a simple run-length encoding scheme."),
        paragraph_text("Multiple files of this type can be stored in resource files (<i>*.res</i>).",
                       "Resource files can be installed as plugins (see <link>c2plugin(6)</link>),",
                       "or loaded using the <cmd>LoadResource</cmd> command."));

section('Commands',
        paragraph_detail('<b>convert</b> <i>INFILE</i> <i>OUTFILE</i>',
                         paragraph_text('Convert file from one format to another;')),
        paragraph_detail('<b>create</b> <i>FILE.res</i> <i>IN</i><b>=</b><i>INFILE</i>...',
                         paragraph_text("Create a resource file from multiple input files.",
                                        "Input files are internally converted to both a compressed 4-bit and 8-bit file",
                                        "as per PCC's expectation;")),
        paragraph_detail('<b>gallery</b> <i>FILE.res</i>',
                         paragraph_text('Create a gallery to view the content of the given resource file.',
                                        'This will extract all pictures into the current directory, along with an <fn>index.html</fn> file.')));

subsect('File Specifications',
        paragraph_detail('<b>bmp:</b><i>PATH.bmp</i>',
                         paragraph_text('Standard bitmap file;')),
        paragraph_detail('<b>custom:</b><i>PATH.cd</i>',
                         paragraph_text('On input, accepts any custom codec; on output, same as <b>plain8:</b>;')),
        paragraph_detail('<b>plain8:</b><i>PATH.cd</i>',
                         paragraph_text('Plain 8-bit custom codec;')),
        paragraph_detail('<b>plain4:</b><i>PATH.cd</i>',
                         paragraph_text('Plain 4-bit custom codec;')),
        paragraph_detail('<b>packed4:</b><i>PATH.cd</i>',
                         paragraph_text('Packed (compressed) 4-bit custom codec;')),
        paragraph_detail('<b>packed8:</b><i>PATH.cd</i>',
                         paragraph_text('Packed (compressed) 8-bit custom codec.')));

section('Environment',
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)', 'c2restool(6)'));
