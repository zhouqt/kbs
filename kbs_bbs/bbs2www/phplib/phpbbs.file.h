#ifndef PHP_BBS_FILE_H
#define PHP_BBS_FILE_H

/* 具体到一个帖子文件的操作 */

PHP_FUNCTION(bbs2_readfile);
PHP_FUNCTION(bbs2_readfile_text);
PHP_FUNCTION(bbs_file_output_attachment);
PHP_FUNCTION(bbs_printansifile);
PHP_FUNCTION(bbs_print_article);
PHP_FUNCTION(bbs_print_article_js);
PHP_FUNCTION(bbs_printoriginfile);
PHP_FUNCTION(bbs_originfile);
PHP_FUNCTION(bbs_decode_att_hash);


#define PHP_BBS_FILE_EXPORT_FUNCTIONS \
    PHP_FE(bbs2_readfile, NULL) \
    PHP_FE(bbs2_readfile_text, NULL) \
    PHP_FE(bbs_file_output_attachment, NULL) \
    PHP_FE(bbs_printansifile, NULL) \
    PHP_FE(bbs_print_article, NULL) \
    PHP_FE(bbs_print_article_js, NULL) \
    PHP_FE(bbs_printoriginfile, NULL) \
    PHP_FE(bbs_originfile, NULL) \
    PHP_FE(bbs_decode_att_hash, NULL) \
    

#endif //PHP_BBS_FILE_H

