#ifndef BBS_OLDSTRUCT_H
#define BBS_OLDSTRUCT_H

#define MYPASSLEN 14

struct olduserec {               /* Structure used to hold information in */
        char            userid[IDLEN+2];   /* PASSFILE */
        time_t          firstlogin;
        char            lasthost[16];
        unsigned int    numlogins;
        unsigned int    numposts;
        char            flags[2];
        char            passwd[MYPASSLEN];
        char            username[NAMELEN];
        char            ident[NAMELEN];
        char            termtype[16];
        char            reginfo[STRLEN-16];
        unsigned int    userlevel;
        time_t          lastlogin;
        time_t          stay;
        char            realname[NAMELEN];
        char            address[STRLEN];
        char            email[STRLEN-12];
        unsigned int    nummails;
        time_t          lastjustify;
        char            gender;
        unsigned char   birthyear;
        unsigned char   birthmonth;
        unsigned char   birthday;
        int             signature;
        unsigned int    userdefine;
        time_t          notedate;
        int             noteline;
};

#define MY_PERM_BASIC      000001
#define MY_PERM_CHAT       000002
#define MY_PERM_PAGE       000004
#define MY_PERM_POST       000010
#define MY_PERM_LOGINOK    000020
#define MY_PERM_DENYPOST   000040
#define MY_PERM_CLOAK      000100
#define MY_PERM_SEECLOAK   000200
#define MY_PERM_XEMPT      000400
#define MY_PERM_WELCOME    001000
#define MY_PERM_BOARDS     002000
#define MY_PERM_ACCOUNTS   004000
#define MY_PERM_CHATCLOAK  010000
#define MY_PERM_OVOTE      020000
#define MY_PERM_SYSOP      040000
#define MY_PERM_POSTMASK  0100000
#define MY_PERM_ANNOUNCE  0200000
#define MY_PERM_OBOARDS   0400000
#define MY_PERM_ACBOARD   01000000
#define MY_PERM_NOZAP     02000000
#define MY_PERM_FORCEPAGE 04000000
#define MY_PERM_EXT_IDLE  010000000
#define MY_PERM_SPECIAL1  020000000
#define MY_PERM_CHATOP    040000000
#define MY_PERM_SPECIAL3  0100000000
#define MY_PERM_SPECIAL4  0200000000
#define MY_PERM_SPECIAL5  0400000000
#define MY_PERM_SPECIAL6  01000000000
#define MY_PERM_SPECIAL7  02000000000
#define MY_PERM_SPECIAL8  04000000000

#define MY_DEF_FRIENDCALL		 000001
#define MY_DEF_ALLMSG			 000002
#define MY_DEF_FRIENDMSG		 000004
#define MY_DEF_SOUNDMSG			 000010
#define MY_DEF_COLOR			 000020
#define MY_DEF_ACBOARD			 000040
#define MY_DEF_ENDLINE			 000100
#define MY_DEF_EDITMSG			 000200
#define MY_DEF_NOTMSGFRIEND		 000400
#define MY_DEF_NORMALSCR		 001000
#define MY_DEF_NEWPOST			 002000
#define MY_DEF_CIRCLE			 004000
#define MY_DEF_FIRSTNEW			 010000
#define MY_DEF_LOGFRIEND		 020000
#define MY_DEF_INNOTE			 040000
#define MY_DEF_OUTNOTE			 0100000
#define MY_DEF_MAILMSG			 0200000
#define MY_DEF_LOGOUT			 0400000
#define MY_DEF_LOGINFROM		 01000000
#define MY_DEF_NOTEPAD			 02000000
#define MY_DEF_NOLOGINSEND		 04000000
#define MY_DEF_THESIS			 010000000
#define MY_DEF_MSGGETKEY		 020000000
#define MY_DEF_GRAPH			 040000000
#define MY_DEF_TOP10			 0100000000
#define MY_DEF_RANDSIGN			 0200000000
#define MY_DEF_COLOREDSEX		 0400000000
#define MY_DEF_SHOWHOROSCOPE	 01000000000
#define MY_DEF_NEWSTOP10		 02000000000

#define MY_NUMDEFINES 30

#define MY_VOTE_FLAG    0x1
#define MY_NOZAP_FLAG   0x2
#define MY_OUT_FLAG     0x4
#define MY_ANONY_FLAG   0x8

#define MY_FILE_READ  0x1
#define MY_FILE_OWND  0x2
#define MY_FILE_VISIT 0x4
#define MY_FILE_MARKED 0x8
#define MY_FILE_DIGEST 0x10
#define MY_FILE_NOREPLY 0x20
#define MY_FILE_NODELETE 0x40

#define MY_MAIL_READ       0x1
#define MY_MAIL_OWND       0x2
#define MY_MAIL_VISIT      0x4
#define MY_MAIL_MARKED     0x8
#define MY_MAIL_REPLIED    0x10
#define MY_MAIL_FORWARD    0x20

struct oldboardheader {          /* This structure is used to hold data in */
        char filename[STRLEN];   /* the BOARDS files */
        char owner[STRLEN - BM_LEN];
        char BM[ BM_LEN - 1];
        char flag;
        char title[STRLEN ];
        unsigned level;
        unsigned char accessed[ 12 ];
};

struct oldfileheader {          /* This structure is used to hold data in */
        char filename[STRLEN];     /* the DIR files */
		char owner[STRLEN - sizeof(time_t)];
		time_t posttime;
        char title[STRLEN];
        unsigned level;
        unsigned char accessed[ 12 ];   /* struct size = 256 bytes */
};

struct override {
	char id[13];
	char exp[40];
};

#define FAVORITES_NUM   20
typedef struct __Favorites
{
	char boards[FAVORITES_NUM][STRLEN];
	int num;
	int initialized;
} Favorites;

#endif
