<?php
    require("www2-funcs.php");
    login_init();
    bbs_session_modify_user_mode(BBS_MODE_EDITUFILE);
    toolbox_header("签名档修改");
    assert_login();
    
    $filename=bbs_sethomefile($currentuser["userid"],"signatures");
    if (isset($_POST['text'])) {
        $fp=@fopen($filename,"w+");
        if ($fp!==false) {
            fwrite($fp,str_replace("\r\n", "\n", $_POST["text"]));
            fclose($fp);
            bbs_recalc_sig();
            html_success_quit($currentuser["userid"] . "签名档修改成功");
        }
        html_error_quit("系统错误");
    }
?>
<form method="post" action="bbssig.php" class="large">
<fieldset><legend>修改签名档 (每 6 行为一个单位，可设置多个签名档)</legend>
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
