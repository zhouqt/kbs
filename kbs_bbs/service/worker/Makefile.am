noinst_HEADERS = gamedata.h  gamedefs.h  gamedraw.h  gameinit.h  gameplay.h  gametty.h  screens.h
lib_LTLIBRARIES = libworker.la
libworker_la_SOURCES = 	maingame.c screens.c gamedata.c gamedraw.c \
    gameinit.c gameplay.c gametty.c 
libdir=${prefix}/game/so
AM_CFLAGS = -DENABLE_DL -DBBSMAIN -I$(top_srcdir)/src
CLEANFILES=@EXTRA_CLEAN@
