lib_LTLIBRARIES = libworker.la
libworker_la_SOURCES = 	maingame.c screens.c gamedata.c gamedraw.c \
    gameinit.c gameplay.c gametty.c 
libdir=${prefix}/game/so
AM_CFLAGS = -DINVISIBLE -DENABLE_DL -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME -I$(top_builddir)/src
CLEANFILES=@EXTRA_CLEAN@
