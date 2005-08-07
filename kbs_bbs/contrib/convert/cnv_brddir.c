#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bbs.h"

#define BOARDSFILE "/home/bbs/.BOARDS"
#define BBS_HOME    "/home/bbs"

extern char alphabet[];

typedef struct __Node
{
	char filename[20];
	char directory;
	struct __Node * next;
} Node;

typedef struct __HTElement
{
	int count;
	Node * head;
} HTElement;

#define HASH_SIZE 65536

static HTElement htable[HASH_SIZE];

int hash_function(char * filename)
{
	int val;

	val = atoi(filename+2) % HASH_SIZE;

	return val;
}

int insert_node(Node * node)
{
	int val;
	Node * ptr;
	Node * oldptr;
	int i;

	if (node == NULL)
		return -1;	// error

	val = hash_function(node->filename);

	i = 0;
	oldptr = ptr = htable[val].head;
	do
	{
		if (ptr == NULL)	// no node in the linked list
		{
			node->next = NULL;
			ptr = htable[val].head = node;
			break;
		}

		if (strcmp(ptr->filename, node->filename) < 0)
		{
			if (i == 0)	// the node must be inserted at first position
			{
				node->next = htable[val].head;
				htable[val].head = node;
				break;
			}
			else
			{
				node->next = ptr;
				oldptr->next = node;
				break;
			}
		}
		oldptr = ptr;
		ptr = ptr->next;
		i++;
	} while (ptr != NULL);

	if (ptr == NULL)	// at the end of linked list
	{
		oldptr->next = node;
		node->next = NULL;
	}

	htable[val].count ++;

	return 0;
};

void destroy_list()
{
	int i;
	Node * ptr;
	Node * oldptr;

	for (i = 0; i < HASH_SIZE; i++)
	{
		if (htable[i].head == NULL)
			continue;

		ptr = htable[i].head;
		while (ptr != NULL)
		{
			oldptr = ptr;
			ptr = ptr->next;
			free(oldptr);
		}
		htable[i].head = NULL;
		htable[i].count = 0;
	}
}

Node * search(char * filename)
{
	int val;
	Node * ptr;

	val = hash_function(filename);
	for (ptr = htable[val].head; ptr != NULL; ptr = ptr->next)
	{
		if (strcmp(ptr->filename + 1, filename + 1) == 0)
			return ptr;
	}

	return NULL;
}

static int build_dir(const char *boardname)
{
	char buff[256];
	int i;
	
	snprintf(buff, sizeof(buff), "%s/boards/%s", BBS_HOME, boardname);
	chdir(buff);
    for (i = 0; i < 52; i++)
    {
        snprintf(buff, sizeof(buff), "%c", alphabet[i]);
        mkdir(buff, 0755);
    }
}

