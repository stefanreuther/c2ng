start('c2console', 'PCC2 Console');

section('Synopsis',
        paragraph_default_help(';'),
        paragraph_synopsis('<b>c2console</b> [<b>--config=</b><i>FILE</i>] [<b>-D</b><i>KEY</i><b>=</b><i>VALUE</i>] [<i>ENV</i>=<i>VALUE</i>] [<i>COMMAND</i>...]'));

section('Description',
        paragraph_text('<i>c2console</i> provides a command-line interface to the microservices of the PlanetsCentral suite.'),
        paragraph_text('If a <i>COMMAND</i> is given, <i>c2console</i> executes that command and exits.',
                       'Otherwise, it will read and execute commands from standard input.'));

section('Options',
        paragraph_default_help(';'),
        paragraph_default_server_config(';'),
        paragraph_default_log(';'),
        paragraph_default_proxy($sep),
        paragraph_detail('<i>ENV</i><b>=</b><i>VALUE</i>',
                         paragraph_text('Set environment variable for script;')),
        paragraph_detail('<i>COMMAND...</i>',
                         paragraph_text('Command to execute (including parameters).')));

section('Environment',
        paragraph_default_server_environment(';'),
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
