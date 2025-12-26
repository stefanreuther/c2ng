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
        paragraph_detail(text_option('--disassemble', '', '-S', ''), paragraph_text('Compile the given scripts into assembler code (<fn>*.qs</fn> files);')),
        paragraph_detail(text_option('--size'),                      paragraph_text('Show size of <fn>*.qc</fn> files;')),
        paragraph_detail(text_option('--strip'),                     paragraph_text('Remove line number information from <fn>*.qc</fn>.')));

subsect('Options',
        paragraph_detail(text_option('-g'),
                         paragraph_text('(compile) Enable debug information (default);')),
        paragraph_detail(text_option('-s'),
                         paragraph_text('(compile) Disable debug information;')),
        paragraph_detail(text_option('-o', 'FILE'),
                         paragraph_text('(compile) Save output in the given file.',
                                        'If this option is not specified, and input is one or more files, output is produced for each individual input file in a separated <fn>*.qc</fn> file.',
                                        'If this option is specified, and input is multiple files, all input files are linked together to a single <fn>*.qc</fn> file.'),
                         paragraph_text('(disassemble) Save output in the given file.',
                                        'Disassemble mode always links all input files linked together.',
                                        'If this option is specified, output is produced in the specified file.',
                                        'Otherwise, output is produced on stdout.'),
                         paragraph_text('(strip) Save result in the given file.',
                                        'This option can only be used if exactly one input file is specified.',
                                        'If this option is not given, input files are replaced by the stripped version;')),
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
        paragraph_text('For Compile and Disassemble mode, <i>c2script</i> can take commands from files (default) or the command line (<b>-k</b>).',
                       'It will first convert these into their internal form (bytecode) and then perform the requested operation,',
                       'independently from the source form.'),
        paragraph_text('Input files for can be script files in text form (<fn>*.q</fn>), object files (<fn>*.qc</fn>), or assembly-language files (<fn>*.qs</fn>).'),
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
                       'but not in a textual assembly-language file.'));

subsect('Size Mode',
        paragraph_text('In size mode (<b>--size</b>), takes a number of object files (<fn>*.qc</fn>) as input,',
                       "and prints a summary of each file's content, broken down into size of code and literals for each bytecode object."));

subsect('Strip Mode',
        paragraph_text('In strip mode (<b>--strip</b>), removes line number information from the files, reducing their size.',
                       '(Incidentally, this removes slightly more data than the compiler option <b>-s</b> omits.)'),
        paragraph_text('By default, each file is replaced by its stripped version.',
                       'If only a single file is requested, you can specify an output file name using <b>-o</b>.'));

subsect('Assembly Language',
        paragraph_text('The "assembly language" syntax is used as output for Disassemble Mode,',
                       'or as input from <b>*.qs</b> files.',
                       'This syntax allows representation of a few more details of the compiled code.',
                       'The assembler intentionally only performs fewer consistency checks, to allow creation of code that exercises corner cases.',
                       'This means it is also possible to create code that will eventually be refused by PCC2.'),
        paragraph_text('Assembly language files can contain one instruction per line.',
                       'Comments can be started with ";" or "%" and extend to the end of the line.'),
        paragraph_text('Instructions are:'),
        paragraph_detail('<b>Declare</b> {<b>Sub</b>|<b>Function</b>|<b>Struct</b>} <i>NAME</i>',
                         paragraph_text('Declare an item without defining it yet;')),
        paragraph_detail('{<b>Sub</b>|<b>Function</b>} <i>NAME</i><b>(</b><i>ARG</i><b>,</b> [<b>Optional</b> <i>ARG</i>...] [<i>ARG</i><b>()</b>]<b>)</b>',
                         paragraph_text('Define a subroutine.',
                                        'The subroutine code follows on the following lines (see below), terminated with <b>EndSub</b> resp. <b>EndFunction</b>;')),
        paragraph_detail('<b>Struct</b> <i>NAME</i>',
                         paragraph_text('Define a structure type.',
                                        'The field names are given on subsequent lines using syntax "<b>.field</b> <i>NAME</i>[<b>,</b> <i>NAME</i>...]".')),
        paragraph_detail('<b>.jumps</b> {<b>symbolic</b>|<b>absolute</b>}',
                         paragraph_text('Set mode for label/jump handling in following subroutines.',
                                        'In absolute mode (default), a jump instruction refers to the target instruction directly.',
                                        'In symbolic mode, a jump targets a matching <i>label</i> instruction;',
                                        'this mode is normally used internally because it simplifies code transformations.')),

        paragraph_text('In addition to normal bytecode instructions, subroutines can contain the following:'),
        paragraph_detail('<i>IDENTIFIER</i><b>:</b>',
                         paragraph_text('Define a label;')),
        paragraph_detail('<b>.defsubs</b>',
                         paragraph_text('Generate code to invoke <b>sdefsub</b> instruction for all uniquely-named subroutines defined so far;')),
        paragraph_detail('<b>.file</b> <i>NAME</i>',
                         paragraph_text('Set the input file name (use double-quotes to preserve spelling);')),
        paragraph_detail('<b>.flags</b> <i>NUM</i>',
                         paragraph_text('Set flags (1=is procedure (sub), 2=is varargs);')),
        paragraph_detail('<b>.line</b> <i>NUM</i>[<b>,</b> <i>NUM</i>]',
                         paragraph_text('Add line-number information;')),
        paragraph_detail('<b>.lit</b> <i>LITERAL</i>',
                         paragraph_text('Define a value in the literal pool.')),
        paragraph_detail('<b>.local</b> <i>NAME</i>',
                         paragraph_text('Predefine a local variable;')),
        paragraph_detail('<b>.max_args</b> <i>NUM</i>',
                         paragraph_text('Set maximum number of parameters;')),
        paragraph_detail('<b>.min_args</b> <i>NUM</i>',
                         paragraph_text('Set minimum number of parameters;')),
        paragraph_detail('<b>.name</b> <i>NAME</i>',
                         paragraph_text("Set the subroutine's name hint;")),
        paragraph_detail('<b>.num_labels</b> <i>NUM</i>',
                         paragraph_text('Set number of labels;')),
        paragraph_detail('<b>.sym</b> <i>NAME</i>',
                         paragraph_text('Add a symbol to the symbol pool;')),
        paragraph_detail('<b>.varargs</b>',
                         paragraph_text('Mark this subroutine as taking a variable number of parameters.')),
        paragraph_text('A <i>NUM</i> parameter can be a decimal number, optionally preceded by + or -.',
                       'A <i>NAME</i> parameter can be a single identifier (will be converted to upper-case),',
                       'a single dash for an empty name, or text in quotes.',
                       'A <i>LITERAL</i> can be the same as in PCC2 script (integer or floating-point number, string in single or double quotes, <b>True</b>, <b>False</b>), <b>Null</b>,',
                       'the name of a subroutine/structure type, or <b>(</b><i>NUM</i><b>,</b><i>NUM</i><b>)</b> for a specific serialized form.')
    );

section('Environment', paragraph_default_environment());

section('Author', paragraph_default_author());

section('See Also', paragraph_link_list('pcc2(6)'));
