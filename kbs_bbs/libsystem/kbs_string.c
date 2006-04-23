/* 使用Boyer-Moore-Horspool-Sunday 算法进行字符串匹配的系列函数
算法提出:BOYER, R., and S. MOORE. 1977. "A Fast String Searching Algorithm." 
			 HORSPOOL, R. N. 1980. "Practical Fast Searching in Strings."
			  Software - Practice and Experience, 10, 501-06. Further improvements by HUME, A., and D. M. SUNDAY. 1991. 
   2002.08.20 周霖 KCN
*/
#include "system.h"
/* 字符串查找函数*/


/* 内存匹配函数memfind
*/
static void *memfind(const void *in_block,     /* 数据块 */
              const size_t block_size,  /* 数据块长度 */
              const void *in_pattern,   /* 需要查找的数据 */
              const size_t pattern_size,        /* 查找数据的长度 */
              size_t * shift,   /* 移位表，应该是256*size_t的数组 */
              int *init)
{                               /* 是否需要初始化移位表 */
    size_t byte_nbr,            /* Distance through block */
     match_size,                /* Size of matched part */
     limit;
    const unsigned char *match_ptr = NULL;
    const unsigned char *block = (unsigned char *) in_block,    /* Concrete pointer to block data */
    *pattern = (unsigned char *) in_pattern;    /* Concrete pointer to search value */

    if (block == NULL || pattern == NULL || shift == NULL)
        return (NULL);

/* 查找的串长应该小于 数据长度*/
    if (block_size < pattern_size)
        return (NULL);

    if (pattern_size == 0)      /* 空串匹配第一个 */
        return ((void *) block);

/* 如果没有初始化，构造移位表*/
    if (!init || !*init) {
        for (byte_nbr = 0; byte_nbr < 256; byte_nbr++)
            shift[byte_nbr] = pattern_size + 1;
        for (byte_nbr = 0; byte_nbr < pattern_size; byte_nbr++)
            shift[(unsigned char) pattern[byte_nbr]] = pattern_size - byte_nbr;

        if (init)
            *init = 1;
    }

/*开始搜索数据块，每次前进移位表中的数量*/
    limit = block_size - pattern_size + 1;
    for (byte_nbr = 0; byte_nbr < limit; byte_nbr += shift[block[byte_nbr + pattern_size]]) {
        if (block[byte_nbr] == *pattern) {
            /*
             * 如果第一个字节匹配，那么继续匹配剩下的
             */
            match_ptr = block + byte_nbr + 1;
            match_size = 1;

            do {
                if (match_size == pattern_size)
                    return (void *) (block + byte_nbr);
            } while (*match_ptr++ == pattern[match_size++]);
        }
    }
    return NULL;
}

/* 大小写不敏感的匹配函数txtfind
*/
static void *txtfind(const void *in_block,     /* 数据块 */
              const size_t block_size,  /* 数据块长度 */
              const void *in_pattern,   /* 需要查找的数据 */
              const size_t pattern_size,        /* 查找数据的长度 */
              size_t * shift,   /* 移位表，应该是256*size_t的数组 */
              int *init)
{                               /* 是否需要初始化移位表 */
    size_t byte_nbr,            /* Distance through block */
     match_size,                /* Size of matched part */
     limit;
    const unsigned char *match_ptr = NULL;
    const unsigned char *block = (unsigned char *) in_block,    /* Concrete pointer to block data */
    *pattern = (unsigned char *) in_pattern;    /* Concrete pointer to search value */

    if (block == NULL || pattern == NULL || shift == NULL)
        return (NULL);

/* 查找的串长应该小于 数据长度*/
    if (block_size < pattern_size)
        return (NULL);

    if (pattern_size == 0)      /* 空串匹配第一个 */
        return ((void *) block);

/* 如果没有初始化，构造移位表*/
    if (!init || !*init) {
        for (byte_nbr = 0; byte_nbr < 256; byte_nbr++)
            shift[byte_nbr] = pattern_size + 1;
        for (byte_nbr = 0; byte_nbr < pattern_size; byte_nbr++)
            shift[(unsigned char) tolower(pattern[byte_nbr])] = pattern_size - byte_nbr;

        if (init)
            *init = 1;
    }

/*开始搜索数据块，每次前进移位表中的数量*/
    limit = block_size - pattern_size + 1;
    for (byte_nbr = 0; byte_nbr < limit; byte_nbr += shift[tolower(block[byte_nbr + pattern_size])]) {
        if (tolower(block[byte_nbr]) == tolower(*pattern)) {
            /*
             * 如果第一个字节匹配，那么继续匹配剩下的
             */
            match_ptr = block + byte_nbr + 1;
            match_size = 1;

            do {
                if (match_size == pattern_size)
                    return (void *) (block + byte_nbr);
            } while (tolower(*match_ptr++) == tolower(pattern[match_size++]));
        }
    }
    return NULL;
}

char *bm_strstr(const char *string, const char *pattern)
{
    size_t shift[256];
    int init = 0;

    return (char *) memfind(string, strlen(string), pattern, strlen(pattern), shift, &init);
}

/* 字符串多次匹配函数*/
char *bm_strstr_rp(const char *string, const char *pattern, size_t * shift, int *init)
{
    return (char *) memfind(string, strlen(string), pattern, strlen(pattern), shift, init);
}

/* 字符串大小写不敏感的匹配函数*/
char *bm_strcasestr(const char *string, const char *pattern)
{
    size_t shift[256];
    int init = 0;

    return (char *) txtfind(string, strlen(string), pattern, strlen(pattern), shift, &init);
}

/* 字符串多次大小写不敏感匹配函数*/
char *bm_strcasestr_rp(const char *string, const char *pattern, size_t * shift, int *init)
{
    return (char *) txtfind(string, strlen(string), pattern, strlen(pattern), shift, init);
}


#ifndef HAVE_STRCASESTR
char *strcasestr(const char *haystack, const char *needle)
{
    return bm_strcasestr(haystack, needle);
}
#endif /* HAVE_STRCASESTR */



#ifndef HAVE_MEMMEM
void *memmem(const void *s, size_t slen, const void *p, size_t plen)
{
    size_t shift[256];
    int init = 0;
    return memfind(s, slen, p, plen, shift, &init);
}
#endif /* HAVE_MEMMEM */



#ifndef HAVE_STRSEP
char* strsep(char **strptr,const char *delim){
    char *ptr;
    if(!(ptr=*strptr))
        return NULL;
    if(!*delim){
        *strptr=NULL;
        return ptr;
    }
    if(!(*strptr=(!*(delim+1)?strchr(*strptr,*delim):strpbrk(*strptr,delim))))
        return ptr;
    *((*strptr)++)=0;
    return ptr;
}
#endif /* HAVE_STRSEP */

