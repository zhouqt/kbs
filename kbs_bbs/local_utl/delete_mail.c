#include "bbs.h"

static int cmpauthor(struct fileheader *fhdr, char *name)
{
	return !strcmp(fhdr->owner, name);
}

int delete_all_mail(int dfd, char *filename, int size, RECORD_FUNC_ARG fptr, void *farg)
{
    int i;
    char *buf, *buf1;
    off_t filesize;
	char *fname;
	char buf2[256];

    BBS_TRY {
        if (safe_mmapfile(filename, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &buf, &filesize, NULL) == 0)
            BBS_RETURN(0);
        for (i = 0, buf1 = buf; i < filesize / size; i++, buf1 += size) {
            if ((*fptr) (buf1, farg)) {
				fname = ((struct fileheader *)buf1)->filename;
				sprintf(buf2, "SMTH.DM%s", fname);
				rename(fname, buf2);
				continue;
            }
			write(dfd, buf1, size);
        }
		end_mmapfile((void *) buf, filesize, -1);
		BBS_RETURN(0);
    }
    BBS_CATCH {
    }
    BBS_END end_mmapfile((void *) buf, filesize, -1);

    return 0;
}
int main(int argc, char **argv)
{
	char mdir[256];
	char mdir_bak[256];
	char mdir_new[256];
	struct stat st;
	int fd;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s fromuser touser\n");
		return -1;
	}
	chdir(BBSHOME);
	setmailfile(mdir, argv[2], ".DIR");
	if (stat(mdir, &st) < 0)
		return -1;
	sprintf(mdir_bak, "%s.BAK", mdir);
	f_cp(mdir, mdir_bak, 0);
	sprintf(mdir_new, "%s.NEW", mdir);
	if ((fd = open(mdir_new, O_RDWR | O_CREAT, 0644)) > 0)
	{
		delete_all_mail(fd, mdir, sizeof(struct fileheader), cmpauthor, argv[1]);
		close(fd);
		f_mv(mdir_new, mdir);
	}
	return 0;
}

