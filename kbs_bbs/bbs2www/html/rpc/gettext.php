<?php
// $Id
/**
 * Xiaoying Hu<xyhu@mtech.cn>
 */
if (!function_exists ('setlocale')){
    function setlocale ($category, $locale) {
            
    }
}

if (!function_exists ('bindtextdomain')) {
    function bindtextdomain ($domain, $directory) {
        
    }
}

if (!function_exists ('textdomain')) {
    function textdomain ($text_domain) {
        
    }
}

if (!function_exists ('gettext')) {
    function gettext ($msg_id) {
        $msg_str = $msg_id;
        return $msg_str;
    }
}

if (!function_exists ('_')) {
    function _($msg_id) {
        return gettext ($msg_id);
    }
}

?>