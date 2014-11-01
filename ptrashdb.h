/*
 * ptrashdb.h -- move unwanted files to trash; This file is part of the
 * program 'ptrash'
 * Copyright (C) 2008 2009 Prasad J Pandit
 *
 * 'ptrash' is a free software; you can redistribute it and/or modify it under
 * the terms of GNU General Public Licence as published by Free Software
 * Foundation; either version 2 of the licence, or (at your option) any later
 * version.
 *
 * 'ptrash' is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY of FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public Licence for more
 * details.
 *
 * You should have received a copy of the GNU General Public Licence along
 * with 'ptrash'; if not, write to Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA 02110-1301 USA
 */
#ifndef MOVEDB_H
#define MOVEDB_H

#include <ptrash.h>

typedef struct node
{
    char *path;
    struct node *next;
} node;

typedef struct
{
    node *head;
    node *tail;
} trashdb;

/* create and return a new node to insert it into trashdb */
extern node * get_node (const char *);

/* insert a new node into trashdb returns +1 on success and -1 on error */
extern int t_insert (const char *);

/* delete node from trashdb, containing string supplied as an argument */
extern void t_delete (const char *);

/* modify node with path supplied as argument */
extern void t_modify (node *, const char *);

/* search a node matching the basename string of the supplied path and returns
 * a copy of the path string */
extern char * t_search (const char *);

/* search a node matching the basename string of the supplied path and returns
 * the node to calling function */
extern node * t_search_node (const char *);

/* display trashdb */
extern void t_display (void);

/* initialize and build trashdb */
extern void t_read (void);

/* write trashdb to file pointed to by tdb */
extern int t_write (void);

#endif
