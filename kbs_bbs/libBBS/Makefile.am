noinst_LIBRARIES = libBBS.a
libBBS_a_SOURCES = record.c log.c ucache.c stuff.c md5.c crypt.c pass.c bcache.c utmp.c \
	lvars.c site.c boards.c lvars.c bbs_sendmail.c libmsg.c article.c convcode.c \
	sysconf.c libann.c
AM_CFLAGS = -DINVISIBLE -DSAFE_KILL -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME -I$(top_builddir)/src
libBBS_a_AR=$(CONFIG_AR) cru
CLEANFILES=@EXTRA_CLEAN@
