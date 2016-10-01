/*
 * ptrash.c -- move unwanted files to trash; This file is part of the program
 * 'ptrash'.
 * Copyright (C) 2008 - 2015 Prasad J Pandit
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
#include <ptrashdb.h>

extern char *tdb;
extern int opterr, optind;

char *trsh = NULL, *pdir = NULL;
char *prog = NULL, *home = NULL, *pwd = NULL;

short mode = 0, perm = 0, omask = 0;
short OVER_WRITE = 0, RESTORE_DIR = 0, MOVE_DIR = 0, DELETE_DIR = 0;

/* operation mode */
enum op_mode { INTERACTIVE = 1, RESTORE = 2, DELETE = 4, VERBOSE = 8 };


void
usage (void)
{
    printf ("Usage: %s [OPTIONS] file-name-1 [file-name-2 ...]\n", prog);
    printf ("\tmove file(s) to $XDG_DATA_HOME/Trash directory\n");
}


void
printh (void)
{
    usage ();
    printf ("\nOptions: \n");
    printf ("%-17s %s\n", "  -d --delete", "delete files from trash");
    printf ("%-17s %s", "  -i", "interactive, confirm before over writing");
    printf ("%s\n", " or deleting a file");
    printf ("%-17s %s", "  -r --restore", "restore a file from trash to");
    printf ("%s\n", " its original location");

    printf ("\n");
    printf ("%-17s %s\n", "  -h --help", "shows this help");
    printf ("%-17s %s\n", "  -v --verbose", "verbose operation mode");
    printf ("%-17s %s\n", "  -V --version", "shows version information");
    printf ("\nReport bugs to <pjp at fedoraproject dot org>\n");
}


/* check_option: checks command line option and takes appropriate action */
int
check_option (int argc, char *argv[])
{
    int n = 0, ind = 0;
    const char optstr[] = "+dhirvV";

    struct option optlst[] = \
    {
        { "delete",  0, NULL, 'd' },
        { "help",    0, NULL, 'h' },
        { "restore", 0, NULL, 'r' },
        { "verbose", 0, NULL, 'v' },
        { "version", 0, NULL, 'V' },
        { 0, 0, 0, 0 }
    };

    mode = opterr = 0;
    while ((n = getopt_long (argc, argv, optstr, optlst, &ind)) != -1)
    {
        switch (n)
        {
        case 'd':
            if (mode & RESTORE)
                goto invopt;
            mode |= DELETE;
            break;

        case 'h':
            printh ();
            exit (0);

        case 'i':
            mode |= INTERACTIVE;
            break;

        case 'r':
            if (mode & DELETE)
                goto invopt;
            mode |= RESTORE;
            break;

        case 'v':
            mode |= VERBOSE;
            break;

        case 'V':
            printf ("%s version %s\n", prog, VERSION);
            exit (0);

invopt: default:
            usage ();
            exit (-1);
        }
    }

    return optind;
}


/* path: absolute path of the directory to be created */
int
create_dir (const char *path)
{
    int ret = 0;

    assert (path != NULL);

    if (mkdir (path, perm) < 0)
        if (errno != EEXIST)
        {
            warn ("could not create directory `%s'", path);
            ret = -1;
        }

    return ret;
}


/*
 * build_path: build a path string pointing to the file. Allocates memory,
 * make sure you free it.
 *
 * dir: absolute path of the base directory
 * file: name of the file
 */
char *
build_path (const char *dir, const char *file)
{
    assert ((dir != NULL) && (file != NULL));

    int dl = strlen (dir), fl = strlen (file);
    char *strtemp = calloc ((dl + fl + 2), sizeof (char));

    if (strtemp != NULL)
    {
        strncpy (strtemp, dir, dl);
        if (dir[dl - 1] != '/')
            strncat (strtemp, "/", 1);
        strcat (strtemp, file);
    }

    return (strtemp);
}


/*
 * dst_path: build a destination path for the file to be moved.
 *
 * spath: absolute path of the source file.
 */
char *
dst_path (const char *spath)
{
    char *fp = NULL;

    assert (spath != NULL);

    if ((mode & RESTORE) && (!RESTORE_DIR))
    {
        if ((fp = t_search (spath)) == NULL)
            errx (-1, "could not retrieve restore path of `%s'", spath);
    }
    else
        fp = build_path (pdir, basename (spath));

    return fp;
}


