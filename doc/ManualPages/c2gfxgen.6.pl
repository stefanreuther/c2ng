start('c2gfxgen', 'PCC2 Procedural Graphics Generator');

section('Synopsis',
        paragraph_synopsis('<b>c2gfxgen -h</b>'),
        paragraph_synopsis('<b>c2gfxgen</b> <i>COMMAND</i> [<i>OPTIONS</i>...]'));

section('Description',
        paragraph_text("<i>c2gfxgen</i> is a front-end for PCC2's procedural generation algorithms."));

section('Commands');

subsect('Common Options',
        paragraph_text('The following options are accepted for every command.'),
        paragraph_detail(text_option('-w', 'WIDTH'),    paragraph_text('Set width of output file;')),
        paragraph_detail(text_option('-h', 'HEIGHT'),   paragraph_text('Set height of output file;')),
        paragraph_detail(text_option('-S', 'SEED'),     paragraph_text('Set seed for random number generator;')),
        paragraph_detail(text_option('-o', 'FILE.bmp'), paragraph_text('Set output file name (mandatory).')));

subsect('Command "space"',
        paragraph_text('This command generates a space view (starfield, nebula).'),
        paragraph_detail(text_option('-s', 'SUNS'),     paragraph_text('Set number of suns (large stars);')),
        paragraph_detail(text_option('-p', 'PROB'),     paragraph_text('Set probability medium size stars (percentage).')));

subsect('Command "planet"',
        paragraph_text('This command generates a single planet.'),
        paragraph_detail(text_option('-x', 'PERCENT'),  paragraph_text('Set planet X position (percentage of width);')),
        paragraph_detail(text_option('-y', 'PERCENT'),  paragraph_text('Set planet Y position (percentage of height);')),
        paragraph_detail(text_option('-r', 'PERCENT'),  paragraph_text('Set planet radius (percentage);')),
        paragraph_detail(text_option('-t', 'TEMP'),     paragraph_text('Set planet temperature (0..100);')),
        paragraph_detail(text_option('-X', 'PERCENT'),  paragraph_text('Set sun X position (for shadow);')),
        paragraph_detail(text_option('-Y', 'PERCENT'),  paragraph_text('Set sun Y position (for shadow);')),
        paragraph_detail(text_option('-Z', 'PERCENT'),  paragraph_text('Set sun Z position (for shadow).')));

subsect('Command "orbit"',
        paragraph_text('This command generates a space view with a planet in foreground, as if viewed from orbit.'),
        paragraph_detail(text_option('-x', 'PERCENT'),  paragraph_text('Set planet X position (percentage of width);')),
        paragraph_detail(text_option('-y', 'PERCENT'),  paragraph_text('Set planet Y position (percentage of height);')),
        paragraph_detail(text_option('-r', 'PERCENT'),  paragraph_text('Set planet radius (percentage);')),
        paragraph_detail(text_option('-n', 'NUM'),      paragraph_text('Set number of stars.'))),

subsect('Command "explosion"',
        paragraph_text('This command generates a generic explosion particle effect.',
                       'The result is a single image containing all explosion frames on a vertical strip.'),
        paragraph_detail(text_option('-n', 'NUM'),      paragraph_text('Set size of explosion (number of hotspots);')),
        paragraph_detail(text_option('-v', 'SPEED'),    paragraph_text('Set speed (integer; default 1).',
                                                                       'Increasing the speed will skip frames.')));

subsect('Command "shield"',
        paragraph_text('This command generates a shield particle effect.',
                       'The result is a single image containing all frames on a vertical strip.'),
        paragraph_detail(text_option('-n', 'NUM'),      paragraph_text('Set size (number of hotspots);')),
        paragraph_detail(text_option('-a', 'ANGLE'),    paragraph_text('Set angle (0-7).')));

subsect('Command "texture"',
        paragraph_text('This command generates different procedural textures.'),
        paragraph_detail('<b>fill(</b><i>COLOR</i><b>)</b>',
                         paragraph_text('Fill canvas with solid color.',
                                        'The color can have form "#rgb", "#rrggbb", "#rgba", "#rrggbbaa", "rgb(r,g,b)", or "rgb(r,g,b,a)";')),
        paragraph_detail('<b>noise(</b><i>RANGE</i><b>)</b>',
                         paragraph_text('Fill canvas with random noise, with colors taken from the given range.',
                                        'The range can have the form "COLOR-COLOR" or "COLOR-COLOR/STEPS".',
                                        'If the STEPS parameter is given, only this many discrete color steps are used,',
                                        'even if interpolation between colors could produce more steps;')),
        paragraph_detail('<b>brush(</b><i>RANGE</i>[<b>,angle=</b><i>N</i>][<b>,n=</b><i>N</i>]<b>)</b>',
                         paragraph_text('Add a brushed-metal effect;')),
        paragraph_detail('<b>circ(</b><i>RANGE</i><b>,</b><i>X</i><b>,</b><i>Y</i><b>,</b><i>R</i>[<b>,</b><i>NOISE</i>]<b>)</b>',
                         paragraph_text('Add a circular gradient effect.')));

section('Environment',
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
