#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include "bbs.h"

#define LEN (ARTICLE_TITLE_LEN-10)
#define PER_TEST 10000

#ifdef NEWSMTH
const char *boards[] = {"newexpress", "trafficinfo", "pielove", "joke", "picture", "advancededu", "newsoftware"};
#else
const char *boards[] = {"SYSOP"};
#endif

typedef char* (*STRNZHCPY_FUNC)(char *dest, const char *src, size_t n);

char* original_strnzhcpy(char *dest, const char *src, size_t n)
{
	strncpy(dest, src, n);
	dest[n-1] = '\0';
	return dest;
}

inline char * originalIL_strnzhcpy(char *dest, const char *src, size_t n)
{
	strncpy(dest, src, n);
	dest[n-1] = '\0';
	return dest;
}

char* originalNP_strnzhcpy(char *dest, const char *src, size_t n)
{
	int l = strlen(src);
	if (n > l + 1)
	n = l + 1;
	memcpy(dest, src, n - 1);
	dest[n - 1] = 0;
	return dest;
}

char* flyriver_strnzhcpy(char *dest, const char *src, size_t n)
{
	register unsigned char is_zh = 0;
	register char c;
	register const char *ptr = src;
	register char *ptr2 = dest;

	if (dest == NULL || src == NULL || n == 0)
		return dest;

	dest[--n] = '\0';
	while ((ptr - src) < n && (c = *ptr) != '\0')
	{
		*ptr2 = c;
		//is_zh += ((c & 0x80));
		is_zh = (((c) & 0x80) & (is_zh ^ 0x80) ); 
		ptr2++;
		ptr++;
	}
	if ((is_zh & 0x80) != 0)
		*(ptr2 - 1) = '\0';
	else
		*ptr2 = 0;

	return dest;
}

