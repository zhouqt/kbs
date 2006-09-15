<?php

    function admin_deny()
    {
        html_error_quit("您没有进入此管理页面的权限。");
    }

    function admin_header($title_simple, $title_full)
    {
        page_header($title_simple, "系统管理");
        print("<br><div align=\"center\"><strong>{$title_full}</strong></div><hr><br>");
    }

    require("www2-funcs.php");
    login_init();  
    assert_login();
?>
