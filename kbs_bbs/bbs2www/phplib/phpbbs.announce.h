#ifndef PHP_BBS_ANNOUNCE_H
#define PHP_BBS_ANNOUNCE_H

/* 精华区读取 */

PHP_FUNCTION(bbs_ann_traverse_check);
PHP_FUNCTION(bbs_ann_num2path);
PHP_FUNCTION(bbs_ann_get_board);
PHP_FUNCTION(bbs_getannpath);
PHP_FUNCTION(bbs_add_import_path);
PHP_FUNCTION(bbs_get_import_path);
PHP_FUNCTION(bbs_x_search);
PHP_FUNCTION(bbs_read_ann_dir);

#define PHP_BBS_ANNOUNCE_EXPORT_FUNCTIONS \
    PHP_FE(bbs_ann_traverse_check, NULL) \
    PHP_FE(bbs_ann_num2path, NULL) \
    PHP_FE(bbs_ann_get_board, NULL) \
    PHP_FE(bbs_getannpath, NULL) \
    PHP_FE(bbs_add_import_path,NULL) \
    PHP_FE(bbs_get_import_path,NULL) \
    PHP_FE(bbs_x_search,third_arg_force_ref_001) \
    PHP_FE(bbs_read_ann_dir,fourth_arg_force_ref_0111)


#endif //PHP_BBS_ANNOUNCE_H
