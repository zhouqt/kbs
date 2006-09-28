<?php

    $adminperm = array(
        "reg" => BBS_PERM_ACCOUNTS,
        "info" => BBS_PERM_ACCOUNTS,
        "perm" => BBS_PERM_ADMIN,
    );
    
    function admin_check($adminitem)
    {
        global $adminperm, $currentuser;
        if(!isset($adminperm[$adminitem])) {
            html_error_quit("没有这样的管理功能。");
            exit;
        }
        if(!($currentuser["userlevel"] & $adminperm[$adminitem])) {
            html_error_quit("您没有进入此管理页面的权限。");
            exit;
        }
    }

    function admin_header($title_simple, $title_full)
    {
        page_header($title_simple, "<a href=\"admindex.php\">系统管理</a>");
        print("<br><div align=\"center\"><strong>{$title_full}</strong></div><hr><br>");
    }

    require("www2-funcs.php");
    login_init();  
    assert_login();
?>
