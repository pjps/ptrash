/*
 * ptrashdb.c -- move unwanted files to trash; This file is part of the
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
#include <ptrash.h>
#include <ptrashdb.h>

char *tdb = NULL;
static trashdb db;
extern char *prog;

/*
 * get_node: returns a new node to insert into a trashdb or NULL
 *
 * p: a path string to store in a trashdb
 */
node *
get_node (const char *p)
{
    int len = 0;
    node *tmp = NULL;

    if (p)
    {
        len = strlen (p);
        if ((tmp = calloc (1, sizeof (node))) != NULL)
        {
            tmp->path = calloc ((len + 1), sizeof (char));
            strncpy (tmp->path, p, len);
            tmp->path[len] = '\0';
            tmp->next = NULL;
        }
    }    

    return tmp;
}


/*
 * t_insert: add a new node into trashdb or update an existing node in case the
 * basename(p) of path p is already present in trashdb.
 *
 * p: path string to be inserted.
 */
int
t_insert (const char *p)
{
    node *new_node = NULL;

    if ((p == NULL) || (!strlen (p)))
        return -1;

    if ((new_node = t_search_node (p)) != NULL)
    {
        t_modify (new_node, p);
        return 1;
    }
    new_node = get_node (p);
    if (db.head == NULL)
    {
        db.head = new_node;
        db.tail = db.head;
    }
    else
    {
        db.tail->next = new_node;
        db.tail = db.tail->next;
    }

    return 1;
}


/*
 * t_delete: delete a node from trashdb.
 *
 * p: pointer to path string.
 */
void
t_delete (const char *p)
{
    assert(p != NULL);

    char *bnm = basename (p);
    node *cur = db.head, *prv = NULL;

    while (cur != NULL)
    {
        if (strcmp (basename (cur->path), bnm) == 0)
        {
            if (prv != NULL)
                prv->next = cur->next;
            else if (cur == db.head)
                db.head = cur->next;
            if (cur == db.tail)
                db.tail = prv;

            free (cur);
            break;
        }
        prv = cur;
        cur = cur->next;
    }
}


/*
 * t_modify: modify existing trashdb entry with new value.
 *
 * n: node to be modified with given path string
 * path: new path string to be assigned to n
 */
void
t_modify (node *n, const char *path)
{
    assert (n != NULL && path != NULL);

    free (n->path);
    n->path = calloc ((strlen (path) + 1), sizeof (char));
    if (n->path != NULL)
        strcpy(n->path, path);
    else
        warn ("memmory error");
}


/*
 * t_search: search a path string in trashdb
 *
 * path: absolute path of the file in trash
 */
char *
t_search (const char *path)
{
    assert (path != NULL);

    char *ret = NULL, *bnm = basename (path);
    node *cur = db.head;

    while ((cur != NULL) && (bnm != NULL))
    {
        if (!strcmp (basename (cur->path), bnm))
        {
            ret = calloc ((strlen (cur -> path) + 1), sizeof (char));
            strcpy (ret, cur->path);
            break;
        }
        cur = cur->next;
    }

    return ret;
}

/*
 * t_search_node: search a path string in trashdb
 *
 * path: absolute path of the file in trash
 */
node *
t_search_node (const char *path)
{
    assert (path != NULL);
    
    char *bnm = basename (path);
    node *ret = NULL, *cur = db.head;

    while ((bnm != NULL) && (cur != NULL))
    {
        if(!strcmp (basename (cur->path), bnm))
        {
            ret = cur;
            break;
        }    
        cur = cur->next;
    }

    return ret;
}

/* display trashdb */
void
t_display (void)
{
    int cnt = 0;
    node *cur = db.head;

    while (cur != NULL)
    {
        printf ("%3d: |%s|\n", ++cnt, cur->path);
        cur = cur->next;
    }
}

/* initialize and build trashdb */
void
t_read (void)
{
    const int SIZE = 256;

    char buf[SIZE];
    FILE *fp = NULL;

    if ((fp = fopen (tdb, "r")) != NULL)
    {    
        db.head = db.tail = NULL;
        while (fgets (buf, SIZE, fp) != NULL)
        {
            buf[strlen(buf) - 1] = '\0';
            t_insert (buf);
        }
    }
    else
        warn ("could not open file `%s'", tdb);
}

/*
 * write trashdb to file pointed to by tdb returns +1 when successful or -1 on
 * error.
 */
int
t_write (void)
{
    int ret = 1;
    FILE *fp = NULL;
    node *cur = NULL;

    if ((fp = fopen (tdb, "w")) != NULL)
    {
        cur = db.head;
        while (cur != NULL)
        {
            fputs (cur->path, fp);
            fputc ('\n', fp);
            cur = cur->next;
        }
    }
    else
    {
        warn ("could not open file `%s'", tdb);
        ret = -1;
    }

    return ret;
}
