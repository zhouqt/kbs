#ifndef PHP_BBS_FILE_H
#define PHP_BBS_FILE_H

PHP_FUNCTION(bbs2_readfile);
PHP_FUNCTION(bbs2_readfile_text);

#define PHP_BBS_FILE_EXPORT_FUNCTIONS \
    PHP_FE(bbs2_readfile, NULL) \
    PHP_FE(bbs2_readfile_text, NULL)

#endif //PHP_BBS_FILE_H

