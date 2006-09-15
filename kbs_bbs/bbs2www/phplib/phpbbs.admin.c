#include "php_kbs_bbs.h"

/* kbs admin function for web
 * pig2532
 */


/* bbs_admin_getnewreg(array reglist);
 */
PHP_FUNCTION(bbs_admin_getnewreg) {

    int ac;
    long i;
    zval *reglist, *element;
    FILE *fp;
    char buf[256], *p;

    ac = ZEND_NUM_ARGS();
    if(ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "a", &reglist) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if(array_init(reglist) != SUCCESS) {
        RETURN_LONG(-1);
    }

    if((fp = fopen("new_register", "r")) == NULL) {
        RETURN_LONG(0);
    }
    MAKE_STD_ZVAL(element);
    array_init(element);
    i = 0;
    while(fgets(buf, 256, fp) != NULL) {
        if(strncmp(buf, "----", 4) == 0) {
            zend_hash_index_update(Z_ARRVAL_P(reglist), i, (void*)&element, sizeof(zval*), NULL);
            i++;
            MAKE_STD_ZVAL(element);
            array_init(element);
        }
        else {
            p = strchr(buf, ':');
            if(p == NULL)
                continue;
            *p = '\0';
            add_assoc_string(element, buf, p + 1, 1);
        }
    }
    fclose(fp);
    RETURN_LONG(i);
}


