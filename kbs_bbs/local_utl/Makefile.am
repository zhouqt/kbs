bin_PROGRAMS = poststat static usage Xclean Xhtml Bhtml refriend chlevel listonline \
	showBM idx0Ann lazybm fixdir autoundeny fixwebmsg
EXTRA_PROGRAMS = 
LDADD=../.libs/libBBS.a ../.libs/libsystem.a ../.libs/libzmodem.a -lbsd -lesmtp -lltdl -ldl
AM_CFLAGS=-DINVISIBLE -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME  -I$(top_builddir)/src
CLEANFILES=@EXTRA_CLEAN@
