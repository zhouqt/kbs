#include "php_kbs_bbs.h"  
#include "SAPI.h"
#include "md5.h"

/*
 * refer Ecma-262 
 * '\033'  -> \r (not exactly the same thing, but borrow...)
 * '\n'    -> \n
 * '\\'    -> \\
 * '\''    -> \'
 * '\"'    -> \"
 * '\0'    -> possible start of attachment
 * 0 <= char < 32 -> ignore
 * others  -> passthrough
 */

PHP_FUNCTION(bbs2_readfile)
{
    char *filename;
    int filename_len;
    char *output_buffer;
    int output_buffer_len, output_buffer_size, j;
    char c;
    char *ptr, *cur_ptr;
    off_t ptrlen, mmap_ptrlen;
    int in_chinese = false;
    int chunk_size = 51200;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &filename_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    if (safe_mmapfile(filename, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &mmap_ptrlen, NULL) == 0)
        RETURN_LONG(-1);
    
    j = ptrlen = mmap_ptrlen;
    if (j > chunk_size) j = chunk_size;
    output_buffer_size = 2 * j + 16;
    output_buffer = (char* )emalloc(output_buffer_size);
    output_buffer_len = 0;
    cur_ptr = ptr;
    strcpy(output_buffer + output_buffer_len, "prints('");
    output_buffer_len += 8;
    while (1) {
        for (; j > 0 ; j--) {
            c = *cur_ptr;
            if (c == '\0') { //assume ATTACHMENT_PAD[0] is '\0'
                if (ptrlen >= ATTACHMENT_SIZE + sizeof(int) + 2) {
                    if (!memcmp(cur_ptr, ATTACHMENT_PAD, ATTACHMENT_SIZE)) {
                        ptrlen = -ptrlen;
                        break;
                    }
                }
                ptrlen--; cur_ptr++;
                continue;
            }
            if (c < 0) {
                in_chinese = !in_chinese;
                output_buffer[output_buffer_len++] = c;
            } else {
                do {
                    if (c == '\n') c = 'n';
                    else if (c == '\033') c = 'r';
                    else if (c != '\\' && c != '\'' && c != '\"'
                             && c != '/'    /* to prevent things like </script> */
                             ) {
                        if (c >= 32) {
                            output_buffer[output_buffer_len++] = c;
                        }
                        break;
                    }
                    if (in_chinese && c == 'n') {
                        output_buffer[output_buffer_len++] = ' ';
                    }
                    output_buffer[output_buffer_len++] = '\\';
                    output_buffer[output_buffer_len++] = c;
                } while(0);
                in_chinese = false;
            }
            ptrlen--; cur_ptr++;
        }
        if (ptrlen <= 0) break;
        j = ptrlen;
        if (j > chunk_size) j = chunk_size;
        output_buffer_size += 2 * j;
        output_buffer = (char*)erealloc(output_buffer, output_buffer_size);
        if (output_buffer == NULL) RETURN_LONG(3);
    }
    if (in_chinese) {
        output_buffer[output_buffer_len++] = ' ';
    }
    strcpy(output_buffer + output_buffer_len, "');");
    output_buffer_len += 3;
    
    if (ptrlen < 0) { //attachment
        char *attachfilename, *attachptr;
        char buf[1024];
        char *startbufptr, *bufptr;
        long attach_len, attach_pos, newlen;
        int l;

        ptrlen = -ptrlen;
        strcpy(buf, "attach('");
        startbufptr = buf + strlen(buf);
        while(ptrlen > 0) {
            if (((attachfilename = checkattach(cur_ptr, ptrlen, 
                                  &attach_len, &attachptr)) == NULL)) {
                break;
            }
            attach_pos = attachfilename - ptr;
            newlen = attachptr - cur_ptr + attach_len;
            cur_ptr += newlen;
            ptrlen -= newlen;
            if (ptrlen < 0) break;
            bufptr = startbufptr;
            while(*attachfilename != '\0') {
                switch(*attachfilename) {
                    case '\'':
                    case '\"':
                    case '\\':
                        *bufptr++ = '\\'; /* TODO: boundary check */
                        /* break is missing *intentionally* */
                    default:
                        *bufptr++ = *attachfilename++;  /* TODO: boundary check */
                }
            }
            sprintf(bufptr, "', %ld, %ld);", attach_len, attach_pos);  /* TODO: boundary check */

            l = strlen(buf);
            if (output_buffer_len + l > output_buffer_size) {
                output_buffer_size = output_buffer_size + sizeof(buf) * 10;
                output_buffer = (char*)erealloc(output_buffer, output_buffer_size);
                if (output_buffer == NULL) RETURN_LONG(3);
            }
            strcpy(output_buffer + output_buffer_len, buf);
            output_buffer_len += l;
        }
    }
    end_mmapfile(ptr, mmap_ptrlen, -1);

    RETVAL_STRINGL(output_buffer, output_buffer_len, 0);
}

PHP_FUNCTION(bbs2_readfile_text)
{
    char *filename;
    int filename_len;
    long maxchar;
    long escape_flag; /* 0(default) - escape <>&, 1 - double escape <>&, 2 - escape <>& and space */
    char *output_buffer;
    int output_buffer_len, output_buffer_size, last_return = 0;
    char c;
    char *ptr, *cur_ptr;
    off_t ptrlen, mmap_ptrlen;
    int in_escape = false, is_last_space = false;
    int i;
    char escape_seq[4][16], escape_seq_len[4];

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll", &filename, &filename_len, &maxchar, &escape_flag) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    if (safe_mmapfile(filename, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &mmap_ptrlen, NULL) == 0)
        RETURN_LONG(-1);

    ptrlen = mmap_ptrlen;
    
    if (!maxchar) {
        maxchar = ptrlen;
    } else if (ptrlen > maxchar) {
        ptrlen = maxchar;
    }
    output_buffer_size = 2 * maxchar;
    output_buffer = (char* )emalloc(output_buffer_size);
    output_buffer_len = 0;
    cur_ptr = ptr;
    if (escape_flag == 1) {
        strcpy(escape_seq[0], "&amp;amp;");
        strcpy(escape_seq[1], "&amp;lt;");
        strcpy(escape_seq[2], "&amp;gt;");
        strcpy(escape_seq[3], "&lt;br/&gt;");
    } else {
        strcpy(escape_seq[0], "&amp;");
        strcpy(escape_seq[1], "&lt;");
        strcpy(escape_seq[2], "&gt;");
        strcpy(escape_seq[3], "<br/>");
    }
    for (i=0;i<4;i++) escape_seq_len[i] = strlen(escape_seq[i]);
    while (ptrlen > 0) {
        c = *cur_ptr;
        if (c == '\0') { //assume ATTACHMENT_PAD[0] is '\0'
            break;
        } else if (c == '\033') {
            in_escape = true;
        } else if (!in_escape) {
            if (output_buffer_len + 16 > output_buffer_size) {
                output_buffer = (char*)erealloc(output_buffer, output_buffer_size += 128);
            }
            if (escape_flag == 2 && c == ' ') {
                if (!is_last_space) {
                    output_buffer[output_buffer_len++] = ' ';
                } else {
                    strcpy(output_buffer + output_buffer_len, "&nbsp;");
                    output_buffer_len += 6;
                }
                is_last_space = !is_last_space;
            } else {
                is_last_space = false;
                switch(c) {
                    case '&':
                        strcpy(output_buffer + output_buffer_len, escape_seq[0]);
                        output_buffer_len += escape_seq_len[0];
                        break;
                    case '<':
                        strcpy(output_buffer + output_buffer_len, escape_seq[1]);
                        output_buffer_len += escape_seq_len[1];
                        break;
                    case '>':
                        strcpy(output_buffer + output_buffer_len, escape_seq[2]);
                        output_buffer_len += escape_seq_len[2];
                        break;
                    case '\n':
                        strcpy(output_buffer + output_buffer_len, escape_seq[3]);
                        output_buffer_len += escape_seq_len[3];
                        last_return = output_buffer_len;
                        is_last_space = true;
                        break;
                    default:
                        if (c < 0 || c >= 32)
                            output_buffer[output_buffer_len++] = c;
                        break;
                }
            }
        } else if (isalpha(c)) {
            in_escape = false;
        }
        ptrlen--; cur_ptr++;
    }

    end_mmapfile(ptr, mmap_ptrlen, -1);

    RETVAL_STRINGL(output_buffer, last_return, 0);
}


PHP_FUNCTION(bbs_file_output_attachment)
{
    char *filename;
    int filename_len;
    char *attachname = NULL;
    int attachname_len = 0;
    long attachpos;
    char *ptr;
    char *sendptr = NULL;
    int sendlen;
    off_t ptrlen, mmap_ptrlen;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|s", &filename, &filename_len, &attachpos, &attachname, &attachname_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    
    if (attachpos == 0 && attachname == NULL) {
        RETURN_LONG(-2);
    }

    if (safe_mmapfile(filename, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &mmap_ptrlen, NULL) == 0)
        RETURN_LONG(-1);

    ptrlen = mmap_ptrlen;

    if (attachpos == 0) {
        sendptr = ptr;
        sendlen = ptrlen;
    } else if (attachpos >= 0 && attachpos < ptrlen) {
        char *p, *pend = ptr + ptrlen;
        p = attachname = ptr + attachpos;
        while(*p && p < pend) {
            p++;
        }
        p++;
        if (p <= pend - 4) {
            memcpy(&sendlen, p, 4);
            sendlen = ntohl(sendlen);
            if (sendlen >= 0 && pend - 4 - p >= sendlen) {
                sendptr = p + 4;
            }
        }
    }

    if (sendptr) {
        sapi_header_line ctr = {0};
        char buf[256];
        ctr.line = buf;

        snprintf(buf, 256, "Content-Type: %s", get_mime_type(attachname));
        ctr.line_len = strlen(buf);
        sapi_header_op(SAPI_HEADER_ADD, &ctr TSRMLS_CC);

        snprintf(buf, 256, "Accept-Ranges: bytes");
        ctr.line_len = strlen(buf);
        sapi_header_op(SAPI_HEADER_ADD, &ctr TSRMLS_CC);

        snprintf(buf, 256, "Content-Length: %d", sendlen);
        ctr.line_len = strlen(buf);
        sapi_header_op(SAPI_HEADER_ADD, &ctr TSRMLS_CC);

        snprintf(buf, 256, "Content-Disposition: inline;filename=%s", attachname);
        ctr.line_len = strlen(buf);
        sapi_header_op(SAPI_HEADER_ADD, &ctr TSRMLS_CC);
        
        ZEND_WRITE(sendptr, sendlen);
    }
    end_mmapfile(ptr, mmap_ptrlen, -1);
    RETURN_LONG(0);
}






static char* output_buffer=NULL;
static int output_buffer_len=0;
static int output_buffer_size=0;

void reset_output_buffer() {
    output_buffer=NULL;
    output_buffer_size=0;
    output_buffer_len=0;
}

static void output_printf(const char* buf, uint len)
{
	int bufLen;
	int n,newsize;
	char * newbuf;
	if (output_buffer==NULL) {
		output_buffer=(char* )emalloc(51200); //first 50k
		if (output_buffer==NULL) {
			return;
		}
		output_buffer_size=51200;
	}
	bufLen=strlen(buf);
	if (bufLen>len) {
		bufLen=len;
	}
	n=1+output_buffer_len+bufLen-output_buffer_size;
	if (n>=0) {
		newsize=output_buffer_size+((n/102400)+1)*102400; //n*100k every time
		newbuf=(char*)erealloc(output_buffer,newsize);
		if (newbuf==NULL){
			return;
		}
		output_buffer=newbuf;
		output_buffer_size=newsize;
	}
	memcpy(output_buffer+output_buffer_len,buf,bufLen);
	output_buffer_len+=bufLen;
}

