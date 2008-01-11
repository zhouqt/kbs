#ifndef PHP_BBS_VOTE_H
#define PHP_BBS_VOTE_H

/* 投票相关，模版发文相关 */

PHP_FUNCTION(bbs_get_votes);
PHP_FUNCTION(bbs_get_vote_from_num);
PHP_FUNCTION(bbs_vote_num);
PHP_FUNCTION(bbs_start_vote);
PHP_FUNCTION(bbs_get_tmpls);
PHP_FUNCTION(bbs_get_tmpl_from_num);
PHP_FUNCTION(bbs_make_tmpl_file);


#define PHP_BBS_VOTE_EXPORT_FUNCTIONS \
    PHP_FE(bbs_get_votes,NULL) \
    PHP_FE(bbs_get_vote_from_num,NULL) \
    PHP_FE(bbs_vote_num,NULL) \
    PHP_FE(bbs_start_vote,NULL) \
    PHP_FE(bbs_get_tmpls,NULL) \
    PHP_FE(bbs_get_tmpl_from_num,NULL) \
    PHP_FE(bbs_make_tmpl_file,NULL) \

#endif //PHP_BBS_VOTE_H
