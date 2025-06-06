/* read specified file, and return pointer to string buffer */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "www.h"

extern int debug;
extern Doc NewDoc;

#define BUFSIZE     4096
#define THRESHOLD   512

char *CurrentDirectory(void)
{
    int n;
    static char buf[256];

    getcwd(buf, 254);
    n = strlen(buf);
    buf[n++] = '/';
    buf[n] = '\0';
    return buf;
}

/* uncompress buffer - buffer is freed and reallocated */

char *Uncompress(char *buf, long *length)
{
    char *p, *tmpfile, cmd[128];
    int len, size, c;
    FILE *fp;

    Announce("Uncompressing data ...");

    len = *length;
    tmpfile = tempnam(".", "w3");

    p = buf;

    if ((fp = fopen(tmpfile, "w")) == 0)
        Warn("can't open temporary image file: %s", tmpfile);
    else
    {
        while (len-- > 0)
            putc(*p++, fp);

        fclose(fp);

        sprintf(cmd, "zcat < %s", tmpfile);
        fp = popen(cmd, "r");


        if (fp == NULL)
        {
            Warn("Couldn't run uncompress command");
            return NULL;
        }

        size = BUFSIZE;
        p = malloc(size);

        if (p == NULL)
        {
            Warn("Couldn't malloc buffer size %d", size);
            return NULL;
        }

        buf = p;
        len = 0;

        while (( c = getc(fp)) != EOF)
        {
            if (len >= size - THRESHOLD)
            {
                size *= 2;  /* attempt to double size */

                p = realloc(buf, size);

                if (p == NULL)
                {
                    Warn("can't realloc buffer to size %ld", size);
                    buf[len] = 0;
                    return buf;
                }

                buf = p;
            }

            buf[len++] = c;
        } 

        pclose(fp);
        unlink(tmpfile);
        buf[len] = '\0';

        *length = len;
    }

    return buf;
}


/* check if file can be displayed using xv */

int HasXVSuffix(char *name)
{
    char *p;

    p = strrchr(name, '.');

    if (strcasecmp(p, ".gif") == 0)
        return 1;

    if (strcasecmp(p, ".jpeg") == 0)
        return 1;

    if (strcasecmp(p, ".jpg") == 0)
        return 1;

    if (strcasecmp(p, ".tiff") == 0)
        return 1;

    if (strcasecmp(p, ".tif") == 0)
        return 1;

    if (strcasecmp(p, ".pbm") == 0)
        return 1;

    if (strcasecmp(p, ".pgm") == 0)
        return 1;

    if (strcasecmp(p, ".ppm") == 0)
        return 1;

    if (strcasecmp(p, ".xbm") == 0)
        return 1;

    if (strcasecmp(p, ".xpm") == 0)
        return 1;

    if (strcasecmp(p, ".pm") == 0)
        return 1;

    if (strcasecmp(p, ".ras") == 0)
        return 1;

    return 0;
}

/* recognise file suffix */

int FileSuffix(char *path)
{
    int c, type;
    char *q, *r;

    r = strrchr(NewDoc.path, '?');

    if (r)
    {
        c = *r;
        *r = '\0';
    }

    q = strrchr(NewDoc.path, '/');

    if (!q)
        q = NewDoc.path;

    q = strrchr(NewDoc.path, '.');
    type = -1;

    if (q && strchr(q, '/') == NULL)
    {
        if (strcasecmp(q, ".html") == 0 || strcasecmp(q, ".htm") == 0)
            type = HTMLDOCUMENT;
        else if (strcasecmp(q, ".txt") == 0 || strcasecmp(q, ".doc") == 0)
            type = TEXTDOCUMENT;
        else if (strcasecmp(q, ".ps") == 0 || (NewDoc.buffer && strncmp(NewDoc.buffer, "%!PS", 4) == 0))
            type = PSDOCUMENT;
        else if (HasXVSuffix(q))
            type = XVDOCUMENT;
        else if (strcasecmp(q, ".dvi") == 0)
            type = DVIDOCUMENT;
        else if (strcasecmp(q, ".mpeg") == 0 || strcasecmp(q, ".mpg") == 0)
            type = MPEGDOCUMENT;
        else if (strcasecmp(q, ".au") == 0)
            type = AUDIODOCUMENT;
        else if (strcasecmp(q, ".xwd") == 0)
            type = XWDDOCUMENT;
        else if (strcasecmp(q, ".mime") == 0)
            type = MIMEDOCUMENT;
    }

    if (r)
        *r = c;

    return type;
}

