<?php
    require("www2-admin.php");

    admin_check("newbrd");

    if(isset($_POST["boardname"])) {
        $boardname = $_POST["boardname"];
        $ret = bbs_admin_newboard($boardname);
        switch($ret) {
        case 0:
            html_success_quit("版面 {$boardname} 开设成功，请点击下面的连接设定版面属性。", array("<a href=\"admeditbrd.php?board={$boardname}\">设定 {$boardname} 的版面属性</a>"));
            break;
        case -1:
            html_error_quit("版面名称不能为空。", array("<a href=\"admnewbrd.php\">返回版面开设界面</a>"));
            break;
        case -2:
            html_error_quit("版面名称不符合规定。", array("<a href=\"admnewbrd.php\">返回版面开设界面</a>"));
            break;
        case -3:
            html_error_quit("同名版面已经存在，请更换一个名称。", array("<a href=\"admnewbrd.php\">返回版面开设界面</a>"));
            break;
        case -4:
            html_error_quit("无法添加版面，也许已经达到系统设定的版面数量上限。", array("<a href=\"admnewbrd.php\">返回版面开设界面</a>"));
            break;
        case -5:
            html_error_quit("系统错误，请联系技术人员。", array("<a href=\"admnewbrd.php\">返回版面开设界面</a>"));
            break;
        default:
        }
    }

    admin_header("开版", "开启一个新的讨论区");
?>
<form method="post" action="admnewbrd.php" class="medium">
<fieldset><legend>开设新版面</legend><div class="inputs">
<label>版面英文名称:</label><input type="text" name="boardname" size="20" maxlength="30">
<input type="submit" value="确定"></div></fieldset></form><br>
<?php
    page_footer();
?>
