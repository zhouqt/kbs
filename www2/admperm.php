<?php
    require("www2-admin.php");

    admin_check("perm");

    $ret = 1;
    if(isset($_POST["modifyuserid"])) {
        $userid = $_POST["modifyuserid"];
        $perm = 0;
        for($i=0; $i<30; $i++) {
            if(@$_POST["p{$i}"] == "o")
                $perm |= (1 << $i);
        }
        $ret = bbs_admin_setuserperm($userid, $perm);
        html_success_quit("修改用户权限成功。", array("<a href=\"admperm.php?userid={$userid}\">返回权限更改页面</a>"));
    }

    if(isset($_POST["userid"]))
        $userid = $_POST["userid"];
    else if(isset($_GET["userid"]))
        $userid = $_GET["userid"];
    else
        $userid = $currentuser["userid"];

    $perm = bbs_admin_getuserperm($userid);
    if(($perm == -1) || ($ret == -1))
        html_error_quit("不存在的用户。");
    else {
        $giveupperm = bbs_admin_getgiveupperm($userid);
        admin_header("改别人权限", "更改使用者的权限");
?>
<form method="post" action="admperm.php" class="medium">
<fieldset><legend>要修改的用户ID</legend><div class="inputs">
<label>ID:</label><input type="text" name="userid" value="<?php print($userid); ?>" size="12" maxlength="12">
<input type="submit" value="确定"><br>注意：如果是封禁解封，请使用封禁选单！
</div></fieldset></form>
<form method="post" action="admperm.php" class="large" onsubmit="return confirm('确实要修改用户权限吗？');">
<fieldset><legend>修改用户权限</legend><div class="inputs">
<label>用户ID:</label><input type="text" name="modifyuserid" value="<?php print($userid); ?>" size="12" readonly>
<table align="center" border="0" cellpadding="1" cellspacing="0">
<tr><td valign="top">
<?php
    for($i=0; $i<16; $i++) {
        $chk = ($perm & (1 << $i))?" checked":"";
        if($giveupperm & (1 << $i))
            print("<input type=\"checkbox\" name=\"p{$i}\" value=\"o\"{$chk}> <span style=\"color:#ff0000\">". constant("BBS_PERMSTRING{$i}") ."</span><br />");
        else
            print("<input type=\"checkbox\" name=\"p{$i}\" value=\"o\"{$chk}> ". constant("BBS_PERMSTRING{$i}") ."<br />");
    }
?>
<td width="100">&nbsp;</td>
</td><td valign="top">
<?php
    for($i=16; $i<30; $i++) {
        $chk = ($perm & (1 << $i))?" checked":"";
        if($giveupperm & (1 << $i))
            print("<input type=\"checkbox\" name=\"p{$i}\" value=\"o\"{$chk}> <span style=\"color:#ff0000\">". constant("BBS_PERMSTRING{$i}") ."</span><br />");
        else
            print("<input type=\"checkbox\" name=\"p{$i}\" value=\"o\"{$chk}> ". constant("BBS_PERMSTRING{$i}") ."<br />");
    }
?>
</td></tr>
</table><br />
<div align="center"><input type="submit" value="更改">&nbsp;<input type="reset" value="重置"></div>
</div></fieldset></form><br />
<?php
    }
    page_footer();
?>
