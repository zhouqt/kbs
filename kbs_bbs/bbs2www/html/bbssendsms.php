<?php
    require("funcs.php");
    if ($loginok != 1)
		html_nologin();
    else
    {
        html_init("gb2312");

        if ($currentuser["userid"]=="guest")
			html_error_quit("匆匆过客不能发送短信，请先登录");

		if (isset($_POST["dest"])){
			$dest = $_POST["dest"];

			if(!isset($_POST["msgstr"]))
				html_error_quit("请输入信息");

			$msgstr = $_POST["msgstr"];

			$ret = bbs_send_sms($dest, $msgstr);

			if( $ret == 0 )
				echo "发送成功";
			else
				echo "发送失败".$ret;

		}else{
?>
<form action=/bbssendsms.php method=post>
对方手机号<input type=text name=dest maxlength=11><br>
信息<input type=text name=msgstr><br>
<input type=submit value=发送>
<input type=reset>
</form>
<?php
		}
	}
?>
