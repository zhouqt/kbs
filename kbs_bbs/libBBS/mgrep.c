/* Copyright (c) 1991 Sun Wu and Udi Manber.  All Rights Reserved. */
/* multipattern matcher */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXPAT  256
#define MAXLINE 1024
#define MAXSYM  256
#define MAXMEMBER1 4096
#define MAXPATFILE 2600 /*pattern文件的最大长度*/
#define BLOCKSIZE  8192  /*用于预读的数据大小*/
#define MAXHASH    512  /*pattern使用的hash表大小*/
#define mm 	   511  /*用于hash值的取模运算*/
#define max_num    200 /*最大的pattern个数*/
#define W_DELIM	   128
#define L_DELIM    10


#undef I_WANT_TO_DEBUG_MGREP_CRAP
#ifdef I_WANT_TO_DEBUG_MGREP_CRAP
#define printf prints
#ifdef putchar
#undef putchar
#define putchar outc
#endif
#endif /* I_WANT_TO_DEBUG_MGREP_CRAP */


#include "bbs.h"
struct pat_list {
    int index;
    int next;
/*    struct pat_list *next;*/
};
struct pattern_image {
	int LONG;
	int SHORT;
	int p_size;
	unsigned char SHIFT1[MAXMEMBER1];
	unsigned char tr[MAXSYM];
	unsigned char tr1[MAXSYM];
    unsigned int HASH[MAXHASH]; /*pattern hash 入口,指向hashtable的位置*/
	unsigned char buf[MAXPATFILE + BLOCKSIZE]; /*原始的file*/
	unsigned char pat_spool[MAXPATFILE + 2 * max_num + MAXPAT];/*保存转化后的pattern*/
	unsigned long patt[max_num]; /*用于指向pat_spool的偏移*/
	unsigned char pat_len[max_num]; /*每一个pattern的长度*/
	struct pat_list hashtable[max_num+1]; /* 保存所有的pattern hash数据,0节点恒为index=0*/
};

int ONLYCOUNT, FNAME, SILENT, FILENAMEONLY;
int INVERSE;
int WORDBOUND, WHOLELINE, NOUPPER;

static void m_short(unsigned char* text,int start,int end,struct pattern_image* patt_img, session_t* session);
static void f_prep(int pat_index, unsigned char *Pattern, struct pattern_image* patt_img);
static void monkey1(register unsigned char *text, int start,int  end, struct pattern_image* patt_img, session_t* session);

int releasepf(struct pattern_image* patt_img)
{
    /*
    for (i = 0; i < MAXHASH; i++) {
        struct pat_list* curr;
        curr=patt_img->HASH[i];
        while (curr!=NULL) {
            struct pat_list* next;
            next=curr->next;
        	free((void*)curr);
        	curr=next;
        }
    }
    */
    free((void*)patt_img);
    return 0;
}