/* determine document type */

int NewDocumentType(void)
{
    int type;
    char *p, *q;

    NewDoc.type = TEXTDOCUMENT;
    p = strrchr(NewDoc.path, '.');

    /* if document is uncompressed, buffer will change address */

    if (p && strcasecmp(p, ".z") == 0 && (q = Uncompress(NewDoc.buffer+NewDoc.hdrlen, &NewDoc.length)))
    {
        *p = '\0';
        Free(NewDoc.buffer);
        NewDoc.buffer = q;
        NewDoc.hdrlen = 0;

        if ((type = FileSuffix(NewDoc.path)) != -1)
            NewDoc.type = type;

        if (IsHTMLDoc(NewDoc.buffer, NewDoc.length))
            NewDoc.type = HTMLDOCUMENT;

        *p = '.';
    }
    else
    {
        if ((type = FileSuffix(NewDoc.path)) != -1)
            NewDoc.type = type;

        if (IsHTMLDoc(NewDoc.buffer, NewDoc.length))
            NewDoc.type = HTMLDOCUMENT;
    }

    return NewDoc.type;
}

/* check HTML file for <SOURCE href="http://info.cern.ch:8001 ... >
   defining the context for this file, for use in expanding partial UDIs */

void CheckSource(char *buf)
{
    int n, i;
    char *p;

    p = buf;
    i = 1024;  /* only look in first 1K of file */

    while ((n = *buf++))
    {
        if (i-- <= 0)
            break;

        if (n != '<')
            continue;

        if (strncasecmp(buf, "source href=\"", 13) == 0)
        {
            buf += 13;
            p = buf;

            while ((n = *++p) && n != '"');

            *p = '\0';
            ParseReference(buf, NewDoc.where);
            *p = n;
        }
    }
}

/* return pointer to '/' terminating parent dir or just after 1st */

char *ParentDirCh(char *dir)
{
    char *p;

    p = dir + strlen(dir) - 2;  /* dir has optional trailing '/' */

    while (p > dir && *p != '/')
        --p;

    return (p > dir ? p : dir+1);
}

/* Take care with string fields to avoid memory leakage - GetFile()
   can be called before OR after ParseReference(), so free any
   currently assigned strings before assing new ones */

