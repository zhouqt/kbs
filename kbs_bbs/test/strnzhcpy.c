#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include "bbs.h"

#define LEN (ARTICLE_TITLE_LEN-2)
#define PER_TEST 10000
const char *boards[] = {"SYSOP"};

typedef char* (*STRNZHCPY_FUNC)(char *dest, const char *src, size_t n);

char* original_strnzhcpy(char *dest, const char *src, size_t n)
{
	strncpy(dest, src, n);
	dest[n-1] = '\0';
	return dest;
}

char* flyriver_strnzhcpy(char *dest, const char *src, size_t n)
{
	register unsigned char is_zh = 0;
	register char c;
	const char *ptr = src;
	char *ptr2 = dest;

	if (dest == NULL || src == NULL || n == 0)
		return NULL;

	dest[--n] = '\0';
	while ((ptr - src) < n && (c = *ptr) != '\0')
	{
		*ptr2 = c;
		is_zh += ((c & 0x80));
		ptr2++;
		ptr++;
	}
	if ((is_zh & 0x80) != 0)
		*(ptr2 - 1) = '\0';

	return dest;
}

char *dvlt_strnzhcpy(char *dest, const char *src, size_t n)
{
	char *d = dest;
	char flag = 0;

	if (n == 0)
		return NULL;

	for ( ; --n > 0; dest++, src++) {
		if (!(*dest = *src))
			break;
		flag = ~flag & ((*dest) & (char)0x80);
	}

	if (flag)
		*(dest-1) = '\0';
	else
		*dest = 0;

	return d;
}

char* etnlegend_strnzhcpy(char *to,const char *from,size_t size){
	char *p;
	unsigned int i;
	size_t len;
	if((len=strlen(from))<size)
		return strcpy(to,from);
	memcpy(to,from,size*sizeof(char));
	for(p=to+size,p--,p--,i=1;!(p<to)&&((*p)&0x80);p--)
		i^=0x03;
	to[size-i]=0;
	return to;
}

char *at3p_strnzhcpy(char *dest, const char *src, size_t n) {
	register int c = 0, i = 0;
	register char *dst = dest;
	while(i < n && *src != '\0') {
		char ch = *src;
		*dest = ch;
		i++; dest++; src++;
		if (ch > 0) c = 0;
		else c = !c;
	}
	*dest = '\0';
	if (c) *(dest-1)='\0';
	return dst;
}

char *stiger_strnzhcpy(char *dest, const char *src, size_t n) {
	register int c = 0;
	register char *dst = dest;
	while( n > 0 && *src != '\0') {
		c = (((*src) & 0x80) & (c ^ 0x80) );
		*dest = *src;
		dest++; src++;
		n--;
	}
	*(dest - (c>>7) )='\0';
	return dst;
}



#define TEST_FUNC(name) test_func(name##_strnzhcpy, #name)

void test_func(STRNZHCPY_FUNC fn, const char *func_name)
{
	struct timeval tv1, tv2;
	char *src;
	char dest[LEN];
	unsigned long t, tt=0, tt2=0, nt=0;
	
	char path[PATHLEN];
	int i,j,fd,count;
	char *bname;
	fileheader_t *ptr;
	fileheader_t *fh;
	struct boardheader *bh;
	size_t fsize;

	for(i=0;i<sizeof(boards)/sizeof(char *);i++)
	{
		bh = getbcache(boards[i]);
		if (!bh) continue;
		setbfile(path, bh->filename, DOT_DIR);
		if ((fd = open(path, O_RDONLY, 0644)) < 0) {
			continue;
		}
		if (safe_mmapfile_handle(fd, PROT_READ, MAP_SHARED, (void **)&ptr, (off_t *)&fsize)) {
			fh = malloc(fsize);
			memcpy(fh, ptr, fsize);
			end_mmapfile((void*)ptr, fsize, -1);
		} else {
			close(fd);
			continue;
		}
		close(fd);

		count = fsize / sizeof(fileheader_t);
		for (j=0; j<count; j++) {
			src = fh[j].title;
			gettimeofday(&tv1, NULL);
			for (i = 0; i < PER_TEST; i++)
				fn(dest, src, LEN);
			gettimeofday(&tv2, NULL);
			t = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
			tt += t;
			tt2 += t*t;
		}
		nt += count;
		free(fh);
	}
	if (nt) printf("%12.12s: avg: %10.3f, std: %10.3f\n", func_name, tt*1.0/nt, sqrt((nt*tt2-tt*tt)*1.0/nt/nt));
}

int main(int argc, char *argv[])
{
	init_all();
	TEST_FUNC(original);
	TEST_FUNC(flyriver);
	TEST_FUNC(dvlt);
	TEST_FUNC(etnlegend);
	TEST_FUNC(at3p);
	TEST_FUNC(stiger);
	
	return 0;
}
