bindir=@CGIPATH@
bin_PROGRAMS = bbsusr bbscon bbsbrdadd bbsall bbsanc bbs0an \
	  bbsleft bbsqry bbsnot  \
	  bbsfadd bbsfdel bbsfall bbsfriend bbspwd bbsplan bbsinfo \
	  bbsmybrd bbssig bbspst bbsgcon bbsdel bbsdelmail bbsmailcon bbsmail \
	  bbsdelmsg bbssnd bbsnotepad bbsmsg bbssendmsg \
	  bbsmailmsg bbssndmail bbsnewmail bbspstmail bbschat bbssel bbscloak \
	  bbsmdoc bbsnick bbstfind bbstcon bbstdoc \
	  bbsccc bbsufind bbsclear bbsedit bbsman bbsparm \
	  bbsfwd bbsmnote bbsdenyall bbsdenydel bbsdenyadd \
	  bbsbfind bbsfav
AM_CFLAGS = -I../include -DINVISIBLE -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME -I../../src 
LDADD = ../lib/.libs/libbbslib.a -L../../libBBS -lBBS -L../../libsystem -lsystem -L../lib -lbbslib