int prepf(int fp,struct pattern_image** ppatt_img,size_t* patt_image_len)
{
    int length = 0, i, p = 1, num_pat;
    struct pattern_image *patt_img;
    unsigned char *pat_ptr;
    unsigned Mask = 15;
    int num_read;

    *ppatt_img=(struct pattern_image*)malloc(sizeof(struct pattern_image));
    patt_img=*ppatt_img;
    *patt_image_len=sizeof(*patt_img);
    bzero(patt_img,*patt_image_len);
    pat_ptr=patt_img->pat_spool;
    patt_img->LONG = 0;
    patt_img->SHORT = 0;
    patt_img->p_size = 0;
	patt_img->hashtable[0].index = 0;
	patt_img->hashtable[0].next = 0;
    while ((num_read = read(fp, patt_img->buf + length, BLOCKSIZE)) > 0) {
        length = length + num_read;
        if (length > MAXPATFILE) {
            bbslog("3error", "maximum pattern file size is %d", MAXPATFILE);
            return -1;
        }
    }
    patt_img->buf[length] = '\n';
    i = 0;
    p = 1;
    while (i < length) {
        patt_img->patt[p] = pat_ptr-patt_img->pat_spool;
        if (WORDBOUND)
            *pat_ptr++ = W_DELIM;
        if (WHOLELINE)
            *pat_ptr++ = L_DELIM;
        while ((*pat_ptr = patt_img->buf[i++]) != '\n')
            pat_ptr++;
        if (WORDBOUND)
            *pat_ptr++ = W_DELIM;
        if (WHOLELINE)
            *pat_ptr++ = L_DELIM;       /* Can't be both on */
        *pat_ptr++ = 0;
        p++;
    }
    if (p > max_num) {
        bbslog("3error", "maximum number of patterns is %d", max_num);
        return -1;
    }
    for (i = 1; i < 20; i++)
        *pat_ptr = i;           /* boundary safety zone */
    for (i = 0; i < MAXSYM; i++)
        patt_img->tr[i] = i;
    if (NOUPPER) {
        for (i = 'A'; i <= 'Z'; i++)
            patt_img->tr[i] = i + 'a' - 'A';
    }
    if (WORDBOUND) {
        for (i = 0; i < 128; i++)
            if (!isalnum(i))
                patt_img->tr[i] = W_DELIM;
    }
    for (i = 0; i < MAXSYM; i++)
        patt_img->tr1[i] = patt_img->tr[i] & Mask;
    num_pat = p - 1;
    patt_img->p_size = MAXPAT;
    for (i = 1; i <= num_pat; i++) {
        p = strlen((char *)(patt_img->pat_spool+patt_img->patt[i]));
        patt_img->pat_len[i] = p;
        if (p != 0 && p < patt_img->p_size)
            patt_img->p_size = p;
    }
    if (patt_img->p_size == 0) {
        bbslog("3error", "the pattern file is empty");
        return -1;
    }
    if (length > 400 && patt_img->p_size > 2)
        patt_img->LONG = 1;
    if (patt_img->p_size == 1)
        patt_img->SHORT = 1;
    for (i = 0; i < MAXMEMBER1; i++)
        patt_img->SHIFT1[i] = patt_img->p_size - 2;
    for (i = 0; i < MAXHASH; i++) {
        patt_img->HASH[i] = 0;
    }
    for (i = 1; i <= num_pat; i++)
        f_prep(i, patt_img->pat_spool+patt_img->patt[i],patt_img);
    return 0;
}

int mgrep_str(char *text, int num,struct pattern_image* patt_img, session_t* session)
{
    if (patt_img->SHORT)
        m_short((unsigned char *)text, 0, num-1, patt_img, session);
    else
        monkey1((unsigned char *)text, 0, num-1, patt_img, session);
    return session->num_of_matched;
}                               /* end mgrep */

static int countline(text, len)
unsigned char *text;
int len;
{
    int i,total_line;

    for (i = 0, total_line=0; i < len; i++)
        if (text[i] == '\n')
            total_line++;
	return total_line;
}

int mgrep(int fd,struct pattern_image *patt_img, session_t* session)
{
    register char r_newline = '\n';
    unsigned char text[2 * BLOCKSIZE + MAXLINE];
    register int buf_end, num_read, start, end, residue = 0;

    text[MAXLINE - 1] = '\n';   /* initial case */
    start = MAXLINE - 1;
    end = MAXLINE - 1;

    while ((num_read = read(fd, text + MAXLINE, BLOCKSIZE)) > 0) {
        if (INVERSE && ONLYCOUNT)
            session->total_line=countline(text + MAXLINE, num_read);
        buf_end = end = MAXLINE + num_read - 1;
        while (text[end] != r_newline && end > MAXLINE)
            end--;
        residue = buf_end - end + 1;
        text[start - 1] = r_newline;
        if (patt_img->SHORT)
            m_short(text, start, end,patt_img, session);
        else
            monkey1(text, start, end,patt_img, session);
        if (FILENAMEONLY && session->num_of_matched) {
            return session->num_of_matched;
        }
        start = MAXLINE - residue;
        if (start < 0) {
            start = 1;
        }
        strncpy((char *)(text + start), (char *)(text + end), residue);
    }                           /* end of while(num_read = ... */
    text[MAXLINE] = '\n';
    text[start - 1] = '\n';
    if (residue > 1) {
        if (patt_img->SHORT)
            m_short(text, start, end,patt_img, session);
        else
            monkey1(text, start, end,patt_img, session);
    }
    return 0;
}                               /* end mgrep */


