<?php
	require("funcs.php");

	html_init("gb2312");

if($loginok != 1)
	html_no_login();
else{

	if (isset($_GET["userid"])){
		$userid = $_GET["userid"];

		$lookupuser=array ();
		if( $userid=="" || bbs_getuser($userid, $lookupuser) == 0 )
			html_error_quit("该用户不存在");

		$usermodestr = bbs_getusermode($userid);
?>
<center>水木清华 -- 查询网友<hr color=green>
</center><pre>
<?php echo $lookupuser["userid"];?> (<?php echo $lookupuser["username"];?>) 共上站 <?php echo $lookupuser["numlogins"];?> 次，发表过 <?php echo $lookupuser["numposts"];?> 篇文章
上次在  [<?php echo date("D M j H:i:s Y",$lookupuser["lastlogin"]);?>] 从 [<?php echo $lookupuser["lasthost"];?>] 到本站一游。
离线时间[<?php 

if( $usermodestr!="" && $usermodestr{0}=="1" ){
	echo date("D M j H:i:s Y", $lookupuser["lastlogin"]+60+( $lookupuser["numlogins"]+$lookupuser["numposts"] )%100 );
} else if( $lookupuser["exittime"] < $lookupuser["lastlogin"] )
	echo "因在线上或非常断线不详";
else
	echo date("D M j H:i:s Y", $lookupuser["exittime"]); 

?>] 信箱：[<?php if( bbs_checknewmail($lookupuser["userid"]) ) echo "信"; else echo "  ";?>] 生命力：[<?php echo bbs_compute_user_value($lookupuser["userid"]); ?>] 身份: [<?php echo bbs_user_level_char($lookupuser["userid"]); ?>]。
<?php if( $usermodestr!="" && $usermodestr{1} != "") echo substr($usermodestr, 1); ?>
</pre>
<?php
		$plansfile = "home/".strtoupper($lookupuser["userid"]{0})."/".$lookupuser["userid"]."/plans";

		if( file_exists( $plansfile ) ){
?>
<font class="c36">个人说明档如下：<font class="c37">
<br>
<?php
			echo bbs_printansifile("home/".strtoupper($lookupuser["userid"]{0})."/".$lookupuser["userid"]."/plans");
		}else{
?>
<font class="c36">没有个人说明档<font class="c37">
<?php
		}
?>
<br><br>
<center>
<a href="/bbspstmail.php?userid=<?php echo $lookupuser["userid"];?>&title=没主题">[写信问候]</a>
<a href="/bbssendmsg.php?destid=<?php echo $lookupuser["userid"];?>">[发送讯息]</a>
<a href="bbsfadd?userid=<?php echo $lookupuser["userid"];?>">[加入好友]</a>
<a href="bbsfdel?userid=<?php echo $lookupuser["userid"];?>">[删除好友]</a>
<br>
</center>
<?php
		html_normal_quit();
?>
<?php
	}else{
?>

<center>水木清华 -- 查询网友<hr color=green>
<form action=/bbsqry.php method=get>
请输入用户名: <input name=userid maxlength=12 size=12>
<input type=submit value=查询用户>
</form><hr>
<?php
		html_normal_quit();
	}
}
?>
