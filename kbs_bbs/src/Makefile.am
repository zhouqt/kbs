bin_PROGRAMS = bbsd
EXTRA_PROGRAMS = 
bbsd_SOURCES=bbs.c record.c delete.c stuff.c mail.c register.c \
           xyz.c talk.c read.c comm_lists.c namecomplete.c chat.c \
           maintain.c edit.c more.c pass.c help.c bcache.c boards.c \
           screen.c userinfo.c vote.c announce.c sendmsg.c boards_t.c\
	   bm.c list.c goodbye.c fileshm.c bbsd_single.c lvars.c\
	   newmain_single.c newio.c newterm.c convcode.c bbs_sendmail.c crypt.c \
	   ucache.c utmp.c log.c md5.c libmsg.c article.c sysconf.c site.c
LDADD=../libsystem/libsystem.a ../rzsz/libzmodem.a -lbsd -lesmtp -lltdl -ldl
CFLAGS=-DBBSMAIN -DINVISIBLE -DSAFE_KILL -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME
