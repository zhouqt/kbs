#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_smth_bbs.h"  

#include "bbs.h"
#include "bbslib.h"


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
    long ptrlen;
    int fd;
    int in_chinese = false;
    int chunk_size = 51200;
    struct stat st;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &filename_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    
    fd = open(filename, O_RDONLY);
    if (fd < 0)
        RETURN_LONG(2);
    if (fstat(fd, &st) < 0) {
        close(fd);
        RETURN_LONG(2);
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        RETURN_LONG(2);
    }
    if (st.st_size <= 0) {
        close(fd);
        RETURN_LONG(2);
    }

    ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    ptrlen = st.st_size;
    close(fd);
    if (ptr == NULL)
        RETURN_LONG(-1);

    j = ptrlen;
    if (j > chunk_size) j = chunk_size;
    output_buffer_size = 2 * j + 16;
    output_buffer = (char* )emalloc(output_buffer_size);
    output_buffer_len = 0;
    cur_ptr = ptr;
    strcpy(output_buffer + output_buffer_len, "prints('");
    output_buffer_len += 8;
    while (1) {
        for (; j >= 0 ; j--) {
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
                    else if (c != '\\' && c != '\'' && c != '\"') {
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
    munmap(ptr, st.st_size);

    RETVAL_STRINGL(output_buffer, output_buffer_len, 0);;
}

PHP_FUNCTION(bbs2_readfile_text)
{
    char *filename;
    int filename_len;
    long maxchar;
    long double_escape;
    char *output_buffer;
    int output_buffer_len, output_buffer_size, last_return = 0;
    char c;
    char *ptr, *cur_ptr;
    long ptrlen;
    int in_escape = false;
    int fd;
    char escape_seq[4][16];
    struct stat st;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll", &filename, &filename_len, &maxchar, &double_escape) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    
    fd = open(filename, O_RDONLY);
    if (fd < 0)
        RETURN_LONG(2);
    if (fstat(fd, &st) < 0) {
        close(fd);
        RETURN_LONG(2);
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        RETURN_LONG(2);
    }
    if (st.st_size <= 0) {
        close(fd);
        RETURN_LONG(2);
    }

    ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    ptrlen = st.st_size;
    close(fd);
    if (ptr == NULL)
        RETURN_LONG(-1);

    if (!maxchar) {
        maxchar = ptrlen;
    } else if (ptrlen > maxchar) {
        ptrlen = maxchar;
    }
    output_buffer_size = 2 * maxchar;
    output_buffer = (char* )emalloc(output_buffer_size);
    output_buffer_len = 0;
    cur_ptr = ptr;
    if (double_escape) {
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
            switch(c) {
                case '&':
                    strcpy(output_buffer + output_buffer_len, escape_seq[0]);
                    output_buffer_len += strlen(escape_seq[0]);
                    break;
                case '<':
                    strcpy(output_buffer + output_buffer_len, escape_seq[1]);
                    output_buffer_len += strlen(escape_seq[1]);
                    break;
                case '>':
                    strcpy(output_buffer + output_buffer_len, escape_seq[2]);
                    output_buffer_len += strlen(escape_seq[2]);
                    break;
                case '\n':
                    strcpy(output_buffer + output_buffer_len, escape_seq[3]);
                    output_buffer_len += strlen(escape_seq[3]);
                    last_return = output_buffer_len;
                    break;
                default:
                    if (c < 0 || c >= 32)
                        output_buffer[output_buffer_len++] = c;
                    break;
            }
        } else if (isalpha(c)) {
            in_escape = false;
        }
        ptrlen--; cur_ptr++;
    }

    munmap(ptr, st.st_size);

    RETVAL_STRINGL(output_buffer, last_return, 0);
}

