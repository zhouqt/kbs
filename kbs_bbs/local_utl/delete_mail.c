#include "bbs.h"

static int cmpauthor(struct fileheader *fhdr, char *name)
{
	return !strcmp(fhdr->owner, name);
}

int delete_all_mail(int dfd, char *touser, char *filename, int size, RECORD_FUNC_ARG fptr, void *farg)
{
    int i;
    char *buf, *buf1;
    off_t filesize;
	char *fname;
	char buf2[80];
	char from[256];
	char to[256];
	struct stat st;
	int subspace = 0;
	struct userec *to_userec;

	printf("Deleting mails of %s... \n", touser);
    BBS_TRY {
        if (safe_mmapfile(filename, O_RDONLY, PROT_READ, MAP_SHARED, &buf, &filesize, NULL) == 0)
            BBS_RETURN(0);
        for (i = 0, buf1 = buf; i < filesize / size; i++, buf1 += size) {
            if ((*fptr) (buf1, farg)) {
				fname = ((struct fileheader *)buf1)->filename;
				sprintf(buf2, "SMTH.DM%s", fname);
				setmailfile(from, touser, fname);
				setmailfile(to, touser, buf2);
				rename(from, to);
				printf("found one %s...\n",fname);
				if (lstat(to, &st) == 0 && S_ISREG(st.st_mode) && st.st_nlink == 1) {
					subspace += st.st_size;
				}
				continue;
			}
			write(dfd, buf1, size);
		}
		end_mmapfile((void *) buf, filesize, -1);
		if (getuser(touser, &to_userec) != 0) {
			if (to_userec->usedspace > subspace)
				to_userec->usedspace -= subspace;
			else
				to_userec->usedspace = 0;
		}
		BBS_RETURN(0);
	}
	BBS_CATCH {
	}
	BBS_END;
    end_mmapfile((void *) buf, filesize, -1);

	return 0;
}

int main(int argc, char **argv)
{
	char mdir[256];
	char mdir_bak[256];
	char mdir_new[256];
	struct stat st;
	int fd;
	char touser[20];
	int len;

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s fromuser\n", argv[0]);
		return -1;
	}
	chdir(BBSHOME);
	resolve_ucache();
	while(!feof(stdin))
	{
		if (fgets(touser, sizeof(touser) - 1, stdin) == NULL)
			return -1;
		touser[sizeof(touser) - 1] = '\0';
		len = strlen(touser);
		if (touser[len - 1] == '\n')
			touser[len - 1] = '\0';
		setmailfile(mdir, touser, ".DIR");
		if (stat(mdir, &st) < 0)
		{
			fprintf(stderr, "User '%s' not found.\n", touser);
			continue;
		}
		sprintf(mdir_bak, "%s.BAK", mdir);
		f_cp(mdir, mdir_bak, 0);
		sprintf(mdir_new, "%s.NEW", mdir);
		if ((fd = open(mdir_new, O_RDWR | O_CREAT, 0644)) > 0)
		{
			delete_all_mail(fd, touser, mdir, sizeof(struct fileheader), 
								cmpauthor, argv[1]);
			close(fd);
			f_mv(mdir_new, mdir);
		}
	}
	return 0;
}

