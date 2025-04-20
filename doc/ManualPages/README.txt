Documentation / Manual Pages for PCC2
=====================================

We want to render manual pages in different formats other than groff,
in particular, as XML for the help system/documentation center.

For simplicity, instead of coming up with or finding a markup format
that allows such conversions and also supports macros, we use a
sequence of Perl function calls as our markup.

- Generate.pl contains minimal boilerplate and the macros;

- OutputXXX.pl contains the conversion to the target format;

- XXX.6.pl contains the source code for the specific manpage.


Markup concepts
---------------

### Label

A label is a plain piece of text, e.g. the name of a utility (the
idea is: no need to worry about escaping).


### Text

A piece of text contains simple XML markup:

- <b>, <i> for boldface, italic;
- <cmd> for script commands;
- <fn> for file names;
- <link> for links; link can point at a manpage (<link>ls(1)</link>)
  or web address;
- escapes using &lt; &gt; &trade; &quot; &reg; &amp;

Tags do not nest.

Functions called text_xxx return text.


### Paragraph

A paragraph is a complete paragraph or list there-of, passed around as
finally formatted/tagged text.

Functions called paragraph_xxx return paragraphs.


### Section

Section output functions (section(), subsect()) immediately produce
output and do not return text.

(They need to return 1, such that perl treats the invocation of the
manpage script as a success.)
