COMMERCIAL	=

RSAREFDEP	= @RSAREFDEP@

WRAPLIBS	= @WRAPLIBS@

SSH_INSTALL_MODE= @SSHINSTALLMODE@
transform	= @program_transform_name@

HOST_KEY_FILE 	= etc/ssh_host_key
HOST_CONFIG_FILE = etc/ssh_config
SERVER_CONFIG_FILE = etc/sshd_config

GMPLIBS         = -lgmp

noinst_HEADERS = cipher.h getput.h compress.h idea.h \
    randoms.h ssh_funcs.h includes.h \
    userfile.h blowfish.h des.h md5.h rsa.h version.h bufaux.h \
    mpaux.h servconf.h xmalloc.h buffer.h emulate.h packet.h ssh.h \
    deattack.h snprintf.h

SSHD_SRCS = sshd.c log-server.c canohost.c servconf.c tildexpand.c \
            serverloop.c rsa.c randoms.c buffer.c emulate.c packet.c \
            compress.c xmalloc.c bufaux.c authfile.c crc32.c rsaglue.c \
            cipher.c des.c match.c mpaux.c userfile.c signals.c \
            blowfish.c deattack.c idea.c auth-passwd.c

EXTRA_SRCS = memmove.c snprintf.c strerror.c remove.c random.c \
	putenv.c crypt.c socketpair.c

EXTRA_DIST= COPYING README README.SECURID README.TIS README.SECURERPC \
	README.CIPHERS README.DEATTACK INSTALL TODO libdes-README \
	libdes-COPYING libdes-ARTISTIC gnu-COPYING-GPL OVERVIEW \
	configure configure.ac config.guess config.sub Makefile.in \
	config.sample install-sh ChangeLog \
	idea.c

BBSD_SRCS = $(srcdir)/../src/bbs.c $(srcdir)/../src/delete.c $(srcdir)/../src/mail.c $(srcdir)/../src/register.c \
           $(srcdir)/../src/xyz.c $(srcdir)/../src/talk.c $(srcdir)/../src/comm_lists.c $(srcdir)/../src/namecomplete.c \
           $(srcdir)/../src/chat.c $(srcdir)/../src/maintain.c $(srcdir)/../src/edit.c $(srcdir)/../src/more.c $(srcdir)/../src/help.c \
           $(srcdir)/../src/screen.c $(srcdir)/../src/userinfo.c $(srcdir)/../src/vote.c $(srcdir)/../src/announce.c $(srcdir)/../src/sendmsg.c $(srcdir)/../src/boards_t.c\
           $(srcdir)/../src/bm.c $(srcdir)/../src/list.c $(srcdir)/../src/goodbye.c $(srcdir)/../src/bbsd_single.c $(srcdir)/../src/select.c \
           $(srcdir)/../src/newmain_single.c $(srcdir)/../src/newio.c $(srcdir)/../src/newterm.c \
           $(srcdir)/../src/definekey.c $(srcdir)/../src/calltime.c $(srcdir)/../src/tmpl.c $(srcdir)/../src/msg_manager.c\
           $(srcdir)/../src/addr_manage.c $(srcdir)/../src/personal_corp.c $(srcdir)/../src/newread.c $(srcdir)/../src/newhelp.c

CLEANFILES=@EXTRA_CLEAN@
bin_PROGRAMS = sshbbsd
CC="@LIBTOOL@"
CCLD="@CC@"
AM_MAKEFLAGS="@MAKEFLAG@"
sshbbsd_SOURCES = $(SSHD_SRCS) $(BBSD_SRCS)

sshbbsd_LDADD = ../libBBS/.libs/libBBS.a ../libsystem/.libs/libsystem.a ../rzsz/.libs/libzmodem.a $(GMPLIBS) $(WRAPLIBS) $(KERBEROS_LIBS) $(SSHLIBS) $(LIBS)
#sshbbsd_DEPENDENCIES = @CONFOBJS@ @SSHDCONFOBJS@ ../libsystem/.libs/libsystem.a ../rzsz/.libs/libzmodem.a 
AM_CFLAGS=-I. -I$(top_srcdir)/../src -DBBSMAIN -DSSHBBS -DHOST_KEY_FILE=\"$(HOST_KEY_FILE)\" -DHOST_CONFIG_FILE=\"$(HOST_CONFIG_FILE)\" -I.. -DSERVER_CONFIG_FILE=\"$(SERVER_CONFIG_FILE)\" -DSSH_PROGRAM=\"$(SSH_PROGRAM)\" -DETCDIR=\"$(etcdir)\" -DPIDDIR=\"$(piddir)\" -DSSH_BINDIR=\"$(bindir)\"
sshbbsd_LDFLAGS=-export-dynamic

# Creates /etc/ssh_host_key
generate-host-key:
	-@if test -f $(install_prefix)$(HOST_KEY_FILE); \
          then echo "You already have a host key in $(install_prefix)$(HOST_KEY_FILE)."; \
	  else \
	    umask 022; echo "Generating 1024 bit host key."; \
	    ssh-keygen -b 1024 -f $(install_prefix)$(HOST_KEY_FILE) -N ''; \
	  fi

# Creates install directories
make-dirs:
	-umask 022; if test '!' -d $(install_prefix)$(prefix); then \
	  mkdir $(install_prefix)$(prefix); fi; \
	if test '!' -d $(install_prefix)$(exec_prefix); then \
	  mkdir $(install_prefix)$(exec_prefix); fi; \
	if test '!' -d $(install_prefix)$(etcdir); then \
	  mkdir $(install_prefix)$(etcdir); fi; \
	if test '!' -d $(install_prefix)$(bindir); then \
	  mkdir $(install_prefix)$(bindir); fi; \
	if test '!' -d $(install_prefix)$(sbindir); then \
	  mkdir $(install_prefix)$(sbindir); fi; \
	if test '!' -d $(install_prefix)$(mandir); then \
	  mkdir $(install_prefix)$(mandir); fi; \
	if test '!' -d $(install_prefix)$(man1dir); then \
	  mkdir $(install_prefix)$(man1dir); fi; \
	if test '!' -d $(install_prefix)$(man8dir); then \
	  mkdir $(install_prefix)$(man8dir); fi

# This target can be used to install the host key and configuration files on
# other machines in a networked environment after the binaries have first
# been installed on one machine into a common binary directory.
hostinstall: $(PROGRAMS) make-dirs generate-host-key install-configs

install-configs:
	-if test '!' -f $(install_prefix)$(HOST_CONFIG_FILE); then \
	  $(INSTALL_DATA) -m 0644 $(srcdir)/host_config.sample \
	    $(install_prefix)$(HOST_CONFIG_FILE); fi
	-if test '!' -f $(install_prefix)$(SERVER_CONFIG_FILE); then \
	  cat $(srcdir)/server_config.sample | \
	  sed "s#_ETCDIR_#$(etcdir)#g" >/tmp/ssh_inst.$$$$; \
	  $(INSTALL_DATA) -m 0644 /tmp/ssh_inst.$$$$ \
	    $(install_prefix)$(SERVER_CONFIG_FILE); \
	  rm -f /tmp/ssh_inst.$$$$; fi
