<?php
	require("www2-funcs.php");
	login_init();
	
	$img_subdir = "images/newstyle/";
	$blog_index = defined("SITE_SMTH") ? "index.html" : "pcmain.php";

	function display_board_list($section_names,$section_nums)
	{
?>
<div class="b1">
<?php
		$i = 0;
		foreach ($section_names as $secname)
		{
			$i++;
			$group=$i-1;
			$group2 = $yank = 0;
			$level = 0;
?>
<a href="javascript:submenu(0,0,<?php echo $group; ?>,0,0)" target="_self">
<img id="submenuimg_brd_<?php echo $group; ?>_0" src="images/close.gif" class="pm" alt="+"
></a><a href="bbsboa.php?group=<?php echo $group; ?>"><img src="images/kfolder1.gif" class="s16x16"><?php echo $secname[0]; ?></a><br/>
<div id="submenu_brd_<?php echo $group; ?>_0" class="lineback"></div>
<?php
		}
?>
<img src="images/open.gif" class="pm" alt="-"
><a href="bbsnewbrd.php"><img src="images/kfolder1.gif" class="s16x16">新开讨论区</a>
</div>
<?php
	}
	
	function display_my_favorite()
	{
?>
<div class="b1">
<?php
		$select = 0; 
		$yank = 0;
				
		if( bbs_load_favboard($select)!=-1 && $boards = bbs_fav_boards($select, 1)) 
		{
			$brd_name = $boards["NAME"]; // 英文名
			$brd_desc = $boards["DESC"]; // 中文描述
			$brd_flag = $boards["FLAG"]; 
			$brd_bid = $boards["BID"];  //版 ID 或者 fav dir 的索引值 
			$rows = sizeof($brd_name);
			
			for ($j = 0; $j < $rows; $j++)
			{
				if( $brd_bid[$j] == -1) { //空收藏目录
					echo "-空目录-";
				}
				else if ($brd_flag[$j]==-1)
				{
?>
<div class="fi">
<a href="javascript:submenu(1,<?php echo $brd_bid[$j]; ?>,0,0,0)" target="_self">
<img id="submenuimg_fav_<?php echo $brd_bid[$j]; ?>" src="images/close.gif" class="pm" alt="+"
></a><a href="bbsfav.php?select=<?php echo $brd_bid[$j]; ?>&up=-1"><img src="images/kfolder1.gif" class="s16x16"><?php echo $brd_desc[$j]; ?></a></div>
<div id="submenu_fav_<?php echo $brd_bid[$j]; ?>" class="lineback"></div>
<?php
				}
				else
				{
					$brd_link="bbsdoc.php?board=" . urlencode($brd_name[$j]);

					if( $j != $rows - 1 )
						echo "<div class='lb'><div class='mi'><a href='".$brd_link."'>".$brd_desc[$j]."</a></div></div>";
					else
						echo "<div class='lmi'><a href='".$brd_link."'>".$brd_desc[$j]."</a></div>";
				}
			}
		}
?>
</div>
<?php 
	}
	
	function display_mail_menu($userid)
	{
?>
<div class="mi"><a href="bbsnewmail.php">阅览新邮件</a></div>
<div class="mi"><a href="bbsmailbox.php?path=.DIR&title=<?php echo rawurlencode("收件箱"); ?>">收件箱</a></div>
<div class="mi"><a href="bbsmailbox.php?path=.SENT&title=<?php echo rawurlencode("发件箱"); ?>">发件箱</a></div>
<div class="mi"><a href="bbsmailbox.php?path=.DELETED&title=<?php echo rawurlencode("垃圾箱"); ?>">垃圾箱</a></div>
<?php
		//custom mailboxs
		$mail_cusbox = bbs_loadmaillist($userid);
		if ($mail_cusbox != -1)
		{
			foreach ($mail_cusbox as $mailbox)
			{
				echo "<div class=\"mi\">".
					"<a href=\"bbsmailbox.php?path=".$mailbox["pathname"]."&title=".urlencode($mailbox["boxname"])."\">".htmlspecialchars($mailbox["boxname"])."</a></div>\n";
			}
		}
?>
<div class="lmi"><a href="bbspstmail.php">发送邮件</a></div>
<?php		
	}
		
	function display_blog_menu($userid,$userfirstlogin)
	{
		$db["HOST"]=bbs_sysconf_str("MYSQLBLOGHOST");
		$db["USER"]=bbs_sysconf_str("MYSQLBLOGUSER");
		$db["PASS"]=bbs_sysconf_str("MYSQLBLOGPASSWORD");
		$db["NAME"]=bbs_sysconf_str("MYSQLBLOGDATABASE");
		
		@$link = mysql_connect($db["HOST"],$db["USER"],$db["PASS"]) ;
		if (!$link) return;
		@mysql_select_db($db["NAME"],$link);
		
		$query = "SELECT `uid` FROM `users` WHERE `username` = '".$userid."' AND `createtime`  > ".date("YmdHis",$userfirstlogin)." LIMIT 0,1 ;";
		$result = mysql_query($query,$link);
		$rows = mysql_fetch_array($result);
		@mysql_free_result($result);
		if(!$rows)
		{
?>
<div class="mi"><a href="pc/pcapp0.html">申请BLOG</a></div>
<?php
		}
		else
		{
?>
<div class="mi"><a href="pc/index.php?id=<?php echo $userid; ?>">我的Blog</a></div>
<div class="mi"><a href="pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=0">公开区</a></div>
<div class="mi"><a href="pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=1">好友区</a></div>
<div class="mi"><a href="pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=2">私人区</a></div>
<div class="mi"><a href="pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=3">收藏区</a></div>
<div class="mi"><a href="pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=4">删除区</a></div>
<div class="mi"><a href="pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=5">好友管理</a></div>
<div class="mi"><a href="pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=6">分类管理</a></div>
<div class="mi"><a href="pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=7">参数设定</a></div>
<div class="mi"><a href="pc/pcfile.php?userid=<?php echo $userid; ?>">个人空间</a></div>
<div class="mi"><a href="pc/pcmanage.php?userid=<?php echo $userid; ?>&act=post&tag=0&pid=0">添加文章</a></div>
<?php		
		}	
	}
	
	cache_header("nocache");
