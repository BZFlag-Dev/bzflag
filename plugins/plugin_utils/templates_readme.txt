*Templates*

The Templateiser class in plugin_HTTPTemplates allows HTTPD based plug-ins to
use templates to generate HTML content for pages.

*syntax*
Template code uses special tags that are always sournded by square backets
"[ ]". The first character after the "[" tells the template engine what
type of tag it is. Acceptable Tag codes are;

[$NAME] for a single item variable.
[*CODE NAME] for a loop.
[?CODE NAME] for a logic test (if).
[!FILE] for an include.
[-TEXT] for a comment.

Tags that to not follow this format will be output as normal text.

*variables*
Tags that start with [$ are variables. These tags will be replaced with 
plug-in generated code. The text after the $ is the name of the variable
and is case insenstive.

Some standard variables are provided by the template system;

[$Date] returns the current date.
[$Time] returns the current time.
[$PageTime] returns the time since the page was started.
[$HostName] returns the domain name/ip and port of the current server.
[$BaseURL] returns the URL to the current plug-ins virtual dir.
[$PluginName] returns the name of the current plug-in.

*loops*
Tags that start with [* are loops. Each loop consists of 3 tags eac with its
own code;
[*Start NAME] starts the loop
[*End NAME] flags the end of the code for each loop item
[*Empty NAME] flags the end of the code for empty loops

Each loop must have these 3 sections in this order. If the loop has items then
The code between Start and End will be called for each item in the loop. If the
loop is empty, then the code between End and Empty will be called once.

Loops can be nested but must still follow this format and have a uniqne name.
There are no default loops.

*logic test (if)*
Tags that start with [? are logic tests and consist of at least 2 tags for
a basic test with an optional third tag.

[?IF NAME] starts an if test.
[?ELSE NAME] flags the start of the code used for cases where the if is false.
[?END NAME] ends an if test.

Each loop must have at least the IF and END flags, with IF before END. If
the ELSE case is used, it must be before the END, but after the IF.

If tests can be nested as long as the names are unique.

Some standard IF tests are provided by the template system;

[$IF Public] returns true if the server is public.

*Include*
Tags that start wtih [! are include tags. The file name after the ! should be
a valid file on the system, in one of the include paths. This file will be
processed and it's code will be inserted at the place where it's called.
On Operating Systems with case senstive file systems, the file name is
case senstive.

*Comment*
Tags that start with [- are comments and are ignored. Comments can not be
nested, but can contain newlines.