static char* get_output_buffer(){
	return output_buffer;
}

static int get_output_buffer_len(){
	int len=output_buffer_len;
	output_buffer_len=0;
	return len;
}

#if 0
static int new_buffered_output(char *buf, size_t buflen, void *arg)
{
	output_printf(buf,buflen);
	return 0;
}
#endif

static int new_write(const char *buf, uint buflen)
{
	output_printf(buf, buflen);
	return 0;
}

/* 注意，当 is_preview 为 1 的时候，第一个参数 filename 就是供预览的帖子内容，而不是文件名 - atppp */
PHP_FUNCTION(bbs_printansifile)
{
    char *filename;
    int filename_len;
    long linkmode,is_tex,is_preview;
    char *ptr;
    off_t ptrlen, mmap_ptrlen;
    const int outbuf_len = 4096;
    buffered_output_t *out;
    char* attachlink;
    int attachlink_len;
    char attachdir[MAXPATH];

    if (ZEND_NUM_ARGS() == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "s", &filename, &filename_len) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        linkmode = 1;
        attachlink=NULL;
        is_tex=is_preview=0;
    } else if (ZEND_NUM_ARGS() == 2) {
        if (zend_parse_parameters(2 TSRMLS_CC, "sl", &filename, &filename_len, &linkmode) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        attachlink=NULL;
        is_tex=is_preview=0;
    } else if (ZEND_NUM_ARGS() == 3) {
        if (zend_parse_parameters(3 TSRMLS_CC, "sls", &filename, &filename_len, &linkmode,&attachlink,&attachlink_len) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        is_tex=is_preview=0;
    } else {
        if (zend_parse_parameters(5 TSRMLS_CC, "slsll", &filename, &filename_len, &linkmode,&attachlink,&attachlink_len,&is_tex,&is_preview) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
    }
    if (!is_preview) {
        if (safe_mmapfile(filename, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &mmap_ptrlen, NULL) == 0)
            RETURN_LONG(-1);

        ptrlen = mmap_ptrlen;
    } else {
        ptr = filename;
        ptrlen = filename_len;
        getattachtmppath(attachdir, MAXPATH, getSession());
    }
	if ((out = alloc_output(outbuf_len)) == NULL)
	{
		if (!is_preview) end_mmapfile(ptr, mmap_ptrlen, -1);
        RETURN_LONG(2);
	}
/*
	override_default_output(out, buffered_output);
	override_default_flush(out, flush_buffer);
*/
	/*override_default_output(out, new_buffered_output);
	override_default_flush(out, new_flush_buffer);*/
	override_default_write(out, new_write);

	output_ansi_html(ptr, ptrlen, out, attachlink, is_tex, is_preview ? attachdir : NULL);
	free_output(out);
    if (!is_preview) end_mmapfile(ptr, mmap_ptrlen, -1);
	RETURN_STRINGL(get_output_buffer(), get_output_buffer_len(),1);
}

PHP_FUNCTION(bbs_print_article)
{
    char *filename;
    int filename_len;
    long linkmode;
    char *ptr;
    off_t mmap_ptrlen;
    const int outbuf_len = 4096;
    buffered_output_t *out;
    char* attachlink;
    int attachlink_len;

    if (ZEND_NUM_ARGS() == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "s", &filename, &filename_len) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        linkmode = 1;
        attachlink=NULL;
    } else if (ZEND_NUM_ARGS() == 2) {
        if (zend_parse_parameters(2 TSRMLS_CC, "sl", &filename, &filename_len, &linkmode) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        attachlink=NULL;
    } else {
        if (zend_parse_parameters(3 TSRMLS_CC, "sls", &filename, &filename_len, &linkmode,&attachlink,&attachlink_len) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
    }
    if (safe_mmapfile(filename, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &mmap_ptrlen, NULL) == 0)
        RETURN_LONG(-1);
	if ((out = alloc_output(outbuf_len)) == NULL)
	{
		end_mmapfile(ptr, mmap_ptrlen, -1);
        RETURN_LONG(2);
	}

	override_default_write(out, zend_write);

	output_ansi_text(ptr, mmap_ptrlen, out, attachlink);
	free_output(out);
    end_mmapfile(ptr, mmap_ptrlen, -1);
}

PHP_FUNCTION(bbs_print_article_js)
{
    char *filename;
    int filename_len;
    long linkmode;
    char *ptr;
    off_t mmap_ptrlen;
    const int outbuf_len = 4096;
    buffered_output_t *out;
    char* attachlink;
    int attachlink_len;

    if (ZEND_NUM_ARGS() == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "s", &filename, &filename_len) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        linkmode = 1;
        attachlink=NULL;
    } else if (ZEND_NUM_ARGS() == 2) {
        if (zend_parse_parameters(2 TSRMLS_CC, "sl", &filename, &filename_len, &linkmode) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        attachlink=NULL;
    } else {
        if (zend_parse_parameters(3 TSRMLS_CC, "sls", &filename, &filename_len, &linkmode,&attachlink,&attachlink_len) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
    }
    if (safe_mmapfile(filename, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &mmap_ptrlen, NULL) == 0)
        RETURN_LONG(-1);
	if ((out = alloc_output(outbuf_len)) == NULL)
	{
		end_mmapfile(ptr, mmap_ptrlen, -1);
        RETURN_LONG(2);
	}

	override_default_write(out, zend_write);

	output_ansi_javascript(ptr, mmap_ptrlen, out, attachlink);
	free_output(out);
    end_mmapfile(ptr, mmap_ptrlen, -1);
}


/* function bbs_printoriginfile(string board, string filename);
 * 输出原文内容供编辑
 */
PHP_FUNCTION(bbs_printoriginfile)
{
    char *board,*filename;
    int boardLen,filenameLen;
    FILE* fp;
    const int outbuf_len = 4096;
	char buf[512],path[512];
    buffered_output_t *out;
	int i;
	int skip;
	const boardheader_t* bp;

    if ((ZEND_NUM_ARGS() != 2) || (zend_parse_parameters(2 TSRMLS_CC, "ss", &board,&boardLen, &filename,&filenameLen) != SUCCESS)) {
		WRONG_PARAM_COUNT;
    } 
	if ( (bp=getbcache(board))==0) {
		RETURN_LONG(-1);
	}
	setbfile(path, bp->filename, filename);
    fp = fopen(path, "r");
    if (fp == 0)
        RETURN_LONG(-1); //文件无法读取
	if ((out = alloc_output(outbuf_len)) == NULL)
	{
        RETURN_LONG(-2);
	}
	override_default_write(out, zend_write);
	/*override_default_output(out, buffered_output);
	override_default_flush(out, flush_buffer);*/
	
	i=0;    
	skip=0;
    while (skip_attach_fgets(buf, sizeof(buf), fp) != 0) {
		i++;
        if (Origin2(buf))
            break;
		if ((i==1) && (strncmp(buf,"发信人",6)==0)) {
			skip=1;
		}
		if ((skip) && (i<=4) ){
			continue;
		}
        if (strstr(buf,"\033[36m※ 修改:·")==buf) {
            continue;
        }
        if (!strcasestr(buf, "</textarea>"))
		{
			int len = strlen(buf);
            BUFFERED_OUTPUT(out, buf, len);
		}
    }
    fclose(fp);
	BUFFERED_FLUSH(out);
	free_output(out);
    RETURN_LONG(0);
}


/* function bbs_originfile(string board, string filename);
 * 返回原文内容供编辑 modified from the function above by pig2532
 解决很猪的问题之二
 */
PHP_FUNCTION(bbs_originfile)
{
    char *board,*filename;
    int boardLen,filenameLen;
    FILE* fp;
	char buf[512],path[512];
    char *content, *ptr;
    int chunk_size=51200, calen, clen, buflen;
	int i;
	int skip;
	const boardheader_t* bp;

    if ((ZEND_NUM_ARGS() != 2) || (zend_parse_parameters(2 TSRMLS_CC, "ss", &board,&boardLen, &filename,&filenameLen) != SUCCESS)) {
		WRONG_PARAM_COUNT;
    } 
	if ( (bp=getbcache(board))==0) {
		RETURN_LONG(-1);
	}
	setbfile(path, bp->filename, filename);
    fp = fopen(path, "r");
    if (fp == 0)
        RETURN_LONG(-1); //文件无法读取
	
	i=0;    
	skip=0;
    calen = chunk_size;
    content = (char *)emalloc(calen);
    clen = 0;
    ptr = content;
    while (skip_attach_fgets(buf, sizeof(buf), fp) != 0) {
		i++;
        if (Origin2(buf))
            break;
		if ((i==1) && (strncmp(buf,"发信人",6)==0)) {
			skip=1;
		}
		if ((skip) && (i<=4) ){
			continue;
		}
        if (strstr(buf,"\033[36m※ 修改:·")==buf) {
            continue;
        }
        buflen = strlen(buf);
        if((clen + buflen) >= calen)
        {
            calen += chunk_size;
            content = (char *)erealloc(content, calen);
            ptr = content + clen;
        }
        memcpy(ptr, buf, buflen);
        clen += buflen;
        ptr += buflen;
    }
    fclose(fp);
    content[clen] = '\0';
    RETURN_STRINGL(content, clen, 0);
}


PHP_FUNCTION(bbs_decode_att_hash)
{
#ifndef DISABLE_INTERNAL_BOARD_PPMM_VIEWING
    char *info;
    int infolen;
    char *spec;
    int speclen;
    int specbytes[10];
    int totalspecbytes;
    char decoded[256];
    char md5ret[256];
    uint16_t u16;
    int i, u32, len;
    char *ptr;
    MD5_CTX md5;
    
    if ((ZEND_NUM_ARGS() != 2) || (zend_parse_parameters(2 TSRMLS_CC, "ss", &info, &infolen, &spec, &speclen) != SUCCESS)) {
		WRONG_PARAM_COUNT;
    }
    
    if (infolen > 128) {
        RETURN_FALSE;
    }
    if (speclen > sizeof(specbytes)/sizeof(int)) {
        RETURN_FALSE;
    }

    totalspecbytes = 0;
    for (i = 0; i < speclen; i++) {
        u32 = spec[i] - '0';
        if (u32 != 2 && u32 != 4) {
            RETURN_FALSE;
        }
        specbytes[i] = u32;
        totalspecbytes += u32;
    }
    memcpy(decoded, info, 9);
    len = from64tobits(decoded+9, info+9);
    if (4+totalspecbytes+4 != len) {
        RETURN_FALSE;
    }

    MD5Init(&md5);
    MD5Update(&md5, (unsigned char *) decoded, 9+4+totalspecbytes);
    MD5Final((unsigned char*)md5ret, &md5);

    if (memcmp(md5ret, decoded+9+4+totalspecbytes, 4) != 0) {
        RETURN_FALSE;
    }
    ptr = decoded+9;
    memcpy(&u32, ptr, 4); ptr+=4;
    if (time(NULL) < u32 || (time(NULL) - u32 >= 300)) {
        RETURN_FALSE;
    }

    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }

    add_next_index_stringl(return_value, info, 9, 1);
    for (i = 0; i < speclen; i++) {
        if (specbytes[i] == 2) {
            memcpy(&u16, ptr, 2); ptr += 2;
            u32 = u16;
        } else {
            memcpy(&u32, ptr, 4); ptr += 4;
        }
        add_next_index_long(return_value, u32);
    }
#else
    RETURN_FALSE;
#endif
}