?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312"/>
<script src="www2-main.js"></script>
<script language="javascript">writeCssLeft();</script>
</head>
<script src="bbsleft.js"></script>
<!--[if IE]>
<style type="text/css">
.t2 {
	width: 167px;
}
</style>
<![endif]-->
<base target="f3" />
<body>
<iframe id="hiddenframe" name="hiddenframe" width="0" height="0" src="" frameborder="0" scrolling="no"></iframe>

<!--站点标志-->
<!--<img src="images/t1.gif" class="pm">-->
<center style="padding: 0.3em;font-weight:bold;font-size:120%;"><?php echo BBS_FULL_NAME; ?></center>

<div class="t2">
<?php
		if($currentuser["userid"]=="guest")
		{
?>
<form action="bbslogin.php" method="post" name="form1" target="_top" onSubmit="return fillf3(this);" class="m0">
<nobr><script language="javascript">putImage('u1.gif','alt="登录用户名" class="pm"');</script>
<input type="text" class="upinput" LENGTH="10" onMouseOver="this.focus()" onFocus="this.select()" name="id" /></nobr><br/>

<nobr><script language="javascript">putImage('u3.gif','alt="用户密码" class="pm"');</script>
<input type="password" class="upinput" LENGTH="10" name="passwd" maxlength="39" /></nobr><br />

<div class="m9">
<nobr><input type="image" name="login" src="images/l1.gif" alt="登录进站" class="m10">
<a href="bbsreg0.html" target="_top"><img src="images/l3.gif" border="0" alt="注册新用户" class="m10"></a></nobr>
</div>
</form>
<?php
		}
		else
		{
?>
<nobr><script language="javascript">putImage('u1.gif','alt="登录用户名" class="pm"');</script>
&nbsp;&nbsp;<?php echo $currentuser["userid"]; ?></nobr><br/>
<?php
		}