static void monkey1(register unsigned char *text, int start,int  end, struct pattern_image* patt_img, session_t* session)
{
    register unsigned char *textend;
    register unsigned hash, i;
    register unsigned char shift;
    register int m1, j, Long = patt_img->LONG;
    int pat_index, m = patt_img->p_size;
    int MATCHED = 0;
    register unsigned char *qx;
    register struct pat_list *p;
    unsigned char *lastout;
#ifdef I_WANT_TO_DEBUG_MGREP_CRAP
    int OUT = 0;
#endif /* I_WANT_TO_DEBUG_MGREP_CRAP */

    textend = text + end;
    m1 = m - 1;
    lastout = text + start + 1;
    text = text + start + m1;
    while (text <= textend) {
        hash = patt_img->tr1[*text];
        hash = (hash << 4) + (patt_img->tr1[*(text - 1)]);
        if (Long)
            hash = (hash << 4) + (patt_img->tr1[*(text - 2)]);
        shift = patt_img->SHIFT1[hash];
        if (shift == 0) {
            hash = 0;
            for (i = 0; i <= m1; i++) {
                hash = (hash << 4) + (patt_img->tr1[*(text - i)]);
            }
            hash = hash & mm;
            p = &patt_img->hashtable[patt_img->HASH[hash]];
            while (p&&p->index != 0) {
                pat_index = p->index;
		if (p->next==0)
		    p=NULL;
		else
                    p = &patt_img->hashtable[p->next];
                qx = text - m1;
                j = 0;
                while (patt_img->tr[*(patt_img->pat_spool+patt_img->patt[pat_index]+j)] == patt_img->tr[*(qx++)])
                    j++;
                if (j > m1) {
                    if (patt_img->pat_len[pat_index] <= j) {
                        if (text > textend)
                            return;
                        session->num_of_matched++;
                        if (FILENAMEONLY || SILENT)
                            return;
                        MATCHED = 1;
                        if (ONLYCOUNT) {
                            while (*text != '\n')
                                text++;
                        } else {
#ifdef I_WANT_TO_DEBUG_MGREP_CRAP
                            if (!INVERSE) {
                                if (FNAME)
                                    printf("%s: ", session->CurrentFileName);
                                while (*(--text) != '\n');
                                while (*(++text) != '\n')
                                    putchar(*text);
                                printf("\n");
                            } else {
                                if (FNAME)
                                    printf("%s: ", session->CurrentFileName);
                                while (*(--text) != '\n');
                                if (lastout < text)
                                    OUT = 1;
                                while (lastout < text)
                                    putchar(*lastout++);
                                if (OUT) {
                                    putchar('\n');
                                    OUT = 0;
                                }
                                while (*(++text) != '\n');
                                lastout = text + 1;
                            }
#endif /* I_WANT_TO_DEBUG_MGREP_CRAP */
                        }
/*
				else {
			  		if(FNAME) printf("%s: ",CurrentFileName);
                          		while(*(--text) != '\n');
                          		while(*(++text) != '\n') putchar(*text);
			  		printf("\n");
				}
*/
                    }
                }
                if (MATCHED)
                    break;
            }
            if (!MATCHED)
                shift = 1;
            else {
                MATCHED = 0;
                shift = m1;
            }
        }
        text = text + shift;
    }
#ifdef I_WANT_TO_DEBUG_MGREP_CRAP
    if (INVERSE && !ONLYCOUNT)
        while (lastout <= textend)
            putchar(*lastout++);
#endif /* I_WANT_TO_DEBUG_MGREP_CRAP */
}

