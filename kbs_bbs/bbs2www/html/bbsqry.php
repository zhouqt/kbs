<?php
	require("www2-funcs.php");
	login_init();
	
	//bbs_session_modify_user_mode(BBS_MODE_QUERY);
	if (isset($_GET["userid"])){
		$userid = $_GET["userid"];

		$lookupuser=array ();
		if( $userid=="" || bbs_getuser($userid, $lookupuser) == 0 )
			html_error_quit("该用户不存在");

		$usermodestr = bbs_getusermode($userid);
		page_header($lookupuser["userid"], "<a href='bbsqry.php'>查询网友</a>");
?>
<link rel="stylesheet" type="text/css" href="ansi.css"/>
<div class="main smaller">
<pre>
<?php echo $lookupuser["userid"];?> (<?php echo htmlspecialchars($lookupuser["username"]);?>) 共上站 <?php echo $lookupuser["numlogins"];?> 次，发表过 <?php echo $lookupuser["numposts"];?> 篇文章
上次在  [<?php echo date("D M j H:i:s Y",$lookupuser["lastlogin"]);?>] 从 [<?php echo $lookupuser["lasthost"];?>] 到本站一游。
离线时间[<?php 
if( $usermodestr!="" && $usermodestr{0}=="1" ){
	echo date("D M j H:i:s Y", $lookupuser["lastlogin"]+60+( $lookupuser["numlogins"]+$lookupuser["numposts"] )%100 );
} else if( $lookupuser["exittime"] < $lookupuser["lastlogin"] )
	echo "因在线上或非常断线不详";
else
	echo date("D M j H:i:s Y", $lookupuser["exittime"]); 
?>] 信箱: [<?php
	echo (bbs_checknewmail($lookupuser["userid"])) ? "信" : "  ";
?>] 生命力: [<?php echo bbs_compute_user_value($lookupuser["userid"]); ?>] 身份: [<?php echo bbs_user_level_char($lookupuser["userid"]); ?>]。
<?php if( $usermodestr!="" && $usermodestr{1} != "") echo substr($usermodestr, 1); ?>
</pre>
<br/><span class="c36">
<?php
		$plansfile = bbs_sethomefile($lookupuser["userid"], "plans");

		if( file_exists( $plansfile ) ){
?>
个人说明档如下: </span><br/>
<?php
			$s = bbs_printansifile($plansfile);
			if (is_string($s)) echo $s;
		}else{
?>
没有个人说明档</span>
<?php
		}
?>
</div>
<div class="oper smaller">
[<a href="bbspstmail.php?userid=<?php echo $lookupuser["userid"];?>&title=没主题">写信问候</a>]
[<a href="bbssendmsg.php?destid=<?php echo $lookupuser["userid"];?>">发送讯息</a>]
[<a href="bbsfadd.php?userid=<?php echo $lookupuser["userid"];?>">加入好友</a>]
[<?php bbs_add_super_fav ('[用户] '.$lookupuser['userid'], 'bbsqry.php?userid='.$lookupuser['userid']); ?>]
[<a href="bbsfdel.php?userid=<?php echo $lookupuser["userid"];?>">删除好友</a>]
[<a href="pc/index.php?id=<?php echo $lookupuser["userid"];?>">BLOG</a>]
</div>
<?php
	} else {
		page_header("查询网友");
?>
<div class="oper">
<form action="bbsqry.php" method="get">
请输入用户名: <input name="userid" maxlength="12" size="12" type="text" id="sfocus"/>
<input type="submit" value="查询用户"/>
</form>
</div>
<?php
	}
	page_footer();
?>

