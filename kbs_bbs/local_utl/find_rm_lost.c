//查找和删除丢失到board目录下的文章     ylsdd   2001/6/16

#include "bbs.h"

#define MAXFILE 10000
#define MINAGE 50000		//at least 50000 sec old
#ifdef NUMBUFFER
#undef NUMBUFFER
#endif
#define NUMBUFFER 100
#define HASHSIZE 50

char *(spcname[]) = {
"deny_users", "read_club_users", "write_club_users", NULL};

int nfile[HASHSIZE];
char allpost[HASHSIZE][MAXFILE][20];
int refcount[HASHSIZE][MAXFILE];
char otherfile[200];
int allfile = 0, allref = 0, alllost = 0, unknownfn = 0, nindexitem = 0,
    nstrangeitem = 0;

int
hash(char *postname)
{
	int i = atoi(postname + 2);
	return i % HASHSIZE;
}

int
ispostfilename(char *file)
{
	if (strncmp(file, "M.", 2) && strncmp(file, "G.", 2)
	    &&strncmp(file, "D.", 2) && strncmp(file, "J.", 2))
		return 0;
	if (!isdigit(file[3]))
		return 0;
	if (strlen(file) >= 20)
		return 0;
	return 1;
}

int
isspcname(char *file)
{
	int i;
	for (i = 0; spcname[i] != NULL; i++)
		if (!strcmp(file, spcname[i]))
			return 1;
	return 0;
}

int
countfile(struct fileheader *fhdr, int index, void *farg)
{
	int i, h;
	char *fname = fhdr->filename;
	nindexitem++;
	h = hash(fname);
	for (i = 0; i < nfile[h]; i++) {
		if (strcmp(allpost[h][i], fname))
			continue;
		refcount[h][i]++;
		break;
	}
	return 0;
}

int totalsize = 0;
int
testPOWERJUNK(char *path, char *fn)
{
	char buf[1024];
	sprintf(buf, "%s/%s", path, fn);
	if (strncmp(fn, ".AUTHOR", 7) && strncmp(buf, ".TITLE", 6)
	    &&strncmp(fn,".ORIGIN",6))
		return 0;
	if (time(NULL) - atoi(fn+2) < 3600 * 5)
		return 0;
	unlink(buf);
	printf("%s\n", buf);
	return 0;
}

int
getallpost(char *path)
{
	DIR *dirp;
	struct dirent *direntp;
	int h;
	dirp = opendir(path);
	if (dirp == NULL)
		return -1;
	while ((direntp = readdir(dirp)) != NULL) {
		if (direntp->d_name[0] == '.') {
			testPOWERJUNK(path, direntp->d_name);
			continue;
		}
		if (ispostfilename(direntp->d_name)) {
			h = hash(direntp->d_name);
			if (nfile[h] >= MAXFILE) {
				printf("啊?!");
				exit(0);
			}
			refcount[h][nfile[h]] = 0;
			strcpy(allpost[h][nfile[h]++], direntp->d_name);
			continue;
		}
		if (isspcname(direntp->d_name))
			continue;
		unknownfn++;
		if (strlen(otherfile) + strlen(direntp->d_name) + 1 <
		    sizeof (otherfile)) {
			strcat(otherfile, " ");
			strcat(otherfile, direntp->d_name);
		}
	}
	closedir(dirp);
	return 0;
}

int
useindexfile(char *filename)
{
	return apply_record(filename, (APPLY_FUNC_ARG)countfile, sizeof (struct fileheader),
				NULL,false,false);
}

int
rm_lost(char *path)
{
	int i, h, total, totalref, lost, t;
	char buf[MAXPATH];
	for (h = 0, total = 0, totalref = 0, lost = 0; h < HASHSIZE; h++) {
		total += nfile[h];
		for (i = 0; i < nfile[h]; i++) {
			totalref += refcount[h][i];
			if (!refcount[h][i]) {
				lost++;
				t = atoi(allpost[h][i] + 2);
				if (time(NULL) - t < MINAGE) {
					//printf("Too young to die, %d %d\n", t, time(NULL));
					continue;
				}
				sprintf(buf, "%s/%s", path, allpost[h][i]);
				unlink(buf);
			}
		}
	}
	allfile += total;
	allref += totalref;
	alllost += lost;
	printf(" total %d, refcount %d, %d file(s) was lost\n", total, totalref,
	       lost);
	if (strlen(otherfile) > 0)
		printf("%s\n", otherfile);
	return 0;
}

int
find_rm_lost(struct boardheader *bhp)
{
	char buf[200];
	int i;
	for (i = 0; i < HASHSIZE; i++) {
		nfile[i] = 0;
	}
	otherfile[0] = 0;
	sprintf(buf, "boards/%s", bhp->filename);
	if (getallpost(buf) < 0)
		return -1;
	sprintf(buf, "boards/%s/.DIR", bhp->filename);
	if (dashf(buf))
		if (useindexfile(buf) < 0)
			return -1;
	sprintf(buf, "boards/%s/.DIGEST", bhp->filename);
	if (dashf(buf))
		if (useindexfile(buf) < 0)
			return -1;
	sprintf(buf, "boards/%s/.DELETED", bhp->filename);
	if (dashf(buf))
		if (useindexfile(buf) < 0)
			return -1;
	sprintf(buf, "boards/%s/.JUNK", bhp->filename);
	if (dashf(buf))
		if (useindexfile(buf) < 0)
			return -1;
	sprintf(buf, "boards/%s", bhp->filename);
	rm_lost(buf);
	return 0;
}

int
main()
{
	chdir(BBSHOME);
	resolve_boards();
	
	printf("find_rm_lost is running~\n");
	apply_boards(find_rm_lost);
	printf("allfile %d, allref %d, alllost %d\n", allfile, allref, alllost);
	printf("unknownfn %d, nindexitem %d, nstrangeitem %d\n", unknownfn,
	       nindexitem, nstrangeitem);
	printf("totalsize = %d", totalsize);
	return 0;
}

