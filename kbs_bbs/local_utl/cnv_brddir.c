#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bbs.h"

#define BOARDSFILE "/home/bbs/.BOARDS"
#define BBS_HOME    "/home/bbs"

char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

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

static HTElement htable[10000];

int hash_function(char * filename)
{
	int val;
	int len;

	len = strlen(filename);
	val = (filename[len-10] - '0') * 1000;
	val += (filename[len-9] - '0') * 100;
	val += (filename[len-8] - '0') * 10;
	val += filename[len-7] - '0';
	val += (filename[len-6] - '0') * 1000;
	val += (filename[len-5] - '0') * 100;
	val += (filename[len-4] - '0') * 10;
	val += filename[len-3] - '0';
	val %= 10000;

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

	for (i = 0; i < 10000; i++)
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

int convert(char *boardname)
{
	char buff[256];
	int fd;
	struct fileheader fh;
	int rc;
	int i, j, k, rn;
	Node * ptr;
#ifdef HAPPY_BBS
	char num_str[20];
#endif // HAPPY_BBS

	///////////// Initialization ///////////// 

	snprintf(buff, sizeof(buff), "%s/boards/%s", BBS_HOME, boardname);
	chdir(buff);
    for (i = 0; i < 52; i++)
    {
        snprintf(buff, sizeof(buff), "%c", alphabet[i]);
        mkdir(buff, 0755);
    }

	///////////// Convert .DIR file ///////////// 

	if ((fd = open(".DIR", O_RDWR)) == -1)
	{
		perror("open .DIR");
		return -1;
	}

	printf("Converting .DIR file ...");
	fflush(stdout);

	k = 0;
	// read every record in sequence
	while ((rc = read(fd, &fh, sizeof(fh))) == sizeof(fh))
	{
        rn = 0 + (int) (52.0 * rand() / (RAND_MAX + 1.0));
		if (fh.accessed[0] & FILE_DIGEST)
		{
			ptr = (Node *)malloc(sizeof(Node));
			if (ptr == NULL)
			{
				perror("malloc");
				destroy_list();
				printf(" Failed.\n");
				return -2;
			}
			strncpy(ptr->filename, fh.filename, sizeof(ptr->filename));
			ptr->directory = alphabet[rn];
			ptr->next = NULL;
			insert_node(ptr);
		}

#ifdef HAPPY_BBS
		strncpy(num_str, fh.filename + 2, sizeof(num_str));
		num_str[strlen(num_str) - 2] = '\0';
		fh.posttime = atoi(num_str);
#endif // HAPPY_BBS
        snprintf(buff, sizeof(buff), "%c/%s", alphabet[rn], fh.filename);
        rename(fh.filename, buff);
        strncpy(fh.filename, buff, sizeof(fh.filename));
        lseek(fd, 0-rc, SEEK_CUR);
        write(fd, &fh, sizeof(fh));
		k++;
	}
	printf(" Done.\n");
	close(fd);

	///////////// Convert .DIGEST file ///////////// 

	if ((fd = open(".DIGEST", O_RDWR)) == -1)
	{
		printf("No .DIGEST file.\n");
		return 1;
	}

	printf("Converting .DIGEST file ...");
	fflush(stdout);

	while ((rc = read(fd, &fh, sizeof(fh))) == sizeof(fh))
	{
		if ((ptr = search(fh.filename)) == NULL)
		{
        	rn = 0 + (int) (52.0 * rand() / (RAND_MAX + 1.0));
        	snprintf(buff, sizeof(buff), "%c/%s", alphabet[rn], fh.filename);
		}
		else
			snprintf(buff, sizeof(buff), "%c/%s", ptr->directory, fh.filename);

#ifdef HAPPY_BBS
		strncpy(num_str, fh.filename + 2, sizeof(num_str));
		num_str[strlen(num_str) - 2] = '\0';
		fh.posttime = atoi(num_str);
#endif // HAPPY_BBS
        rename(fh.filename, buff);
        strncpy(fh.filename, buff, sizeof(fh.filename));
        lseek(fd, 0-rc, SEEK_CUR);
        write(fd, &fh, sizeof(fh));
	}

	printf(" Done.\n");
	close(fd);

	///////////// Collision Summary ///////////// 

	j = 0;
	for (i = 0; i < 10000; i++)
	{
		if (htable[i].count > 1)
			j++;
	}
	printf("  Collision: %4d    Total:    %4d     Ratio: %.2f\n\n",
			j, k, j * 1.0 / k);

	destroy_list();

	return 0;
}

int main(int argc, char ** argv)
{
	struct boardheader board;
	int fd;
	int rc;
	int all = 0;

	if (argc == 1)
		all = 1;
	else if (argc > 2)
	{
		printf("Usage: %s [boardid]\n", argv[0]);
		return -1;
	}
	if ((fd = open(BOARDSFILE, O_RDONLY)) < 0)
	{
		perror("open");
		exit(-1);
	}
	while ((rc = read(fd, &board, sizeof(board))) == sizeof(board))
	{
		if (all == 1)
		{
			printf("Boardname: %s\n", board.filename);
			convert(board.filename);
		}
		else if (!strcasecmp(board.filename, argv[1]))
		{
			convert(board.filename);
			break;
		}
	}
	close(fd);

	return 0;
}

