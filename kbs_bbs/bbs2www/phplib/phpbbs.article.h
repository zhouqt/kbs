#ifndef PHP_BBS_ARTICLE_H
#define PHP_BBS_ARTICLE_H

/* 文章索引读取和查询 */

PHP_FUNCTION(bbs_get_records_from_id);
PHP_FUNCTION(bbs_get_records_from_num);
PHP_FUNCTION(bbs_get_filename_from_num);
PHP_FUNCTION(bbs_get_threads_from_id);
PHP_FUNCTION(bbs_get_threads_from_gid);



#define PHP_BBS_ARTICLE_EXPORT_FUNCTIONS \
    PHP_FE(bbs_get_records_from_id, NULL) \
    PHP_FE(bbs_get_records_from_num, NULL) \
    PHP_FE(bbs_get_filename_from_num, NULL) \
    PHP_FE(bbs_get_threads_from_id, NULL) \
    PHP_FE(bbs_get_threads_from_gid, fifth_arg_force_ref_00011)

#endif //PHP_BBS_ARTICLE_H

