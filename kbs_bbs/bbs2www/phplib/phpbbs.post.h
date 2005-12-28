#ifndef PHP_BBS_POST_H
#define PHP_BBS_POST_H

/* 文章写入操作，包括发文，修改文章等 */

PHP_FUNCTION(bbs_getattachtmppath);
PHP_FUNCTION(bbs_filteruploadfilename);
PHP_FUNCTION(bbs_postarticle);


#define PHP_BBS_POST_EXPORT_FUNCTIONS \
    PHP_FE(bbs_getattachtmppath, NULL) \
    PHP_FE(bbs_filteruploadfilename,NULL) \
    PHP_FE(bbs_postarticle,NULL) \

#endif //PHP_BBS_POST_H