static void m_short(unsigned char* text,int start,int end,struct pattern_image* patt_img, session_t* session)
{
    register unsigned char *textend;
    register int j;
    register struct pat_list *p;
    register int pat_index;
    int MATCHED = 0;
#ifdef I_WANT_TO_DEBUG_MGREP_CRAP
    int OUT = 0;
#endif /* I_WANT_TO_DEBUG_MGREP_CRAP */
    unsigned char *lastout;
    unsigned char *qx;

    textend = text + end;
    lastout = text + start + 1;
    text = text + start - 1;
    while (++text <= textend) {
        p = &patt_img->hashtable[patt_img->HASH[*text]];
        while (p&&p->index != 0) {
            pat_index = p->index;
	    if (p->next==0)
	        p=NULL;
	    else
                p = &patt_img->hashtable[p->next];
            qx = text;
            j = 0;
            while (patt_img->tr[*(patt_img->pat_spool+patt_img->patt[pat_index]+j)] == patt_img->tr[*(qx++)])
                j++;
            if (patt_img->pat_len[pat_index] <= j) {
                if (text >= textend)
                    return;
                session->num_of_matched++;
                if (FILENAMEONLY || SILENT)
                    return;
                if (ONLYCOUNT) {
                    while (*text != '\n')
                        text++;
                } else {
#ifdef I_WANT_TO_DEBUG_MGREP_CRAP
                    if (FNAME)
                        printf("%s: ", session->CurrentFileName);
                    if (!INVERSE) {
                        while (*(--text) != '\n');
                        while (*(++text) != '\n')
                            putchar(*text);
                        printf("\n");
                        MATCHED = 1;
                    } else {
                        while (*(--text) != '\n');
                        if (lastout < text)
                            OUT = 1;
                        while (lastout < text)
                            putchar(*lastout++);
                        if (OUT) {
                            putchar('\n');
                            OUT = 0;
                        }
                        while (*(++text) != '\n');
                        lastout = text + 1;
                        MATCHED = 1;
                    }
#endif /* I_WANT_TO_DEBUG_MGREP_CRAP */
                }
            }
            if (MATCHED)
                break;
        }
        MATCHED = 0;
    }                           /* while */
#ifdef I_WANT_TO_DEBUG_MGREP_CRAP
    if (INVERSE && !ONLYCOUNT)
        while (lastout <= textend)
            putchar(*lastout++);
#endif /* I_WANT_TO_DEBUG_MGREP_CRAP */
}

static void f_prep(int pat_index, unsigned char *Pattern, struct pattern_image* patt_img)
{
    int i, m;
    register unsigned hash, Mask = 15;
	struct pat_list *pt, *qt;

    m = patt_img->p_size;
    for (i = m - 1; i >= (1 + patt_img->LONG); i--) {
        hash = (Pattern[i] & Mask);
        hash = (hash << 4) + (Pattern[i - 1] & Mask);
        if (patt_img->LONG)
            hash = (hash << 4) + (Pattern[i - 2] & Mask);
        if (patt_img->SHIFT1[hash] >= m - 1 - i)
            patt_img->SHIFT1[hash] = m - 1 - i;
    }
    if (patt_img->SHORT)
        Mask = 255;             /* 011111111 */
    hash = 0;
    for (i = m - 1; i >= 0; i--) {
        hash = (hash << 4) + (patt_img->tr[Pattern[i]] & Mask);
    }
/*
	if(INVERSE) hash = Pattern[1];
*/
    hash = hash & mm;
    qt = &patt_img->hashtable[pat_index];
    qt->index = pat_index;
	if (patt_img->HASH[hash]!=0) {
    pt = &patt_img->hashtable[patt_img->HASH[hash]];
    qt->next = pt->index;
	} else qt->next=0;
    patt_img->HASH[hash] = pat_index;
}
