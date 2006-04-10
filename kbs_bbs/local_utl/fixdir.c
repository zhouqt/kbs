#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include "bbs.h"
extern int errno;

int mailmode;
struct fileheader data[20000];
int len = 0;
extern const char alphabet[];


int cmpfile(const void *f11, const void *f21)
{
    time_t t1, t2;
    struct fileheader *f1, *f2;
    f1 = (struct fileheader*)f11;
    f2 = (struct fileheader*)f21;
    
    t1 = get_posttime(f1);
    t2 = get_posttime(f2);
    return t1 - t2;
}

void insertfile(fh)
struct fileheader fh;
{
    data[len] = fh;
    len++;
/*
   int i,j;
   for (i=0;i<len;i++)
   {
        time_t t1,t2;
        t1=atoi(fh.filename+2);
	t2=atoi(data[i].filename+2);
	if (t1<t2) break;
   }
   for (j=len;j<i;j--)
	data[j]=data[j-1];	
   data[i]=fh;
   len++;
*/
}

extern char *optarg;
extern int optind, opterr, optopt;

int print_usage(char *prog)
{
    printf("\
Usage: %s [OPTION]... DIR\n\
	-m fix mail mode\n\
	-t test mode. generate .tmpfile only\n\
        -h show this message\n\
examples:\n\
	To fix a user's mail directory:\n\
		%s -m /home/bbs/mail/K/kxn\n\
notice:\n\
	In mailmode it will set all the mail readed.\n\
", prog, prog);
    return 0;
}


int addfile(const char* filename){
    struct fileheader fh;
    FILE *art;
    if ((strcmp(filename, ".DIR"))
        && (strcmp(filename, "."))
        && (strcmp(filename, ".."))
        && (((filename[0] == 'M')&&(filename[1]=='.')) || (filename[2]=='M'))) {
        struct stat st;
        char buf1[256], buf2[256];
    
        if (stat(filename, &st)) {
            return 0;
	}
        if ((art = fopen(filename, "r")) != NULL) {
            char *p;
    
            bzero(&fh, sizeof(fh));
            fgets(buf1, 256, art);
            if (buf1 == 0) {
		fclose(art);
                return 0;
            }
            p = strchr(buf1 + 8, ' ');
            if (p)
                *p = 0;
            /* etnlegend, 2006.04.07, 这地方用 strpbrk 比较好... */
            if((p=strpbrk(buf1+8,"(\n"))!=NULL)
                *p=0;
            strncpy(fh.owner, buf1 + 8, OWNER_LEN);
            fh.owner[OWNER_LEN-1]=0;
            fgets(buf2, 256, art);
            if (buf2 == 0) {
		fclose(art);
                return 0;
            }
            /* etnlegend, 2006.04.07, 这地方用 strpbrk 比较好... */
            if((p=strpbrk(buf2+8,"\n\r"))!=NULL)
                *p=0;
            strcpy(fh.filename, filename);
            strnzhcpy(fh.title, buf2 + 8, ARTICLE_TITLE_LEN);
            if (mailmode) {
                fh.accessed[0] |= FILE_READ;
            }
            if (strncmp(buf1, "发信站", 6) && strncmp(buf1, "寄信人: ", 8) && strncmp(buf1, "发信人: ", 8)) {
		fclose(art);
                return 0;
            }
            if ((strncmp(buf2, "标  题: ", 8))
                && (strncmp(buf2, "标　题: ", 8))) {
		fclose(art);
                return 0;
            }
		set_posttime(&fh);
            insertfile(fh);
            fclose(art);
        }
    }
    return 0;
}

int
ispostfilename(char *file)
{
		if (strncmp(file, "M.", 2) && strncmp(file, "G.", 2)
					    &&strncmp(file, "D.", 2) && strncmp(file, "J.", 2) && strncmp(file, "Z.", 2))
					return 0;
			if (!isdigit(file[3]))
						return 0;
				if (strlen(file) >= 20)
							return 0;
					return 1;
}


int getallpost(char *path,char prefix){
	DIR *dirp;
	struct dirent *direntp;
	dirp = opendir(path);
	if (dirp == NULL)
		return -1;
	while ((direntp = readdir(dirp)) != NULL) {
		if (direntp->d_name[0] == '.') {
			continue;
		}
#ifdef SMTH
		if( prefix == NULL && strlen(direntp->d_name)==1 && strchr( alphabet, direntp->d_name[0] ) ){
			char buf[200];
			sprintf(buf, "%s/%c", path, direntp->d_name[0] );
			getallpost(buf, direntp->d_name[0]);
			continue;
		}
#endif
		if (ispostfilename(direntp->d_name)) {
			char buf[200];
			if (prefix!=0) 
			sprintf(buf, "%c/%s", prefix, direntp->d_name );
			else
			sprintf(buf, "%s", direntp->d_name );
                    addfile(buf);
			continue;
		}
	}
	closedir(dirp);
	return 0;
}

int main(int argc, char **argv){
    DIR *pdir;
    char *name;
    int i,file,flag=1;
    mode_t old_dir_mode;
    char cwd[255];

    mailmode = 0;
    while (1) {
        int c;

        c = getopt(argc, argv, "mth");
        if (c == -1)
            break;
        ;
        switch (c) {
        case 'm':
            mailmode = 1;
            break;
        case 't':
            flag = 0;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        case '?':
            printf("%s:Unknown argument.\nTry `%s -h' for more information.\n", argv[0], argv[0]);
            return 0;
        default:
            printf("?? getopt returned character code 0%o ??\n", c);
            return 0;
        }
    }

    if (optind < argc) {
        struct stat st;

        name = argv[optind++];
        if (optind < argc) {
            printf("%s:Too many arguments.\nTry `%s -h' for more information.\n", argv[0], argv[0]);
            return 0;
        }
        if (stat(name, &st) == -1) {
            perror("Can't stat directory:");
            return 0;
        }
        if (!S_ISDIR(st.st_mode)) {
            printf("%s isn't directory:", name);
            return 0;
        }
        old_dir_mode = st.st_mode;
    } else
        return print_usage(argv[0]);

    if (!flag)
        chmod(name, 0744);
    pdir = opendir(name);
    getcwd(cwd, 255);
    chdir(name);
    file = open(".tmpfile", O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if (file == 0) {
        perror("open .tmpfile error!\n");
        closedir(pdir);
        return -1;
    }
    i = 1;
    getallpost(name,NULL);
    qsort(data, len, sizeof(struct fileheader), cmpfile);
    printf("end.len=%d %d", len, len * sizeof(struct fileheader));
    if (write(file, data, len * sizeof(struct fileheader)) == -1)
        perror("write error");
    if (close(file))
        printf("close error=%d\n", errno);
    closedir(pdir);
    if (!flag) {
        struct stat st;
        int restore = 0;
        struct utimbuf ut;

        if (stat(".DIR", &st) != -1)
            restore = 1;
        printf("\nwrite .DIR ok=%d\n", f_mv(".tmpfile", ".DIR"));
        if (restore) {
            chmod(".DIR", st.st_mode);
            ut.actime = st.st_atime;
            ut.modtime = st.st_mtime;
            utime(".DIR", &ut);
        }
    }
    chdir(cwd);
    if (!flag)
        chmod(name, old_dir_mode);
    return 0;
}

