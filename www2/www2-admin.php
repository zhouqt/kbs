<?php

    require("site.php");
    if(!defined("www2dev")) {
        print("KBS administration pages are under constuction...");
        exit;
    }

    $adminperm = array(
        "reg" => BBS_PERM_ACCOUNTS,
        "info" => BBS_PERM_ACCOUNTS,
        "perm" => BBS_PERM_ADMIN,
        "newbrd" => BBS_PERM_SYSOP,
        "editbrd" => BBS_PERM_ANNOUNCE
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

    function bbs_admin_permtable($prefix, $level, $start, $end) {
        print("<tr>");
        for($i=$start; $i<$end; $i++) {
            print("<td><span title=\"" . constant("BBS_PERMSTRING{$i}") . "\">" . substr(BBS_XPERMSTR, $i, 1) . "</span></td>");
        }
        print("</tr><tr>");
        for($i=$start; $i<$end; $i++) {
            print("<td><input type=\"checkbox\" name=\"{$prefix}{$i}\" value=\"o\"" . (($level & (1 << $i))?" checked":"") . "></td>");
        }
        print("</tr>");
    }

    function bbs_admin_resolvepermtable($prefix, $count) {
        $level = 0;
        for($i=0; $i<$count; $i++) {
            if(@$_POST["{$prefix}{$i}"] == "o") {
                $level |= 1 << $i;
            }
        }
        return($level);
    }

    require("www2-funcs.php");
    login_init();  
    assert_login();
?>