/*
 * open_src_file: opens the source file which is to be trashed.
 *
 * file: absolute path of the file to be trashed
 */
int
open_src_file (char *file)
{
    int fd = -1;

    assert (file != NULL);

    if ((fd = open (file, O_RDONLY)) < 0)
        warn ("could not open file `%s'", file);

    return fd;
}


/*
 * open_dst_file: open the destination file under '$XDG_DATA_HOME/Trash'
 * directory.
 *
 * file: absolute path of the file to open or create.
 */
int
open_dst_file (char *file)
{
    int fd = -1;

    assert (file != NULL);

    OVER_WRITE = 0;    /* do not over write */
    if((file = dst_path (file)) != NULL)
    {
        fd = open (file, O_CREAT|O_EXCL|O_WRONLY, perm);
        if (fd < 0)
        {
            if (errno == EEXIST)
            {
                if (mode & INTERACTIVE  &&  !get_choice (file, "overwrite"))
                        return fd;

                OVER_WRITE = 1;    /* over write file */
                fd = open (file, O_CREAT|O_WRONLY|O_TRUNC, perm);
                if (fd < 0)
                    warn ("could not open file `%s'", file);
            }
        }
        free (file);
    }

    return fd;
}


/*
 * get_choice: this function reads a choice yes/no and returns 1 for yes and 0
 * for no.
 */
short
get_choice (char *file, const char *prompt)
{
    char c = '0';
    short ret = 0;      /* negative choice */

    printf ("%s: %s `%s'?", prog, prompt, basename (file));
    printf (" [y/n]: ");
    scanf ("%c", &c);
    getchar ();
    if ((c == 'Y') || (c == 'y'))
        ret = 1;    /* positive choice */

    return ret;
}


/*
 * update_tdb: add/remove an entry of the file(last moved) to/from Trash
 * database under Trash/info directory.
 *
 * path: absolute path of the file last moved by move
 */
int
update_tdb (char *path)
{
    assert (path != NULL);

    if ((mode & RESTORE) || (mode & DELETE))
        t_delete (path);
    else
        t_insert (path);

    return 1;
}


/*
 * copy_file: copy source file to destination file.
 *
 * dst: file descriptor of destination file.
 * src: file descriptor of source file.
 */
int
copy_file (int dst, int src)
{
    char *buff = NULL;
    struct stat stat_buf;
    int rcnt = 0, bcnt = 0, cnt = 0, blk = 0;
    float slice = 0.0f, inc = 0.0f, div = 25.0f;

    assert ((src >= 0) && (dst >= 0));

    fstat (src, &stat_buf);
    if (mode & VERBOSE)
        slice = (stat_buf.st_size / div);
    blk = stat_buf.st_blksize;
    buff = calloc (blk, sizeof (char));
    while ((rcnt = read (src, buff, blk)) > 0)
    {
        if (write (dst, buff, rcnt) < 0)
            return -1;    /* copy error */

        bcnt += rcnt;
        if (mode & VERBOSE)
        {
            while ((bcnt > inc) && (cnt < div))
            {
                printf("%c%s", '\b', "=>");
                inc += slice;
                cnt++;
            }
            fflush (stdout);
        }
        memset (buff, '\0', blk);
    }
    free (buff);

    return 1;
}