static int convert_normal(const char *boardname)
{
	char buff[256];
	int fd;
	struct fileheader *fh;
	struct fileheader *fhptr;
	struct stat fs;
	int records;
	int i, j, k, rn;
	Node * ptr;

	snprintf(buff, sizeof(buff), "%s/boards/%s", BBS_HOME, boardname);
	chdir(buff);
	///////////// Convert .DIR file ///////////// 

	if ((fd = open(".DIR", O_RDWR)) == -1)
	{
		perror("open .DIR");
		return -1;
	}

	printf("Converting .DIR file ...");
	fflush(stdout);

	fstat(fd, &fs);
	records = fs.st_size / sizeof(struct fileheader);
	fh = mmap(NULL, fs.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (fh == MAP_FAILED)
	{
		perror("mmap");
		close(fd);
		return -1;
	}
	k = 0;
	// read every record in sequence
	for (i = 0; i < records; i++)
	{
		fhptr = fh + i;
        rn = 0 + (int) (52.0 * rand() / (RAND_MAX + 1.0));
		if (!(fhptr->filename[0] != '\0' && fhptr->filename[1] == '.'))
			continue;
		if (fhptr->accessed[0] & FILE_DIGEST)
		{
			ptr = (Node *)malloc(sizeof(Node));
			if (ptr == NULL)
			{
				perror("malloc");
				destroy_list();
				printf(" Failed.\n");
				close(fd);
				munmap(fh, fs.st_size);
				return -2;
			}
			strncpy(ptr->filename, fhptr->filename, sizeof(ptr->filename));
			ptr->directory = alphabet[rn];
			ptr->next = NULL;
			insert_node(ptr);
		}

		fhptr->posttime = atoi(fhptr->filename + 2);
        snprintf(buff, sizeof(buff), "%c/%s", alphabet[rn], fhptr->filename);
        rename(fhptr->filename, buff);
        strncpy(fhptr->filename, buff, sizeof(fhptr->filename));
		k++;
	}
	close(fd);
	munmap(fh, fs.st_size);
	printf(" Done.\n");
	
	///////////// Convert .DIGEST file ///////////// 
	if ((fd = open(".DIGEST", O_RDWR)) == -1)
	{
		printf("No .DIGEST file.\n");
		return 1;
	}

	printf("Converting .DIGEST file ...");
	fflush(stdout);

	fstat(fd, &fs);
	records = fs.st_size / sizeof(struct fileheader);
	fh = mmap(NULL, fs.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (fh == MAP_FAILED)
	{
		perror("mmap");
		close(fd);
		return -1;
	}
	for (i = 0; i < records; i++)
	{
		fhptr = fh + i;
		if (!(fhptr->filename[0] != '\0' && fhptr->filename[1] == '.'))
			continue;
		if ((ptr = search(fhptr->filename)) == NULL)
		{
        	rn = 0 + (int) (52.0 * rand() / (RAND_MAX + 1.0));
        	snprintf(buff, sizeof(buff), "%c/%s", alphabet[rn], fhptr->filename);
		}
		else
			snprintf(buff, sizeof(buff), "%c/%s", ptr->directory, fhptr->filename);

		fhptr->posttime = atoi(fhptr->filename + 2);
        rename(fhptr->filename, buff);
        strncpy(fhptr->filename, buff, sizeof(fhptr->filename));
	}
	close(fd);
	munmap(fh, fs.st_size);
	printf(" Done.\n");

	///////////// Collision Summary ///////////// 

	j = 0;
	for (i = 0; i < HASH_SIZE; i++)
	{
		if (htable[i].count > 1)
			j++;
	}
	printf("  Collision: %4d    Total:    %4d     Ratio: %.2f\n\n",
			j, k, j * 1.0 / k);

	destroy_list();

	return 0;
}

static int convert_delete(const char *boardname)
{
	char buff[256];
	int fd;
	struct fileheader *fh;
	struct fileheader *fhptr;
	struct stat fs;
	int records;
	int i, j, k, rn;
	Node * ptr;

	snprintf(buff, sizeof(buff), "%s/boards/%s", BBS_HOME, boardname);
	chdir(buff);
	///////////// Convert .DELETED file ///////////// 

	if ((fd = open(".DELETED", O_RDWR)) == -1)
	{
		perror("open .DELETED");
		return -1;
	}

	printf("Converting .DELETED file ...");
	fflush(stdout);

	fstat(fd, &fs);
	records = fs.st_size / sizeof(struct fileheader);
	fh = mmap(NULL, fs.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (fh == MAP_FAILED)
	{
		perror("mmap");
		close(fd);
		return -1;
	}
	k = 0;
	// read every record in sequence
	for (i = 0; i < records; i++)
	{
		fhptr = fh + i;
        rn = 0 + (int) (52.0 * rand() / (RAND_MAX + 1.0));
		if (fhptr->filename[0] != '\0' && fhptr->filename[1] == '.')
		{
			fhptr->posttime = atoi(fhptr->filename + 2);
			snprintf(buff, sizeof(buff), "%c/%s", alphabet[rn], fhptr->filename);
			rename(fhptr->filename, buff);
			strncpy(fhptr->filename, buff, sizeof(fhptr->filename));
		}
	}
	close(fd);
	munmap(fh, fs.st_size);
	printf(" Done.\n");

	return 0;
}

static int convert_junk(const char *boardname)
{
	char buff[256];
	int fd;
	struct fileheader *fh;
	struct fileheader *fhptr;
	struct stat fs;
	int records;
	int i, j, k, rn;
	Node * ptr;

	snprintf(buff, sizeof(buff), "%s/boards/%s", BBS_HOME, boardname);
	chdir(buff);
	///////////// Convert .DELETED file ///////////// 

	if ((fd = open(".JUNK", O_RDWR)) == -1)
	{
		perror("open .JUNK");
		return -1;
	}

	printf("Converting .JUNK file ...");
	fflush(stdout);

	fstat(fd, &fs);
	records = fs.st_size / sizeof(struct fileheader);
	fh = mmap(NULL, fs.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (fh == MAP_FAILED)
	{
		perror("mmap");
		close(fd);
		return -1;
	}
	k = 0;
	// read every record in sequence
	for (i = 0; i < records; i++)
	{
		fhptr = fh + i;
        rn = 0 + (int) (52.0 * rand() / (RAND_MAX + 1.0));
		if (fhptr->filename[0] != '\0' && fhptr->filename[1] == '.')
		{
			fhptr->posttime = atoi(fhptr->filename + 2);
			snprintf(buff, sizeof(buff), "%c/%s", alphabet[rn], fhptr->filename);
			rename(fhptr->filename, buff);
			strncpy(fhptr->filename, buff, sizeof(fhptr->filename));
		}
	}
	close(fd);
	munmap(fh, fs.st_size);
	printf(" Done.\n");

	return 0;
}

static int convert_ding(const char *boardname)
{
	char buff[256];
	int fd;
	struct fileheader *fh;
	struct fileheader *fhptr;
	struct stat fs;
	int records;
	int i, j, k, rn;
	Node * ptr;

	snprintf(buff, sizeof(buff), "%s/boards/%s", BBS_HOME, boardname);
	chdir(buff);
	///////////// Convert .DELETED file ///////////// 

	if ((fd = open(".DINGDIR", O_RDWR)) == -1)
	{
		perror("open .DINGDIR");
		return -1;
	}

	printf("Converting .DINGDIR file ...");
	fflush(stdout);

	fstat(fd, &fs);
	records = fs.st_size / sizeof(struct fileheader);
	fh = mmap(NULL, fs.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (fh == MAP_FAILED)
	{
		perror("mmap");
		close(fd);
		return -1;
	}
	k = 0;
	// read every record in sequence
	for (i = 0; i < records; i++)
	{
		fhptr = fh + i;
        rn = 0 + (int) (52.0 * rand() / (RAND_MAX + 1.0));
		if (fhptr->filename[0] != '\0' && fhptr->filename[1] == '.')
		{
			fhptr->posttime = atoi(fhptr->filename + 2);
			snprintf(buff, sizeof(buff), "%c/%s", alphabet[rn], fhptr->filename);
			rename(fhptr->filename, buff);
			strncpy(fhptr->filename, buff, sizeof(fhptr->filename));
		}
	}
	close(fd);
	munmap(fh, fs.st_size);
	printf(" Done.\n");

	return 0;
}

static int remove_files(const char *boardname)
{
	char buff[256];
	
	snprintf(buff, sizeof(buff), "%s/boards/%s", BBS_HOME, boardname);
	chdir(buff);
	unlink(".MARK");
	unlink(".ORIGIN");
	unlink(".THREAD");

	return 0;
}

static int cnv_board_dir(struct boardheader *bh, void *arg)
{
	printf("=======================================\n");
	printf("Boardname: %s\n", bh->filename);
	build_dir(bh->filename);
	convert_normal(bh->filename);
	convert_delete(bh->filename);
	convert_junk(bh->filename);
	convert_ding(bh->filename);
	remove_files(bh->filename);
	return 0;
}

int main(int argc, char ** argv)
{
	int all = 0;
	struct boardheader *bp;
	struct userec* sysopuser;

    if (init_all()) {
        printf("init data fail\n");
        return -1;
    }

	getuser("SYSOP", &sysopuser);
	setCurrentUser(sysopuser);
	if (argc == 1)
		all = 1;
	else if (argc > 2)
	{
		printf("Usage: %s [boardid]\n", argv[0]);
		return -1;
	}
	if (all == 1)
	{
		apply_boards(cnv_board_dir,NULL);
	}
	else
	{
		if ((bp = getbcache(argv[1])) != NULL)
			cnv_board_dir(bp,NULL);
	}

	return 0;
}

