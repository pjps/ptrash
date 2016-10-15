/*
 * trashdb.c -- move unwanted files to trash; This file is part of the program
 * 'ptrash'.
 * Copyright (C) 2016 Prasad J Pandit
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
 */

#include <ptrash.h>
#include <time.h>

char *tdb = NULL;

void
t_insert (const char *path)
{
    int fd;
    time_t t = 0;
    char buf[1024], dtm[20], *fp = NULL;

    assert (path != NULL);

    snprintf (buf, sizeof (buf), "%s.trashinfo", basename (path));
    fp = build_path (tdb, buf);
    fd = open (fp, O_CREAT|O_EXCL|O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd < 0)
        warn ("could not open file: `%s'", fp);

    time (&t);
    strftime (dtm, sizeof (dtm), "%Y%m%dT%T", localtime (&t));
    t = snprintf (buf, sizeof (buf),
                "%s\nPath=%s\nDeletionDate=%s\n", "[Trash Info]", path, dtm);

    write (fd, buf, t);
    fsync (fd);

    free (fp);
    close (fd);
}

void
t_delete (const char *path)
{
    char buf[1024], *fp = NULL;

    assert (path != NULL);

    snprintf (buf, sizeof (buf), "%s.trashinfo", basename (path));
    fp = build_path (tdb, buf);

    if (truncate (fp, 0) < 0)
        warn ("could not truncate file `%s'", fp);
    if (unlink (fp) < 0)
        warn ("could not remove file `%s'", fp);

    free (fp);
}

static char *
read_line (int fd)
{
    int i = 0;
    char buf[1024];

    while (read (fd, &buf[i], 1) > 0)
    {
        if (buf[i] == '\n')
            break;
        i++;
    }
    buf[i++] = '\0';

    return strndup (buf, i);
}

char *
t_search (const char *path)
{
    int fd;
    char *ret = NULL;
    char buf[1024], *fp = NULL, *ln = NULL;

    assert (path != NULL);

    snprintf (buf, sizeof (buf), "%s.trashinfo", basename (path));
    fp = build_path (tdb, buf);
    fd = open (fp, O_RDONLY|O_CLOEXEC);
    if (fd < 0)
        warn ("could not open file `%s'", fp);

    ln = read_line (fd);
    if (strncmp (ln, "[Trash Info]", sizeof ("[Trash Info]")))
        warnx ("invalid Trash Info entry `%s'", fp);
    free (ln);

    ln = read_line (fd);
    if (strncmp (ln, "Path=", sizeof ("Path")))
        warnx ("invalid Path entry `%s'", fp);
    ret = strdup (&ln[5]);
    free (ln);

    ln = read_line (fd);
    if (strncmp (ln, "DeletionDate=", sizeof ("DeletionDate")))
        warnx ("invalid Date entry `%s'", fp);
    free (ln);

    free (fp);
    close (fd);

    return ret;
}
