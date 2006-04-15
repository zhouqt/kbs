noinst_HEADERS = gamedata.h  gamedefs.h  gamedraw.h  gameinit.h  gameplay.h  gametty.h  screens.h
lib_LTLIBRARIES = libworker.la
libworker_la_SOURCES = 	maingame.c screens.c gamedata.c gamedraw.c \
    gameinit.c gameplay.c gametty.c 
libdir=${prefix}/game/so
AM_CFLAGS = -DENABLE_DL -I$(top_builddir)/src -Wall
CLEANFILES=@EXTRA_CLEAN@
