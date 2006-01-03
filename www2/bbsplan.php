<?php
    require("www2-funcs.php");
    login_init();
    bbs_session_modify_user_mode(BBS_MODE_EDITUFILE);
    toolbox_header("说明档修改");
    assert_login();
    
    $filename=bbs_sethomefile($currentuser["userid"],"plans");
    if (isset($_POST['text'])) {
        $fp=@fopen($filename,"w+");
        if ($fp!==false) {
            fwrite($fp,str_replace("\r\n", "\n", $_POST["text"]));
            fclose($fp);
            html_success_quit($currentuser["userid"] . "说明档修改成功");
        }
        html_error_quit("系统错误");
    }
?>
<form method="post" action="bbsplan.php" class="large">
<fieldset><legend>修改说明档</legend>
<textarea name="text" onkeydown='return textarea_okd(this, event);' wrap="physical" id="sfocus">
<?php
    echo @htmlspecialchars(file_get_contents($filename));
?>
</textarea>
</fieldset>
<div class="oper">
<input type="submit" value="存盘" /> <input type="reset" value="复原" />
</div>
</form>
<?php
	page_footer();
?>
