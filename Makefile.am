##
## Process this file with automake to produce 'Makefile.in'
## 
## To create the configure script using this file follow these steps
## in order
## 1. write Makefile.am    
## 2. autoscan
## 3. aclocal
## 4. autoheader
## 5. autoconf
## 6. automake -ac --foreign
##

AM_CFLAGS = -D_GNU_SOURCE -Wall

bin_PROGRAMS = ptrash
ptrash_SOURCES = ptrash.c ptrashdb.c ptrash.h ptrashdb.h

# man1_MANS = ptrash.1
info_TEXINFOS = ptrash.texi

dist_man_MANS = ptrash.1

EXTRA_DIST = $(man_MANS) README COPYING readme.ms
