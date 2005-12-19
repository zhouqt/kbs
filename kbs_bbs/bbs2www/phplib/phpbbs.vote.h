#ifndef PHP_BBS_VOTE_H
#define PHP_BBS_VOTE_H

PHP_FUNCTION(bbs_get_votes);
PHP_FUNCTION(bbs_get_vote_from_num);
PHP_FUNCTION(bbs_vote_num);
PHP_FUNCTION(bbs_start_vote);

#define PHP_BBS_VOTE_EXPORT_FUNCTIONS \
    PHP_FE(bbs_get_votes,NULL) \
    PHP_FE(bbs_get_vote_from_num,NULL) \
    PHP_FE(bbs_vote_num,NULL) \
    PHP_FE(bbs_start_vote,NULL)

#endif //PHP_BBS_VOTE_H
