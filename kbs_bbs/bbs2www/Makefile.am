EXTRA_DIST = README INSTALL include/bbslib.h include/types.h \
html/nologin.html html/sound/msg.wav html/browser.js \
html/bbsread.jsp html/bbslogout.jsp html/index.html \
html/bbs-bf.css html/images/edit.gif html/images/folder2.gif \
html/images/pic001.gif html/images/email.gif html/images/userlist.gif \
html/images/refresh.gif html/images/find.gif html/images/home.gif \
html/images/folder.gif html/images/replynow.gif html/images/back1.gif \
html/images/link0.gif html/images/closedfold.gif html/images/prethread.gif \
html/images/friend.gif html/images/smth.jpg html/images/telnet.gif \
html/images/link.gif html/func.js html/bbschat.css \
html/bbschat2.css html/bbs.css html/bbslogin.jsp \
html/frames.html
SUBDIRS = lib phplib src
CLEANFILES=@EXTRA_CLEAN@

install-data-local:
	cp -r html/* @HTMPATH@
	rm -rf @HTMPATH@/CVS
