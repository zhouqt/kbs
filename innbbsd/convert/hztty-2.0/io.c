/* $Id$ */

/* Copyright 1992	Yongguang Zhang	*/

#ifndef lint
static char *rcs_id = "$Id$";
#endif                          /* lint */

#include "config.h"
#include "io.h"

static char *dummy(s)
char *s;
{
    return (s);
}
static int dummy_init()
{
    return (0);
}

/* all implemented modules for io streams are listed here */
struct mod_def moduleTable[] = {

    {"gb2hz", gb2hz, gb2hz_init, 0,
     ENC_GB, ENC_HZ, "GB => HZ"},
    {"hz2gb", hz2gb, hz2gb_init, 0,
     ENC_HZ, ENC_GB, "HZ => GB"},

#ifdef HZTTY_GB_BIG5
    {"gb2big", gb2big, gb2big_init, 0,
     ENC_GB, ENC_BIG5, "GB => Big5"},
    {"big2gb", big2gb, big2gb_init, 0,
     ENC_BIG5, ENC_GB, "Big5 => GB"},
#endif

    {"uni-utf", uni_utf8, uni_utf8_init, 0,
     ENC_UNICODE, ENC_UTF8, "Unicode (16bit) => UTF-8"},
    {"utf-uni", utf8_uni, utf8_uni_init, 0,
     ENC_UTF8, ENC_UNICODE, "UTF-8 => Unicode (16bit)"},

    {"uni-utf7", uni_utf7, uni_utf7_init, 0,
     ENC_UNICODE, ENC_UTF7, "Unicode (16bit) => UTF7 (mail-safe)"},
    {"utf7-uni", utf7_uni, utf7_uni_init, 0,
     ENC_UTF7, ENC_UNICODE, "UTF7 (mail-safe) => Unicode (16bit)"},

# ifdef HZTTY_UNICODE_GB
    {"gb2uni", gb2uni, gb2uni_init, 0,
     ENC_GB, ENC_UNICODE, "GB => Unicode"},
    {"uni2gb", uni2gb, uni2gb_init, 0,
     ENC_UNICODE, ENC_GB, "Unicode => GB"},
# endif

# ifdef HZTTY_UNICODE_BIG5
    {"big2uni", big2uni, big2uni_init, 0,
     ENC_BIG5, ENC_UNICODE, "Big5 => Unicode"},
    {"uni2big", uni2big, uni2big_init, 0,
     ENC_UNICODE, ENC_BIG5, "Unicode => Big5"},
# endif

    {"toascii", to_ascii, toascii_init, 0,
     ENC_ANY, ENC_ANY, "Strip the 8th bit"},
    {"toupper", to_upper, toupper_init, 0,
     ENC_ANY, ENC_ANY, "make everything upper-case"},
    {"tolower", to_lower, tolower_init, 0,
     ENC_ANY, ENC_ANY, "make everything lower-case"},

    {"rot13", rot13, rot13_init, 0,
     ENC_ANY, ENC_ANY, "ROT-13 (don't try this at home :-)"},

    {"log", io_log, log_init, 1,
     ENC_ANY, ENC_ANY, "keep a log in file (arg: filename)"},

    {"dummy", dummy, dummy_init, 0,
     ENC_ANY, ENC_ANY, "does nothing"},
    {"", dummy, dummy_init, 0,
     ENC_ANY, ENC_ANY, ""},
    {NULL, dummy, dummy_init, 0,
     ENC_ANY, ENC_ANY, NULL}
};

/* input/output stream */
struct io_stream {
    struct io_module {
        char *(*func) ();       /* the conversion function */
        int inst;               /* instant number for the function */
    } st[MAX_MODULE];
    int len;                    /* number of modules in the stream */
} in_stream, out_stream;

static int stream_init(str, pstream)
char *str;
struct io_stream *pstream;
{
    char *pstr = str;
    char *name, *arg;
    int more_module, has_arg;
    struct mod_def *pmod;
    int inst;

    pstream->len = 0;
    if ((!pstr) || (!*pstr) || ((*pstr == '-') && (*(pstr + 1) == '\0')))
        return (0);