/* main: main function starts the execution */
int
main (int argc, char *argv[])
{
    int n = 0, l = 0;
    struct stat stat_buf;

#ifdef __GLIBC__
    extern char * dirname (char *);
#endif

    prog = argv[0];
    n = check_option (argc, argv);
    argc -= n;
    argv += n;

    if (argc == 0)
    {
        usage ();
        return -1;
    }
    if (init_move () == -1)
        return -1;

    n = 0;
    while (n < argc)
    {
        char *fnm = NULL;
        long pmax = pathconf (argv[n], _PC_PATH_MAX);

        pdir = trsh;
        OVER_WRITE = RESTORE_DIR = MOVE_DIR = DELETE_DIR = 0;
        if ((mode & RESTORE) || (mode & DELETE))
        {
            l = strlen (argv[n]);
            if (argv[n][l-1] == '/')
                argv[n][l-1] = '\0';
            fnm = build_path (trsh, basename (argv[n]));
        }
        else
        {
            fnm = calloc (pmax, sizeof (char));
            realpath (argv[n], fnm);
        }
        if (lstat (fnm, &stat_buf) == 0)
        {
            /*
             * If a file is under '$XDG_DATA_HOME/Trash' and
             * restore(-r) is NOT used, delete it.
             */
            char *dnm = strdup (fnm);

            dnm = dirname (dnm);
            if (!strcmp (trsh, dnm) && !(mode & RESTORE))
                mode |= DELETE;
            free (dnm);

            if (mode & DELETE)
                delete (fnm, &stat_buf);
            else
                move (fnm, &stat_buf);
        }
        else
            err (-1, "could not locate file `%s'", argv[n]);

        free (fnm);
        n++;
    }
    free (trsh);
    free (tdb);
    umask (omask);

    return 0;
}


/* init_move: makes necessary initialisations like home, pwd variables */
int
init_move (void)
{
    short msk = 0000;
    struct passwd *pw = NULL;

    pw = getpwuid (getuid ());
    trsh = build_path (pw->pw_dir, ".local/share/Trash/files");
    tdb  = build_path (trsh, "../info/");
    perm = S_IRWXU;
    if ((create_dir (trsh) == -1) || (create_dir (tdb) == -1))
    {
        warnx ("initialisation error");
        return -1;
    }
    pdir = trsh;
    omask = umask (msk);

    return 1;
}


/*
 * move: checks the file type and calls the appropriate move_<file type>
 * function to move that file to $XDG_DATA_HOME/Trash.
 *
 * file: absolute path of the file to be trashed.
 * stat_buf: pointer pointing to stat structure of file.
 */
int
move (char *file, struct stat *stat_buf)
{
    short flag = 0;
    perm = stat_buf->st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

    if (S_ISDIR (stat_buf->st_mode))
    {    /* file: is directory     */
        MOVE_DIR++;
        if (mode & RESTORE)
            RESTORE_DIR++;
        if (!move_dir (file))
        {
            if (!--MOVE_DIR)
                update_tdb (file);
            rmdir (file);
        }
        RESTORE_DIR--;
    }
    else if (S_ISREG (stat_buf->st_mode))
    {    /* file: is regular     */
        if (!move_reg (file))
            flag = 1;
    }
    else if (S_ISFIFO (stat_buf->st_mode))
    {    /* file: is fifo     */
        if (!move_fifo (file))
            flag = 1;
    }
    else if (S_ISCHR (stat_buf->st_mode) || S_ISBLK (stat_buf->st_mode))
    {    /* file: is character/block special device file    */
        if (!move_nod (file))
            flag = 1;
    }
    else
        warnx ("can not move this type of file!");

    if (flag)
    {
        if (!MOVE_DIR)
            update_tdb (file);
        remove (file);
    }
    perm = 0000;

    return 0;
}


/*
 * move_reg: does the actual movement of regular file. Returns 0 on success
 * and -1 in case of an error.
 *
 * fpath: absolute path of the file to be trashed.
 */
int
move_reg (char *fpath)
{
    int s = open_src_file (fpath);
    int d = open_dst_file (fpath);

    if ((s == -1) || (d == -1))
        return -1;

    if (mode & VERBOSE)
    {
        printf ("moving: %-25s |>", basename (fpath));
        fflush (stdout);
    }
    if (copy_file (d, s) == -1)
        return -1;
    if (mode & VERBOSE)
        printf ("%c%s", '\b', "|\n");

    close (s);
    close (d);

    return 0;
}


/*
 * move_fifo: moves the fifo special file to $XDG_DATA_HOME/Trash.
 * Returns 0 on success and -1 in case of an error.
 *
 * fpath: absolute path of the file to be moved.
 */
int
move_fifo (char *fpath)
{
    int ret = 0;
    char *fp = NULL;
    struct stat stat_buf;

    assert (fpath != NULL);

    lstat (fpath, &stat_buf);
    fp = dst_path (fpath);

    if (mode & VERBOSE)
    {
        printf ("moving: %-25s |>", basename (fpath));
        fflush (stdout);
    }
    if ((ret = mkfifo (fp, stat_buf.st_mode)) < 0)
         err (-1, "could not create fifo file `%s'", fp);
    if (mode & VERBOSE)
        printf ("%c%27s", '\b', "|\n");

    free (fp);
    return ret;
}


