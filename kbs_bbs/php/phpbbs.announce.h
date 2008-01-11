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
PHP_FUNCTION(bbs_ann_mkdir);
PHP_FUNCTION(bbs_ann_mkfile);
PHP_FUNCTION(bbs_ann_editdir);
PHP_FUNCTION(bbs_ann_originfile);
PHP_FUNCTION(bbs_ann_editfile);
PHP_FUNCTION(bbs_ann_delete);
PHP_FUNCTION(bbs_ann_copy);
PHP_FUNCTION(bbs_ann_paste);
PHP_FUNCTION(bbs_ann_move);
PHP_FUNCTION(bbs_ipath_list);
PHP_FUNCTION(bbs_ipath_modify);
PHP_FUNCTION(bbs_ann_get_title);

#define PHP_BBS_ANNOUNCE_EXPORT_FUNCTIONS \
    PHP_FE(bbs_ann_traverse_check, NULL) \
    PHP_FE(bbs_ann_num2path, NULL) \
    PHP_FE(bbs_ann_get_board, NULL) \
    PHP_FE(bbs_getannpath, NULL) \
    PHP_FE(bbs_add_import_path,NULL) \
    PHP_FE(bbs_get_import_path,NULL) \
    PHP_FE(bbs_x_search,third_arg_force_ref_001) \
    PHP_FE(bbs_read_ann_dir,fourth_arg_force_ref_0111) \
    PHP_FE(bbs_ann_mkdir,NULL) \
    PHP_FE(bbs_ann_mkfile,NULL) \
    PHP_FE(bbs_ann_editdir,NULL) \
    PHP_FE(bbs_ann_originfile,NULL) \
    PHP_FE(bbs_ann_editfile,NULL) \
    PHP_FE(bbs_ann_delete,NULL) \
    PHP_FE(bbs_ann_copy,NULL) \
    PHP_FE(bbs_ann_paste,NULL) \
    PHP_FE(bbs_ann_move,NULL) \
    PHP_FE(bbs_ipath_list,NULL) \
    PHP_FE(bbs_ipath_modify,NULL) \
    PHP_FE(bbs_ann_get_title,NULL)


#endif //PHP_BBS_ANNOUNCE_H
