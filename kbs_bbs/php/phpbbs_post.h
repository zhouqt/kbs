#ifndef PHP_BBS_POST_H
#define PHP_BBS_POST_H

/* 文章写入操作，包括发文，修改文章等 */

PHP_FUNCTION(bbs_getattachtmppath);
PHP_FUNCTION(bbs_filteruploadfilename);
PHP_FUNCTION(bbs_upload_read_fileinfo);
PHP_FUNCTION(bbs_upload_del_file);
PHP_FUNCTION(bbs_upload_add_file);
PHP_FUNCTION(bbs_attachment_add);
PHP_FUNCTION(bbs_attachment_del);
PHP_FUNCTION(bbs_attachment_list);

PHP_FUNCTION(bbs_postarticle);

PHP_FUNCTION(bbs_delpost);
PHP_FUNCTION(bbs_article_deny_modify);
PHP_FUNCTION(bbs_article_modify);
PHP_FUNCTION(bbs_updatearticle);
PHP_FUNCTION(bbs_edittitle);
PHP_FUNCTION(bbs_doforward);
PHP_FUNCTION(bbs_docross);
PHP_FUNCTION(bbs_docommend);

PHP_FUNCTION(bbs_brcaddread);
PHP_FUNCTION(bbs_brcclear);
PHP_FUNCTION(bbs2_brcdump);
PHP_FUNCTION(bbs2_brcsync);


#define PHP_BBS_POST_EXPORT_FUNCTIONS \
    PHP_FE(bbs_getattachtmppath, NULL) \
    PHP_FE(bbs_filteruploadfilename,NULL) \
    PHP_FE(bbs_upload_read_fileinfo,NULL) \
    PHP_FE(bbs_upload_del_file,NULL) \
    PHP_FE(bbs_upload_add_file,NULL) \
    PHP_FE(bbs_attachment_add,NULL) \
    PHP_FE(bbs_attachment_del,NULL) \
    PHP_FE(bbs_attachment_list,NULL) \
    PHP_FE(bbs_postarticle,NULL) \
    PHP_FE(bbs_delpost,NULL) \
    PHP_FE(bbs_article_deny_modify,NULL) \
    PHP_FE(bbs_article_modify,NULL) \
    PHP_FE(bbs_updatearticle, NULL) \
    PHP_FE(bbs_edittitle, NULL) \
    PHP_FE(bbs_doforward, NULL) \
    PHP_FE(bbs_docross,NULL) \
    PHP_FE(bbs_docommend,NULL) \
    PHP_FE(bbs_brcaddread, NULL) \
    PHP_FE(bbs_brcclear, NULL) \
    PHP_FE(bbs2_brcdump, NULL) \
    PHP_FE(bbs2_brcsync, NULL) \


#endif //PHP_BBS_POST_H

