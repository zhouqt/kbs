#ifndef PHP_BBS_POST_H
#define PHP_BBS_POST_H

/* 文章写入操作，包括发文，修改文章等 */

PHP_FUNCTION(bbs_getattachtmppath);
PHP_FUNCTION(bbs_filteruploadfilename);
PHP_FUNCTION(bbs_postarticle);

PHP_FUNCTION(bbs_caneditfile);
PHP_FUNCTION(bbs_updatearticle);
PHP_FUNCTION(bbs_edittitle);
PHP_FUNCTION(bbs_doforward);
PHP_FUNCTION(bbs_docross);
PHP_FUNCTION(bbs_docommend);

PHP_FUNCTION(bbs_brcaddread);
PHP_FUNCTION(bbs_brcclear);



#define PHP_BBS_POST_EXPORT_FUNCTIONS \
    PHP_FE(bbs_getattachtmppath, NULL) \
    PHP_FE(bbs_filteruploadfilename,NULL) \
    PHP_FE(bbs_postarticle,NULL) \
    PHP_FE(bbs_caneditfile,NULL) \
    PHP_FE(bbs_updatearticle, NULL) \
    PHP_FE(bbs_edittitle, NULL) \
    PHP_FE(bbs_doforward, NULL) \
    PHP_FE(bbs_docross,NULL) \
    PHP_FE(bbs_docommend,NULL) \
    PHP_FE(bbs_brcaddread, NULL) \
    PHP_FE(bbs_brcclear, NULL)

#endif //PHP_BBS_POST_H

