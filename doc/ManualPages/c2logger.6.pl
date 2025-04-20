start('c2logger', 'PlanetsCentral Server Supervisor');

section('Synopsis',
        paragraph_synopsis('<b>c2logger</b> [<b>-</b><i>OPTS</i>] <i>COMMAND</i> [<i>ARGS</i>...]'));

section('Description',
        paragraph_text('<i>c2logger</i> runs another program (a server, typically), and logs its stdout/stderr into a file.',
                       'It can also terminate and/or restart the program.'));

section('Options',
        paragraph_detail(text_option('--help'),
                         paragraph_text('Show <i>help</i> summary;')),
        paragraph_detail(text_option('-log=', 'LOGFILE'),
                         paragraph_text('Set name of log file.',
                                        'The default is the name of the program, without path, with ".log" appended;')),
        paragraph_detail(text_option('--pid=', 'PIDFILE'),
                         paragraph_text('Set name of pidfile.',
                                        'The pidfile is required to support <b>--restart</b> and <b>--kill</b>.',
                                        'The default is no pid file.',
                                        'To avoid problems with stale pidfiles, <i>c2logger</i> checks /proc/uptime first to see',
                                        'whether the log file is from this life-cycle of the machine;')),
        paragraph_detail(text_option('--cd=', 'DIR'),
                         paragraph_text('Change to directory <i>DIR</i> before invoking the program;')),
        paragraph_detail(text_option('--limit=', 'BYTES'),
                         paragraph_text('Set maximum size of log file.',
                                        'The default is 10 MiB.',
                                        'When that size has been reached, the log file will be renamed with the current date in YYYYMMDD format as suffix,',
                                        'and a new log file begins.',
                                        'Set to 0 to disable log rotation;')),
        paragraph_detail(text_option('--uid=', 'USERNAME'),
                         paragraph_text('Invoke program under different user Id.',
                                        "The controlled program will run with that other user's privileges.",
                                        '<i>c2logger</i> must be invoked with root privileges when you use this option.',
                                        'It will keep its root privileges during its lifetime,',
                                        'so the logfile can be outside the file system tree accessible to the controlled program;')),
        paragraph_detail(text_option('--listen=', 'HOST:PORT'),
                         paragraph_text('Create a listen socket on the given host/port.',
                                        'The controlled program will see the file descriptor of that socket in an environment variable <b>C2SOCKET</b>.',
                                        'If that variable is present, it need not create and bind the socket itself.',
                                        'When <i>c2logger</i> runs with root privileges,',
                                        'this allows it to bind privileged sockets wile still running the controlled program with user privileges;')),
        paragraph_detail(text_option('--restart'),
                         paragraph_text('Terminate a possible previous instance, before starting a new one.',
                                        'The previous instance of the controlled program, if any, will be sent a SIGTERM,',
                                        'causing it and its <i>c2logger</i> to exit.'),
                         paragraph_text('Alternatively, you can send the controlled program a SIGUSR1 to have its <i>c2logger</i> restart it;')),
        paragraph_detail(text_option('--kill'),
                         paragraph_text("Terminate a possible previous instance, and don't start a new one.",
                                        'The previous instance of the controlled program, if any, will be sent a SIGTERM,',
                                        'causing it and its <i>c2logger</i> to exit;')),
        paragraph_detail(text_option('--fg'),
                         paragraph_text('Stay in foreground.',
                                        'By default, <i>c2logger</i> backgrounds itself.')));

section('Environment',
        paragraph_text('Although <i>c2logger</i> does not need it, the programs it controls usually need a',
                       '<b>C2CONFIG</b> environment variable pointing to the configuration file.'));

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2 (6)'));
