/*
 * ptrash.h -- move unwanted files to trash; This file is part of the
 * program 'ptrash'
 * Copyright (C) 2008 2009 Prasad J Pandit
 *
 * 'ptrash' is a free software; you can redistribute it and/or modify it under
 * the terms of GNU General Public Licence as published by Free Software
 * Foundation; either version 2 of the licence, or (at your option) any later
 * version.
 *
 * 'ptrash' is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public Licence for more
 * details.
 *
 * You should have received a copy of the GNU General Public Licence along
 * with 'ptrash'; if not, write to Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */
#ifndef MOVE_H
#define MOVE_H

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>          /* for errno */
#include <sys/stat.h>       /* for mkdir */
#include <sys/types.h>
#include <fcntl.h>          /* for O_CREAT, O_RDONLY, etc */
#include <unistd.h>         /* for read write, etc */
#include <getopt.h>         /* for getopt_long, etc */
#include <dirent.h>         /* for struct DIR  */
#include <pwd.h>            /* for struct passwd */

#ifndef __GLIBC__
    #include <libgen.h>     /* for basename */
#endif

#define BUFSZ           100
#define VERSION         "1.0.1"


/* displays the help information for move */
extern void printh (void);

/* checks command line option and takes appropriate action
 * returns index of the first non-option command line argument or -1 on error */
extern int check_option (int, char *[]);

/* create a directory at specified path
 * returns 1 when successful or -1 on error */
extern int create_dir (const char *);

/* build an absolute path of the file and returns a pointer to path string or
 * NULL in case of error */
extern char * build_path (const char *, const char *);

/* to build a destination path for a file to be moved, returns an absolute
 * path string or NULL in case of error    */
extern char * dst_path (const char *);

/* open a file named by first argument and return an absolute path of this file
 * in location pointed to by src also returns file descriptor of the opened
 * file or -1 in case of error */
extern int open_src_file (char *);

/* open a file named by first argument and return a file descriptor of the
 * opened file or -1 on error */
extern int open_dst_file (char *);

/* prompt user to enter choice [y/n] and return 1 for choice 'y' and 0
 * otherwise */
extern short get_choice (char *, const char *);

/* aad/remove an entry of the file(last moved) to/from move database under trash
 * directory, returns a -1 on error or 1 on success */
extern int update_mdb (char *);

/* copy source file to destination file returns -1 on error or +1 when
 * successful */
extern int copy_file (int, int);

/* initialize move operation returns -1 on error or 1 when successful */
extern int init_move (void);

/* checks the file type and calls the apropriate move_<type> function
 * to do the job */
extern int move (char *, struct stat *);

/* function that actually moves regular file from source to .trash */
extern int move_reg (char *);

/* function to move the fifo special file to .trash */
extern int move_fifo (char *);

/* function that moves directories from source to .trash */
extern int move_dir (char *);

/* function to move character or block special files to .trash */
extern int move_nod (char *);

/* function to delete file from the .trash directory */
extern int delete (char *, struct stat *);

/* function to delete directory from .trash */
int delete_dir (char *);

#endif
