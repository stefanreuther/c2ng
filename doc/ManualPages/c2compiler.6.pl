start('c2compiler', 'PCC2 Script Compiler');

section('Synopsis',
        paragraph_synopsis('<b>c2compiler -h</b>'),
        paragraph_synopsis('<b>c2compiler</b> [<b>-</b><i>ACTION</i>] [<i>OPTIONS</i>] <i>FILE</i>...'),
        paragraph_synopsis('<b>c2compiler</b> [<b>-</b><i>ACTION</i>] [<i>OPTIONS</i>] -k <i>COMMAND</i>...'));

section('Description',
        paragraph_text('<i>c2compiler</i> is a standalone version of the PCC2 script compiler.',
                       'It can compiler scripts to the internal bytecode, and deal with the resulting files.'),
        paragraph_text('This tool is mostly aimed at developers.',
                       'To execute scripts, use <link>c2script(6)</link>.'));
section('Options');
subsect('Actions',
        paragraph_default_help(';'),
        paragraph_detail(text_option('--compile', '', '-c', ''),     paragraph_text('<i>Compile</i> the given scripts into object code (<fn>*.qc</fn> files, default);')),
        paragraph_detail(text_option('--disassemble', '', '-S', ''), paragraph_text('Compile the given scripts into assembler code (<fn>*.qs</fn> files).')));

subsect('Options',
        paragraph_detail(text_option('-g'),
                         paragraph_text('(compile) Enable debug information (default);')),
        paragraph_detail(text_option('-s'),
                         paragraph_text('(compile) Disable debug information;')),
        paragraph_detail(text_option('-o', 'FILE'),
                         paragraph_text('(compile) Save output in the given file.',
                                        '#If this option is not specified, and input is one or more files, output is produced for each individual input file in a separated <fn>*.qc</fn> file.',
                                        'If this option is specified, and input is multiple files, all input files are linked together to a single <fn>*.qc</fn> file.'),
                         paragraph_text('(disassemble) Save output in the given file.',
                                        'Disassemble mode always links all input files linked together.',
                                        'If this option is specified, output is produced in the specified file.',
                                        'Otherwise, output is produced on stdout;')),
        paragraph_detail(text_option('-I', 'PATH'),
                         paragraph_text('Add load path.',
                                        'The <cmd>Load</cmd> and <cmd>TryLoad</cmd> statements are resolved against this path.',
                                        'This option can be specified multiple times to look in multiple directories;')),
        paragraph_detail(text_option('--charset', 'CS').', '.text_option('-C', 'CS'),
                         paragraph_text('Set character set;')),
        paragraph_default_optimizer(';'),
        paragraph_default_command(';'),
        paragraph_default_log(';'),
        paragraph_detail(text_option('-q'),
                         paragraph_text('Configure log output to show script output only, no status messages;')),
        paragraph_detail(text_option('-fpreexec-load'),
                         paragraph_text('Try to process <cmd>Load</cmd>/<cmd>TryLoad</cmd> statements during compilation.',
                                        'Normally, those statements are resolved at runtime by invoking the compiler again when the statement is executed.',
                                        'With <b>-fpreexec-load</b>, loads with a literal file name as parameter will be compiled immediately.')));

section('Mode Details',
        paragraph_text('<i>c2script</i> can take commands from files (default) or the command line (<b>-k</b>).',
                       'It will first convert these into their internal form (bytecode) and then perform the requested operation,',
                       'independently from the source form.'),
        paragraph_text('Input files can be script files in text form (<fn>*.q</fn>) or object files (<fn>*.qc</fn>).'),
        paragraph_text('A command line consisting of multiple commands will be interpreted as a single multi-line script.'));

subsect('Compile Mode',
        paragraph_text('In compile mode (<b>--compile</b>, <b>-c</b>), the bytecode is written to object files.',
                       'You can either produce one object file for each input file, or one containing them all (<b>-o</b>).'),
        paragraph_text('If input to compile mode is object files, <i>c2script</i> will just write out equivalent (though not neccessarily bit-identical) files again.'));

subsect('Disassemble Mode',
        paragraph_text('In disassemble mode (<b>--disassemble</b>, <b>-S</b>), the bytecode is output in textual form.',
                       'Note that unlike a "classic" compiler, <i>c2script</i> does not generate assembler code during compilation.',
                       'Instead, the output you receive from disassembler mode is an attempt to reproduce the internal bytecode form in a human-readable way.',
                       'This means input to disassemble mode can be object files as easily as textual scripts.'),
        paragraph_text('However, there are obscure script constructs that can be represented in the internal form and in object files,',
                       'but not in a textual assembler file.'));

section('Environment', paragraph_default_environment());

section('Author', paragraph_default_author());

section('See Also', paragraph_link_list('pcc2(6)'));
