bindir=@CGIPATH@
bin_PROGRAMS = bbsusr bbstop10 bbssec bbsdoc bbscon bbsbrdadd bbsboa bbsall bbsanc bbs0an \
	  bbslogout bbsleft bbsqry bbsnot  \
	  bbsfadd bbsfdel bbsfall bbsfriend bbsfoot bbspwd bbsplan bbsinfo \
	  bbsmybrd bbssig bbspst bbsgcon bbsgdoc bbsdel bbsdelmail bbsmailcon bbsmail \
	  bbsdelmsg bbssnd bbsnotepad bbsmsg bbssendmsg \
	  bbsmailmsg bbssndmail bbsnewmail bbspstmail bbsgetmsg bbschat bbssel bbscloak \
	  bbsmdoc bbsnick bbstfind bbstcon bbstdoc \
	  bbsccc bbsufind bbsclear bbsedit bbsman bbsparm \
	  bbsfwd bbsmnote bbsdenyall bbsdenydel bbsdenyadd \
	  bbsbfind bbsstyle
AM_CFLAGS = -I../include -DINVISIBLE -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME -I../../src
LDADD = ../lib/.libs/libbbslib.a -lBBS -lsystem -lbsd -lesmtp -lltdl -ldl
