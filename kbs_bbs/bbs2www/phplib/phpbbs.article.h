#ifndef PHP_BBS_ARTICLE_H
#define PHP_BBS_ARTICLE_H

PHP_FUNCTION(bbs_getattachtmppath);
PHP_FUNCTION(bbs_postarticle);

#define PHP_BBS_ARTICLE_EXPORT_FUNCTIONS \
    PHP_FE(bbs_getattachtmppath, NULL) \
    PHP_FE(bbs_postarticle,NULL)

#endif //PHP_BBS_ARTICLE_H