char *dvlt_strnzhcpy(char *dest, const char *src, size_t n)
{
	char *d = dest;
	char flag = 0;

	if (n == 0)
		return dest;

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

char* etn_strnzhcpy(char *to,const char *from,size_t size){
	char *p;
	unsigned int i;
	size_t len;
	if((len=strlen(from))<size) {
		strcpy(to,from);
		len++;
	} else {
		strncpy(to,from,size*sizeof(char));
		len = size;
	}
	for(p=to+len,p--,p--,i=1;!(p<to)&&((*p)&0x80);p--)
		i^=0x03;
	to[len-i]=0;
	return to;
}

char* etnlegend_strnzhcpy(char *to,const char *from,size_t size){
		register char *p;
		register unsigned int i;
		size_t len;
		if (size==0) return to;
		len=strnlen(from,--size);
		memcpy(to,from,len);
		for(p=to+len,p--,i=0;!(p<to)&&((*p)&0x80);p--)
				i^=0x01;
		to[len-i]=0;
		return to;
}


char *at3p_strnzhcpy(char *dest, const char *src, size_t n) {
	register int c = 0, i = 0;
	register char *dst = dest;
	if (n==0) return dest;
	n--;
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

char *atppp_strnzhcpy(char *dest, const char *src, size_t n) {
#define COPY_ONE_BYTE \
		if (*src == '\0') { n = 0; break; } \
		c = (((*src) & 0x80) & (c ^ 0x80) ); \
		*dest = *src; \
		dest++; src++; \
		n--;
	register int c = 0;
	register char *dst = dest;
	if (n==0) return dest;
	n--;
	while( n > 8 ) {
		COPY_ONE_BYTE
		COPY_ONE_BYTE
		COPY_ONE_BYTE
		COPY_ONE_BYTE
		COPY_ONE_BYTE
		COPY_ONE_BYTE
		COPY_ONE_BYTE
		COPY_ONE_BYTE
	}
	while (n > 0) {
		COPY_ONE_BYTE
	}
	*(dest - (c>>7) )='\0';
	return dst;
}
char *stiger_strnzhcpy(char *dest, const char *src, size_t n) {
	register int c = 0;
	register char *dst = dest;
	if (n==0) return dest;
	n--;
	while( n > 0 && *src != '\0') {
		c = (((*src) & 0x80) & (c ^ 0x80) );
		*dest = *src;
		dest++; src++;
		n--;
	}
	*(dest - (c>>7) )='\0';
	return dst;
}

/*
 * although this is the fastest, but: 
 * it only works on little-endian
 * it might read beyond the end of src string
 */
char *crazyatppp_strnzhcpy(char *dest, const char *src, size_t n) {
	register char *dd = dest;
	register int c, m;
	register unsigned int si;
	if (n==0) return dest;
	--n;
	m = n % 16;
	n = n >> 4;
	c = 0;
	while( n > 0 ) {
#define CDW \
		si = *((unsigned int*) src); \
		*((unsigned int*) dest) = si; \
		dest+=4; src+=4; \
		if ((si & 0xff) == 0) { dest-=4; goto outofhere;} \
		c = ((si & 0x80) & (c ^ 0x80) ); \
		si = si >> 8; \
		if ((si & 0xff) == 0) { dest-=3; goto outofhere;} \
		c = ((si & 0x80) & (c ^ 0x80) ); \
		si = si >> 8; \
		if ((si & 0xff) == 0) { dest-=2; goto outofhere;} \
		c = ((si & 0x80) & (c ^ 0x80) ); \
		si = si >> 8; \
		if ((si & 0xff) == 0) { dest-=1; goto outofhere;} \
		c = ((si & 0x80) & (c ^ 0x80) ); \

		n--;
		CDW
		CDW
		CDW
		CDW
	}
#define COB \
		if ((*src) == 0) goto outofhere; \
		c = (((*src) & 0x80) & (c ^ 0x80) ); \
		*dest = *src; \
		dest++; src++;
	switch(m) {
		case 15: COB
		case 14: COB
		case 13: COB
		case 12: COB
		case 11: COB
		case 10: COB
		case 9: COB
		case 8: COB
		case 7: COB
		case 6: COB
		case 5: COB
		case 4: COB
		case 3: COB
		case 2: COB
		case 1: COB
	}
outofhere:
	*(dest - (c>>7) )='\0';
	return dd;
}




#define TEST_FUNC(name) test_func(name##_strnzhcpy, #name)
		
static int range_rand(double range){
	return (int)(range*rand()/((double)RAND_MAX+1));
}

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
	int isInline = (fn == originalIL_strnzhcpy);

#define OOPREFIX "original"
	if (strncmp(func_name, OOPREFIX, strlen(OOPREFIX))) {
		char ss[80], dd[80], dds[80];
		char * ret;
		for (j=-128;j<128;j++) {
			dd[0] = j;
			ret = fn(dd, ss, 0);
			if (ret != dd || dd[0] != j) {
				printf("%s: failed boundary test.\n", func_name);
				return;
			}
		}
		srand(time(NULL));
		for (j=0;j<10000;j++){
			int ll;
			for(i=0;i<79;i++) ss[i]= range_rand(256) - 128;
			ss[79] = 0;
			ll = range_rand(60) + 1;
			stiger_strnzhcpy(dds, ss, ll);
			ret = fn(dd, ss, ll);
			if (ret != dd || strcmp(dd, dds)) {
				printf("%s: failed correctness test.\n", func_name);
				return;
			}
		}
	}

	for(i=0;i<sizeof(boards)/sizeof(char *);i++)
	{
		bh = getbcache(boards[i]);
		if (!bh) {
			printf("%s: shit, no board!\n", boards[i]);
			exit(0);
		}
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
			if (isInline) {
				gettimeofday(&tv1, NULL);
				for (i = 0; i < PER_TEST; i++)
					originalIL_strnzhcpy(dest, src, LEN);
				gettimeofday(&tv2, NULL);
			} else {				
				gettimeofday(&tv1, NULL);
				for (i = 0; i < PER_TEST; i++)
					fn(dest, src, LEN);
				gettimeofday(&tv2, NULL);
			}
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
	TEST_FUNC(originalIL); /* inline */
	TEST_FUNC(originalNP); /* no tail padding */
	TEST_FUNC(flyriver);
	TEST_FUNC(dvlt);
	TEST_FUNC(etn);
	TEST_FUNC(etnlegend);
	TEST_FUNC(at3p);
	TEST_FUNC(stiger);
	TEST_FUNC(atppp);
	TEST_FUNC(crazyatppp);
	
	return 0;
}

