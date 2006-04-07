CC="@LIBTOOL@"
CCLD="@CC@"
bin_PROGRAMS = poststat static usage Xhtml Bhtml listonline \
	showBM idx0Ann lazybm fixdir autoundeny statBM gen_title \
	find_rm_lost statguy autoclear showipdeny \
	scanIP scanMODE listMODE smth_usage \
	mailall genhot bonlinelog uonlinelog dayuserlog alluserlog \
	gennewboard outdaybm \
	post_brdlog post_buse newpoststat genbplot sync_mailsize

EXTRA_DIST = urlencode.c
EXTRA_PROGRAMS = 
LDADD=-lBBS -lsystem -L../libBBS -L../libsystem
AM_CFLAGS=-DINVISIBLE -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME -I$(top_builddir)/src
CLEANFILES=@EXTRA_CLEAN@
