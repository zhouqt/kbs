bindir=@CGIPATH@
bin_PROGRAMS = bbsusr bbsdoc bbscon bbsbrdadd bbsboa bbsall bbsanc bbs0an \
	  bbsleft bbsqry bbsnot  \
	  bbsfadd bbsfdel bbsfall bbsfriend bbsfoot bbspwd bbsplan bbsinfo \
	  bbsmybrd bbssig bbspst bbsgcon bbsgdoc bbsdel bbsdelmail bbsmailcon bbsmail \
	  bbsdelmsg bbssnd bbsnotepad bbsmsg bbssendmsg \
	  bbsmailmsg bbssndmail bbsnewmail bbspstmail bbsgetmsg bbschat bbssel bbscloak \
	  bbsmdoc bbsnick bbstfind bbstcon bbstdoc \
	  bbsccc bbsufind bbsclear bbsedit bbsman bbsparm \
	  bbsfwd bbsmnote bbsdenyall bbsdenydel bbsdenyadd \
	  bbsbfind bbsfav
AM_CFLAGS = -I../include -DINVISIBLE -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME -I../../src
LDADD = ../lib/.libs/libbbslib.a -L../../libBBS -lBBS -L../../libsystem -lsystem
