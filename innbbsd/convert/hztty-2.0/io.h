/* $Id$ */

#define	DEF_ISTREAM	"gb2hz"
#define	DEF_OSTREAM	"hz2gb"

#define	MAX_MODULE	16      /* maximun number of module in a stream */

/* constants for encoding string */
#define	ENC_ANY		""
#define	ENC_UNICODE	"UNICODE"
#define	ENC_UTF8	"UTF8"
#define	ENC_UTF7	"UTF7"
#define	ENC_GB		"GB"
#define	ENC_HZ		"HZ"
#define	ENC_BIG5	"BIG5"

/* record of implemented modules */
struct mod_def {
    char *name;
    char *(*func) ();           /* conversion function */
    int (*f_init) ();           /* initialization function */
    int needarg;                /* need arguments? */
    char *from_encoding;
    char *to_encoding;
    char *annotation;
};

extern struct mod_def moduleTable[];

extern int in_stream_setup(), out_stream_setup();
extern int stream_read(), stream_write();

/*
 * All io functions must take the following signatures:
 *	char *xx2yy (char *s, int *plen, int inst)
 *	int xx2yy_init (char *arg)
 */

extern char *gb2hz(), *hz2gb(); /* hz2gb.c */
extern int gb2hz_init(), hz2gb_init();  /* hz2gb.c */

#ifdef HZTTY_GB_BIG5
extern char *gb2big(), *big2gb();       /* b2g.c */
extern int gb2big_init(), big2gb_init();        /* b2g.c */
#endif

extern char *uni_utf8(), *utf8_uni();   /* unicode.c */
extern int uni_utf8_init(), utf8_uni_init();    /* unicode.c */
extern char *uni_utf7(), *utf7_uni();   /* unicode.c */
extern int uni_utf7_init(), utf7_uni_init();    /* unicode.c */

#ifdef HZTTY_UNICODE_GB
extern char *gb2uni(), *uni2gb();       /* g2u.c */
extern int gb2uni_init(), uni2gb_init();        /* g2u.c */
#endif

#ifdef HZTTY_UNICODE_BIG5
extern char *big2uni(), *uni2big();     /* b2u.c */
extern int big2uni_init(), uni2big_init();      /* b2u.c */
#endif

extern char *io_log();          /* log.c */
extern int log_init();          /* log.c */

extern char *rot13(), *to_ascii(), *to_upper(), *to_lower();    /* hack.c */
extern int rot13_init(), toascii_init(), toupper_init(), tolower_init();