    do {
        /*
         * A stream definition string looks like:
         *    <stream>  ::=  <module> { ':' <module> }
         *    <module>  ::=  <module_name> [ '(' <arguments> ')' ]
         */

        name = pstr;
        for (; (*pstr && (*pstr != ':')); pstr++)
            /*
             * empty 
             */ ;
        more_module = (*pstr == ':') ? 1 : 0;   /* more modules? */
        *pstr++ = '\0';         /* null-terminate the module */

        for (arg = name; (*arg) && (*arg != '('); arg++)
            /*
             * empty 
             */ ;
        has_arg = ((*arg == '(') ? 1 : 0);      /* has arguments? */
        *arg++ = '\0';          /* null-terminate the module_name */

        /*
         * search for the name in the module table 
         */
        for (pmod = moduleTable; pmod->name; pmod++)
            if (strcmp(name, pmod->name) == 0)
                break;
        if (!pmod->name) {      /* not found */
            fprintf(stderr, "no such module \"%s\"\n", name);
            return (-1);
        }
        if (pmod->needarg != has_arg) {
            fprintf(stderr, "\"%s:%s\" argument mismatch\n", name, arg);
            return (-1);
        }

        pstream->st[pstream->len].func = pmod->func;
        if (pmod->needarg) {
            /*
             * remember that arg end with a ')' 
             */
            arg[strlen(arg) - 1] = '\0';
            inst = (*pmod->f_init) (arg);
        } else
            inst = (*pmod->f_init) (NULL);
        if (inst < 0) {
            fprintf(stderr, "fail to initialize \"%s\"\n", name);
            return (-1);
        }
        pstream->st[pstream->len].inst = inst;
        pstream->len++;

    } while (more_module);
    return (0);
}


int in_stream_setup(idef)
char *idef;
{
    char defstr[256];

    strcpy(defstr, (idef ? idef : ""));
    return (stream_init(defstr, &in_stream));
}

int out_stream_setup(odef)
char *odef;
{
    char defstr[256];

    strcpy(defstr, (odef ? odef : ""));
    return (stream_init(defstr, &out_stream));
}

/*
 * Conversion buffer.  One for both directions.  They are in different
 * address space anyway.  Set the area to be at the middle of the buffer.
 * Just play safe
 */
static char conv_buf[BUFSIZ * 3];
static char *buf = conv_buf + BUFSIZ;

/* pty --> stdout, do out_stream */
int stream_write(fd, s, len)
int fd;
char *s;
int len;
{
    int i;
    char *p = buf;              /* point to the buffer for conversion */

    bcopy(s, p, len);           /* copy in */
    for (i = 0; i < out_stream.len; i++)
        p = (*out_stream.st[i].func) (p, &len, out_stream.st[i].inst);
    return ((len != 0) ? write(fd, p, len) : 0);
}

/* stdin --> pty, do in_stream */
int stream_read(fd, s, len)
int fd;
char *s;
int len;
{
    int i, cc;
    char *p = buf;              /* point to the buffer for conversion */

    cc = read(fd, p, len);
    if (cc <= 0)
        return (-1);
    for (i = 0; i < in_stream.len; i++)
        p = (*in_stream.st[i].func) (p, &cc, in_stream.st[i].inst);
    if (cc != 0)
        bcopy(p, s, cc);        /* copy out */
    return (cc);
}

char *hzconvert(s, plen, psaved, dbcvrt)
char *s;
int *plen;
char *psaved;                   /* unprocessed char buffer pointer */
void (*dbcvrt) ();              /* 2-byte conversion func for a hanzi */
{
    char *p, *pend;

    if (*plen == 0)
        return (s);
    if (*psaved) {              /* previous buffered char */
        *(--s) = *psaved;       /* put the unprocessed char down */
        (*plen)++;
        *psaved = 0;            /* clean this char buffer */
    }
    p = s;
    pend = s + (*plen);         /* begin/end of the buffer string */
    while (p < pend) {
        if ((*p) & 0x80)        /* hi-bit on: hanzi */
            if (p < pend - 1)   /* not the last one */
                dbcvrt(p++);
            else {              /* the end of string */
                *psaved = *p;   /* save the unprocessed char */
                (*plen)--;
                break;
            }
        p++;
    }
    return (s);
}