/*
 * move_dir: moves a whole directory to $XDG_DATA_HOME/Trash.
 *
 * dpath: absolute path of the directory to be trashed.
 */
int
move_dir (char *dpath)
{
    DIR *d = NULL;
    struct stat buf;
    char *lpdir = NULL;
    struct dirent *dent = NULL;
    static unsigned char flag = 0;

    assert (dpath != NULL);

    if ((mode & RESTORE) && (!flag))
    {
        lpdir = pdir = t_search (dpath);
        if (!pdir)
            errx (-1, "could not retrieve restore path of `%s'", dpath);
        flag = 1;
    }
    else
        lpdir = pdir = build_path (pdir, basename (dpath));

    if (create_dir (pdir) == -1)
        return -1;

    d = opendir (dpath);
    while ((dent = readdir (d)) != NULL)
    {
        char *p = NULL, *dnm = dent->d_name;

        p = build_path (dpath, dnm);
        if (p == NULL)
            break;
        lstat (p, &buf);
        perm = buf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
        if (S_ISDIR (buf.st_mode)
            && (!strcmp (dnm, ".") || !strcmp (dnm, "..")))
            continue;

        move (p, &buf);
        pdir = lpdir;
        perm = 0;
        free (p);
    }
    free (lpdir);
    closedir (d);

    return 0;
}


/*
 * move_nod: moves the device special (character|block) file to
 * $XDG_DATA_HOME/Trash. Returns 0 on success and -1 in case of an error.
 * User must be root to do this.
 *
 * npath: absolute path of the file to be moved.
 */
int
move_nod (char *npath)
{
    int ret = 0;
    char *fp = NULL;
    struct stat stat_buf;

    assert (npath != NULL);

    fp = dst_path (npath);
    lstat (npath, &stat_buf);
    if (mode & VERBOSE)
    {
        printf ("moving: %-25s |>", basename (npath));
        fflush (stdout);
    }
    if ((ret = mknod (fp, stat_buf.st_mode, stat_buf.st_dev)) < 0)
        err (-1, "could not create file `%s'", basename (fp));
    if (mode & VERBOSE)
        printf ("%c%27s", '\b', "|\n");

    free (fp);
    return ret;
}


/*
 * delete: deletes the named file from $XDG_DATA_HOME/Trash directory.
 * Returns 0 on success and -1 in case of error.
 *
 * file: name of the file in trash to be deleted.
 * stat_buf: pointer to stat structure of @file.
 */
int
delete (char *file, struct stat *stat_buf)
{
    assert (file != NULL && stat_buf != NULL);

    if ((mode & INTERACTIVE) && (!get_choice (file, "delete")))
        return -1;
    if (mode & VERBOSE)
        printf ("removing: %s\n", basename (file));
    if (S_ISDIR (stat_buf->st_mode))
    {
        DELETE_DIR++;
        if (!delete_dir (file))
        {
            if(!--DELETE_DIR)
                update_tdb (file);
            rmdir (file);
        }
    }
    else if (!remove (file))
    {
        if (!DELETE_DIR)
            update_tdb (file);
    }
    else
        err (-1, "could not remove file `%s'", file);

    return 0;
}


/*
 * delete_dir: removes directory from $XDG_DATA_HOME/Trash. On success it
 * returns 0, and returns -1 in case of an error.
 *
 * dpath: path of the directory to be deleted from $XDG_DATA_HOME/Trash
 */
int
delete_dir (char *dpath)
{
    DIR *d = NULL;
    struct stat buf;
    struct dirent *dent = NULL;

    assert (dpath != NULL);

    d = opendir (dpath);
    while ((dent = readdir (d)) != NULL)
    {
        char *p = NULL, *dnm = dent->d_name;
        if ((p = build_path (dpath, dnm)) == NULL)
            break;
        lstat (p, &buf);
        if (S_ISDIR (buf.st_mode) && ((strcmp (dnm, ".") == 0)
            || (strcmp (dnm, "..") == 0)))
            continue;

        delete (p, &buf);
        free (p);
    }
    closedir (d);

    return 0;
}
