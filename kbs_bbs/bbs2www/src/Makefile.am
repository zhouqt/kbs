bindir=@CGIPATH@
bin_PROGRAMS = bbsbrdadd bbsall bbs0an \
	  bbspwd \
	  bbsmybrd \
	  bbssnd \
	  bbssndmail bbschat bbssel \
	  bbstfind bbstcon \
	  bbsccc bbsufind bbsclear bbsedit bbsman bbsparm \
	  bbsdenyall bbsdenydel bbsdenyadd
AM_CFLAGS = -I../include -DINVISIBLE -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME -I../../src 
LDADD = ../lib/.libs/libbbslib.a -L../../libBBS -lBBS -L../../libsystem -lsystem -L../lib -lbbslib
