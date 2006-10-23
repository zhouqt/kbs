<?php
    require("www2-admin.php");
   
    $adminmenu = array(
        "reg" => "设定使用者注册资料",
        "info" => "修改用户数据",
        "perm" => "更改使用者权限",
        "newbrd" => "开设一个新的讨论区",
        "editbrd" => "修改讨论区说明与设定"
    );
   
    admin_header("管理菜单", "系统管理功能表");
    $count = 0;
    foreach($adminperm as $key => $value) {
        if($currentuser["userlevel"] & $adminperm[$key]) {
            print("<a href=\"adm{$key}.php\">{$adminmenu[$key]}</a><br>");
        }
    }
    print("<br>");
    page_footer();
?>