?>
<!--<img src="images/t2.gif" class="pm">-->
</div>

<div class="b1 m4">
	<img src="images/open.gif" class="pm" alt="-"
	><a href="<?php echo MAINPAGE_FILE; ?>"><img src="<?php echo $img_subdir; ?>m0.gif" class="sfolder">首页导读</a><br/>
	
	<img src="images/open.gif" class="pm" alt="-"
	><a href="bbs0an.php"><img src="<?php echo $img_subdir; ?>m1.gif" class="sfolder">精华区</a><br/>
	
	<a href='javascript:changemn("board");' target="_self"><img id="imgboard" src="images/close.gif" class="pm" alt="+"
	></a><a href="bbssec.php"><img src="<?php echo $img_subdir; ?>mfolder0.gif" class="sfolder">分类讨论区</a><br/>
	<div class="pp" id="divboard">
<?php
	display_board_list($section_names,$section_nums);
?>
	</div>

	<div><form action="bbssel.php" method="get" class="m0"><nobr
		><img src="images/open.gif" class="pm" alt="-"><img src="<?php echo $img_subdir; ?>m5.gif" class="sfolder"
		><input name="board" type="text" class="f2" value="搜索讨论区" size="12" onMouseOver="this.focus()" onFocus="this.select()" /> 
		<input name="submit" type="submit" value="GO" class="sgo" />
		</nobr>
	</form></div>
<?php
	if($currentuser["userid"]!="guest"){
?>
	<a href='javascript:changemn("fav");' target="_self"><img id="imgfav" src="images/close.gif" class="pm" alt="+"></a
	><a href="bbsfav.php?select=0"><img src="<?php echo $img_subdir; ?>mfolder3.gif" class="sfolder">我的收藏夹</a><br/>

	<div class="pp" id="divfav">
<?php
		display_my_favorite();
?>
	</div>

	<img src="images/open.gif" class="pm" alt="-"
	><a href="bbssfav.php?userid=<?php echo $currentuser['userid']; ?>"><img src="<?php echo $img_subdir; ?>m18.gif" class="sfolder"><?php echo FAVORITE_NAME; ?></a><br/>

<?php
	}
	if (defined("HAVE_PC"))
	{
?>
	<a href='javascript:changemn("pc");' target="_self"><img id="imgpc" src="images/close.gif" class="pm" alt="+"
	></a><a href='/pc/<?php echo $blog_index; ?>'><img src="<?php echo $img_subdir; ?>m3.gif" class="sfolder"
	>水木Blog</a><br/>

	<div class="pp" id="divpc">
<?php
		if($currentuser["userid"]!="guest")
			display_blog_menu($currentuser["userid"],$currentuser["firstlogin"]);
?>
		<div class="mi"><a href="pc/<?php echo $blog_index; ?>">Blog首页</a></div>
		<div class="mi"><a href="pc/pc.php">用户列表</a></div>
		<div class="mi"><a href="pc/pcreco.php">推荐文章</a></div>
		<div class="mi"><a href="pc/pclist.php">热门排行</a></div>
		<div class="mi"><a href="pc/pcsec.php">分类目录</a></div>
		<div class="mi"><a href="pc/pcnew.php">最新日志</a></div>
		<div class="mi"><a href="pc/pcnew.php?t=c">最新评论</a></div>
		<div class="mi"><a href="pc/pcsearch2.php">博客搜索</a></div>
		<div class="mi"><a href="pc/pcnsearch.php">日志搜索</a></div>
		<div class="mi"><a href="bbsdoc.php?board=SMTH_blog">Blog论坛</a></div>
<?php
		if ($currentuser && $currentuser["index"]) { //blog manage menu
			@include("pc/pcconf.php");
			if (isset($pcconfig["BOARD"])) {
				$brdarr = array();
				$pcconfig["BRDNUM"] = bbs_getboard($pcconfig["BOARD"], $brdarr);
				if (bbs_is_bm($pcconfig["BRDNUM"], $currentuser["index"])) {
?>
		<div class="mi"><a href="/pc/pcadmin_rec.php">Blog管理</a></div>
<?php
		}}} //blog manage menu
?>
		<div class="lmi"><a href="/pc/index.php?id=SYSOP">帮助主题</a></div>
		</div>
<?php
	} // defined(HAVE_PC)

	if($currentuser["userid"]!="guest"){
?>
	<a href='javascript:changemn("mail");' target="_self"><img id="imgmail" src="images/close.gif" class="pm" alt="+"
	></a><a href="bbsmail.php"><img src="<?php echo $img_subdir; ?>m4.gif" class="sfolder">我的信箱</a><br/>

	<div class="pp" id="divmail">
<?php
		display_mail_menu($currentuser["userid"]);
?>					
	</div>
<?php
	}
