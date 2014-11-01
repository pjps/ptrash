\" User nroff -ms <file-name> to format this file

.po 1.25i
.ll 6i
.pl 66

.nr PO 1.25i
.nr LL 6i
.nr PL 66

\" Page header
.ds LH
.ds CH
.ds RH

\" Page footer
.ds LF
.ds CF
.ds RF

\" No hyphenation
.hy 0
.nr HY 0

.DS C
Ptrash-1.0.1
.DE
.LP
Hello all,
.PP
I am pleased to release this version 1.0.1 of Ptrash. Ptrash is a small console
based utility program. Ptrash is intended to move file(s) to ~/.trash. Where,
trash is nothing but a '~/.trash' directory under home directory of a user.
Ptrash facilitates moving of all types of files to trash, as well as
restoring those files and/or directories to their original location.
.PP
I took upon writing Ptrash, when I deleted(using rm) few files and found my
self badly desperate to retrieve them back, but just could not do so.

.IP Installation:
.LP
Installing Ptrash is quite easy and straight forward, do 'cd' to the directory
(ptrash-1.0.1) containing all sources and follow these steps... 
.DS I
$ ./configure
$ make
# make install 
.DE
That's all, you have Ptrash installed. For more information about usage and
general options try...
.DS I
$ ptrash --help 
$ man ptrash
$ info ptrash
.DE
.IP Problems: 
.LP
Please send any bug reports/problems to <pj.pandit at yahoo.co.in>. While doing
so do include "ptrash-1.0.1" string in subject line and also describe precisely
the behaviour of ptrash. Sending actual command, which caused that behaviour
would be even more helpful while debugging.
.LP
Thank you!
