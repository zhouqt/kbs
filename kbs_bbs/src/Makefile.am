bin_PROGRAMS = bbsd
EXTRA_PROGRAMS = 
bbsd_SOURCES=bbs.c record.c delete.c stuff.c mail.c register.c \
           xyz.c talk.c read.c comm_lists.c namecomplete.c chat.c \
           maintain.c edit.c more.c pass.c help.c bcache.c boards.c \
           screen.c userinfo.c vote.c announce.c sendmsg.c boards_t.c\
	   bm.c list.c goodbye.c bbsd_single.c lvars.c select.c libann.c\
	   newmain_single.c newio.c newterm.c convcode.c bbs_sendmail.c crypt.c \
	   ucache.c utmp.c log.c md5.c libmsg.c article.c sysconf.c site.c\
	   filter.c mgrep.c definekey.c default.c calltime.c super_filter.c libtmpl.c msg_manager.c\
		addr_manage.c
noinst_HEADERS = bbsconfig.h ann.h bbs.h chat.h defines_t.h edit.h func.h ioctl.h \
           md5.h modes.h permissions.h prototype.h screen.h struct.h system.h \
           systemexp.h uhashgen.h vars.h vote.h webmsg.h select.h default.h ansi.h output.h \
           calltime.h tmpl.h
LDADD=$(top_builddir)/libsystem/.libs/libsystem.a $(top_builddir)/rzsz/.libs/libzmodem.a
bbsd_LDFLAGS=-export-dynamic
AM_CFLAGS=-DBBSMAIN -DINVISIBLE -DSAFE_KILL -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME
CLEANFILES=@EXTRA_CLEAN@
