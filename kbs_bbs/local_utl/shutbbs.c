#include <windows.h>
#include <sys/cygwin.h>
#include "bbs.h"

void kill_process(const char *prog_name)
{
	struct external_pinfo *p;
	cygwin_getinfo_types query = CW_GETPINFO;
	char *s;
	char pname[MAX_PATH];
	int pid;
	
	for (pid = 0;
		(p = (struct external_pinfo *)cygwin_internal(query, pid | CW_NEXTPID));
		pid = p->pid)
	{
		pname[0] = '\0';
		cygwin_conv_to_posix_path(p->progname, pname);
		s = strchr(pname, '\0') - 4;
		if (s > pname && strcasecmp(s, ".exe") == 0)
			*s = '\0';
		if (strcasecmp(pname, prog_name) == 0)
			kill(p->pid, SIGTERM);
	}
}

int main()
{
	cygwin_internal(CW_LOCK_PINFO, 1000);
	kill_process(BBSHOME "/bin/innbbsd");
	kill_process(BBSHOME "/bin/sshbbsd");
	kill_process(BBSHOME "/bin/bbsd");
	kill_process(BBSHOME "/bin/bbslogd");
	kill_process(BBSHOME "/bin/miscd");
	kill_process("/usr/bin/ipc-daemon2");
	cygwin_internal(CW_UNLOCK_PINFO);

	return 0;
}
