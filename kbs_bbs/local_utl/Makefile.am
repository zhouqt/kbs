CC="@LIBTOOL@"
CCLD="@CC@"
bin_PROGRAMS = poststat static usage Xclean Xhtml Bhtml refriend listonline \
	showBM idx0Ann lazybm fixdir autoundeny login statBM gen_title \
	resetMailTotal cnv_brdflag find_rm_lost \
	cnv_bh statguy cnv_bh2 autoclear ch_brdflag statkiller showipdeny \
	convpass356 scanIP scanMODE listMODE cnv_register calc_effsize smth_usage change_tmpl \
	mailall settoptitle genhot bonlinelog uonlinelog dayuserlog alluserlog \
	cnv_passwd strip_fh gennewboard recalc_signum find_lost_user outdaybm \
	cnv_brddir
	
EXTRA_DIST = urlencode.c
EXTRA_PROGRAMS = 
LDADD=-lBBS -lsystem -L../libBBS -L../libsystem
AM_CFLAGS=-DINVISIBLE -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME -I$(top_builddir)/src
CLEANFILES=@EXTRA_CLEAN@