?>
	<a href='javascript:changemn("chat");' target="_self"><img id="imgchat" src="images/close.gif" class="pm" alt="+"
	><img src="<?php echo $img_subdir; ?>m10.gif" class="sfolder">谈天说地</a><br/>
	<div class="pp" id="divchat">
<?php
	if (!defined("SITE_SMTH")) { // Smth不提供在线用户列表 add by windinsn, May 5,2004
?>
		<div class="mi"><a href="bbsuser.php">在线用户</a></div>
<?php
	}
	if($currentuser["userid"]=="guest"){
?>					
		<div class="lmi"><a href="bbsqry.php">查询网友</a></div>
<?php
	}					
	else{
?>
		<div class="mi"><a href="bbsqry.php">查询网友</a></div>
		<div class="mi"><a href="bbsfriend.php">在线好友</a></div>
		<div class="mi"><a href="bbsmsg.php">查看所有讯息</a></div>
		<div class="lmi"><a href="bbssendmsg.php">发送讯息</a></div>
<?php
	}
?>	
	</div>

 	 
	<img src="images/open.gif" class="pm" alt="-" 	 
	><a href="bbsstyle.php"><img src="<?php echo $img_subdir; ?>m2.gif" class="sfolder">自定义界面</a><br/> 	 
 
<?php
	if($currentuser["userid"]!="guest")
	{
?>
	<a href='javascript:changemn("tool");' target="_self"><img id="imgtool" src="images/close.gif" class="pm" alt="+"
	><img src="<?php echo $img_subdir; ?>m11.gif" class="sfolder">个人参数设置</a><br/>

	<div class="pp" id="divtool">
<?php
		if(!($currentuser["userlevel"]&BBS_PERM_LOGINOK) )
		{
?>
		<div class="mi"><a href="bbsnew.php">新用户须知</a></div>
<?php
			if (defined("HAVE_ACTIVATION")) {
?>
		<div class="mi"><a href="bbssendacode.php">发送激活码</a></div>
<?php
			}
?>
		<div class="mi"><a href="bbsfillform.html">填写注册单</a></div>
<?php
		}
?>
		<div class="mi"><a href="bbsinfo.php">个人资料</a></div>
		<div class="mi"><a href="bbsplan.php">改说明档</a></div>
		<div class="mi"><a href="bbssig.php">改签名档</a></div>
		<div class="mi"><a href="bbspwd.php">修改密码</a></div>
		<div class="mi"><a href="bbsparm.php">修改个人参数</a></div>
		<div class="mi"><a href="bbsal.php">通讯录</a></div>
		<div class="mi"><a href="bbsnick.php">临时改昵称</a></div>
		<div class="lmi"><a href="bbsfall.php">设定好友</a></div>
	</div>
<?php
	}
	if($currentuser["userid"]!="guest"){
?>
	<img src="images/open.gif" class="pm" alt="-"
	><a href="bbslogout.php" target="_top"><img src="<?php echo $img_subdir; ?>m7.gif" class="sfolder">离开本站</a><br/>
<?php
	}
?>
</div>
<p align="center">
<a href="http://dev.kcn.cn/" target="_blank"><img src="/images/poweredby.gif" border="0" alt="Powered by KBS" /></a>
</p>
</body>
</html>