char *GetFile(char *name)
{
    int fd, c, AddSlash, len;
    unsigned int size;
    FILE *fp;
    char *buf, *p, *q, *r, *s, *me, lbuf[1024];

#if 0
  /* initialise NewDoc to reasonable values */

    NewDoc.type = TEXTDOCUMENT;
    NewDoc.cache = 0;
    NewDoc.hdrlen = 0;
    NewDoc.length = 0;
    NewDoc.height = 0;
    NewDoc.offset = 0;
    NewDoc.protocol = MYFILE;

    if (NewDoc.host)
        Free(NewDoc.host);

    NewDoc.host = strdup(MyHostName());
    NewDoc.port = 0;

    if (NewDoc.path)
        Free(NewDoc.path);

    if (NewDoc.anchor)
        Free(NewDoc.anchor);

    if ((p = strchr(name, '#')))
    {
        NewDoc.anchor = strdup(p+1);
        *p = '\0';
        NewDoc.path = strdup(name);
        *p = '#';
    }
    else
    {
        NewDoc.anchor = 0;
        NewDoc.path = strdup(name);
    }

    if (NewDoc.url)
        Free(NewDoc.url);

    if (NewDoc.cache)
        Free(NewDoc.cache);

    NewDoc.url = strdup(name);  /* note full url for printing */
#endif

    NewDoc.cache = strdup(name);
    NewDoc.type = TEXTDOCUMENT;

    if ((c = FileSuffix(name)) != -1)
        NewDoc.type = c;

#if 0
    switch (NewDoc.type)
    {
        case XVDOCUMENT:
            {
                sprintf(lbuf, "xv %s &", name);
                system(lbuf);
                Announce("using xv to show %s", name);
                FreeDoc(&NewDoc);
                return NULL;
            }

        case DVIDOCUMENT:
            {
                sprintf(lbuf, "xdvi %s &", name);
                system(lbuf);
                Announce("using xdvi to show %s", name);
                FreeDoc(&NewDoc);
                return NULL;
            }

        case PSDOCUMENT:
            {
                sprintf(lbuf, "ghostview %s &", name);
                system(lbuf);
                Announce("using ghostview to show %s", name);
                FreeDoc(&NewDoc);
                return NULL;
            }

        case MPEGDOCUMENT:
            {
                sprintf(lbuf, "mpeg_play %s &", name);
                system(lbuf);
                Announce("using mpeg_play to show %s", name);
                FreeDoc(&NewDoc);
                return NULL;
            }

        case AUDIODOCUMENT:
            {
                sprintf(lbuf, "showaudio %s &", name);
                system(lbuf);
                Announce("using showaudio to show %s", name);
                FreeDoc(&NewDoc);
                return NULL;
            }

        case XWDDOCUMENT:
            {
                sprintf(lbuf, "xwud -in %s &", name);
                system(lbuf);
                Announce("using xwud to show %s", name);
                FreeDoc(&NewDoc);
                return NULL;
            }

        case MIMEDOCUMENT:
            {
                sprintf(lbuf, "xterm -e metamail %s &", name);
                system(lbuf);
                Announce("using metamail to show %s", name);
                FreeDoc(&NewDoc);
                return NULL;
            }
    }
#endif

    if (chdir(NewDoc.path) == 0)  /* its a directory */
    {
        size = 2048;
        buf = malloc(size);


        if (buf == 0)
        {
            Warn("can't allocate buffer size %d", size);
            return 0;
        }

        me = MyHostName();

        sprintf(buf, "<TITLE>%s at %s</TITLE>\n<H1>%s at %s</H1>\n<PRE>\n",
                NewDoc.path, me, NewDoc.path, me);

        len = strlen(buf);

        /* check is a trailing slash is needed */

        AddSlash = (NewDoc.path[strlen(NewDoc.path)-1] != '/' ? 1 : 0);

        /* invoke ls command to list directory to FILE *fp */

        fp = popen("ls -al", "r");

        q = lbuf;

        while ( (c = getc(fp)) != EOF )
        {
            *q++ = c;

            if (c == '\n')
            {
                *--q = '\0';

                /* Unix dir list has total line */

                p = strrchr(lbuf, ' ');
            
                if (p && strncasecmp(lbuf, "total", 5) != 0)
                {
                    *p++ = '\0';

                    if (strcmp(p, ".") == 0)
                        goto next_line;
                    if (strcmp(p, "..") == 0 && (r = ParentDirCh(NewDoc.path)))
                    {
                        if (strcmp(NewDoc.path, "/") == 0)
                            goto next_line;

                        c = *r;
                        *r = '\0';
                        sprintf(buf+len, "%s <A HREF=\"%s\">", lbuf, NewDoc.path);
                        *r = c;
                    }
                    else if (*lbuf == 'l')
                    {
                        *(p - 4) = '\0';
                        r = strrchr(lbuf, ' ');
                        *r++ = '\0';

                        if (*p == '/')  /* absolute link */
                             sprintf(buf+len, "%s <A HREF=\"%s\">", lbuf, p);
                        else if (AddSlash)
                             sprintf(buf+len, "%s <A HREF=\"%s\">", lbuf, NewDoc.path, p);   
                        else
                             sprintf(buf+len, "%s <A HREF=\"%s%s\">", lbuf, NewDoc.path, p);
                    }
                    else if (AddSlash)
                        sprintf(buf+len, "%s <A HREF=\"%s/%s\">", lbuf, NewDoc.path, p);
                    else
                        sprintf(buf+len, "%s <A HREF=\"%s%s\">", lbuf, NewDoc.path, p);

                    len += strlen(buf+len);

                    if (lbuf[0] == 'd')
                        sprintf(buf+len, "%s/</A>\n", p);
                    else if (lbuf[0] == 'l')
                        sprintf(buf+len, "%s@</A>\n", r);
                    else if (lbuf[3] == 'x')
                        sprintf(buf+len, "%s*</A>\n", p);
                    else
                        sprintf(buf+len, "%s</A>\n", p);

                    len += strlen(buf+len);
                }
                else
                {
                     memcpy(buf+len, lbuf, q-lbuf);
                     len += q-lbuf;
                     buf[len++] = '\n';
                }

             next_line:

                q = lbuf;

                if (len >= size - THRESHOLD)
                {
                    size *= 2;  /* attempt to double size */

                    p = realloc(buf, size);

                    if (p == NULL)
                    {
                        Warn("can't realloc buffer to size %ld", size);
                        buf[len++] = 0;
                        NewDoc.type = HTMLDOCUMENT;
                        NewDoc.buffer = buf;
                        NewDoc.length = len;
                        return buf;
                    }

                    buf = p;
                }
            }
        }

        pclose(fp);
        sprintf(buf+len, "</PRE>");
        len += 6;
        buf[len++] = '\0';

        NewDoc.type = HTMLDOCUMENT;
        NewDoc.length = len;
        Announce(name);
        NewDoc.buffer = buf;
        return buf;
    }
    else if (errno != ENOTDIR)
    {
        if (strcmp(NewDoc.path, "default.html") != 0)
            Warn("%s: %s", NewDoc.path, strerror(errno));
        else
            Announce("%s: %s", NewDoc.path, strerror(errno));

        return NULL;
    }
    else if ((fd = open(NewDoc.path, O_RDONLY)) == -1)
    {
        if (strcmp(NewDoc.path, "default.html") != 0)
            Warn("%s: %s", NewDoc.path, strerror(errno));
        else
            Announce("%s: %s", NewDoc.path, strerror(errno));

        return NULL;
    }

    size = lseek(fd, 0L, SEEK_END);

    buf = malloc(1 + size);
    lseek(fd, 0L, SEEK_SET);

    /* the next line assumes that read won't return -1 look at man page! */
    buf[read(fd, (void *)buf, size)] = '\0';

    close(fd);

    /* Kludge for HTML docs with .txt or .doc suffix */
    /* if first non-space char is '<' assume as HTML doc */

    if (IsHTMLDoc(buf, size))
    {
         /*
           CheckSource is used to check for <source> tag
           and call ParseReference to ensure that URL looks
           like the original source. This is needed to make
           sense of relative references in anchors.

           Unfortunately, this also screws up PopDoc when it
           comes to restoring a document, that PopDoc thinks
           is a local file.

           PopDoc expects the path to be to a local file,
           not the path on the original remote server.
         */

        /* CheckSource(buf); */
        NewDoc.type = HTMLDOCUMENT;
    }


    ShowAbortButton(0);
    Announce(name);
    NewDoc.length = size;
    NewDoc.buffer = buf;

 /* recognise type and decompress as necessary */

    NewDocumentType();
    NewDoc.hdrlen = HeaderLength(NewDoc.buffer, &NewDoc.type);
    return buf;
}

