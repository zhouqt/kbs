EXTRA_DIST=crontab.SMTH
bin_PROGRAMS = poststat static usage Xclean Xhtml Bhtml refriend chlevel listonline \
	showBM idx0Ann lazybm fixdir autoundeny fixwebmsg login statBM gen_title \
	resetMailTotal
EXTRA_PROGRAMS = 
LDADD=-lBBS -lsystem -L../libBBS -L../libsystem
AM_CFLAGS=-DINVISIBLE -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME -I$(top_builddir)/src
CLEANFILES=@EXTRA_CLEAN@
