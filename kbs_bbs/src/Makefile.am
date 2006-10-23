bin_PROGRAMS = bbsd
EXTRA_PROGRAMS = 
CC="@LIBTOOL@"
CCLD="@CC@"
bbsd_SOURCES=bbs.c delete.c mail.c register.c \
           xyz.c talk.c comm_lists.c namecomplete.c chat.c \
           maintain.c edit.c more.c help.c \
           screen.c userinfo.c vote.c announce.c sendmsg.c boards_t.c\
	   bm.c list.c goodbye.c bbsd_single.c select.c \
	   newmain_single.c newio.c newterm.c \
	   definekey.c calltime.c tmpl.c msg_manager.c\
	   addr_manage.c personal_corp.c newread.c newhelp.c
noinst_HEADERS = ann.h bbs.h chat.h defines_t.h edit.h func.h \
           md5.h modes.h screen.h struct.h system.h comm_lists.h \
           systemexp.h uhashgen.h vars.h vote.h webmsg.h select.h ansi.h output.h \
           calltime.h tmpl.h read.h service.h var.h
LDADD=$(top_builddir)/libBBS/.libs/libBBS.a $(top_builddir)/libsystem/.libs/libsystem.a \
      $(top_builddir)/rzsz/.libs/libzmodem.a
bbsd_LDFLAGS=-export-dynamic
AM_CFLAGS=-DBBSMAIN
CLEANFILES=@EXTRA_CLEAN@
