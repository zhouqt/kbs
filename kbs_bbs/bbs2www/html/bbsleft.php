<?php
	/*This file shows user tools. windinsn Oct 27,2003*/
	
	require("funcs.php");
login_init();
	
        $img_subdir = "/images/menuspring/";
        
	function display_board_list($section_names,$section_nums)
	{
?>
<table width="100%" border="0" cellspacing="0" cellpadding="1" class="b1">
<?php
		$i = 0;
		foreach ($section_names as $secname)
		{
			$i++;
			$group=$i-1;
			$group2 = $yank = 0;
			$level = 0;
?>
<tr>
<td align="right" width="16">
<a href="javascript:submenu(0,0,<?php echo $group; ?>,0,0)">
<img id="submenuimg_brd_<?php echo $group; ?>_0" src="/images/close.gif" border="0">
</a>
</td>
<td>
<a href="/bbsboa.php?group=<?php echo $group; ?>" target="f3"><img src="/images/kfolder1.gif" width="16" height="16" border="0" align="absmiddle"><?php echo $secname[0]; ?></a>
</td>
</tr>
<tr id="submenu_brd_<?php echo $group; ?>_0" style="display:none">
<td> </td>
<td id="submenu_brd_<?php echo $group; ?>_0_td">
<DIV></DIV>
</td>
</tr>
<?php
		}
?>
<tr>
<td align="right" width="16">
<img src="/images/open.gif" border="0">
</td>
<td>
<a href="/bbsnewbrd.php" target="f3"><img src="/images/kfolder1.gif" width="16" height="16" border="0" align="absmiddle">新开讨论区</a>
</td>
</tr>
</table>
<?php
	}
	
	function display_my_favorite()
	{
?>
<table width="100%" border="0" cellspacing="0" cellpadding="0" class="b1">
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
				if ($brd_flag[$j]==-1)
				{
?>
<tr>
<td align="right" width="16">
<a href="javascript:submenu(1,<?php echo $brd_bid[$j]; ?>,0,0,0)">
<img id="submenuimg_fav_<?php echo $brd_bid[$j]; ?>" src="/images/close.gif" border="0">
</a>
</td>
<td>
<a href="/bbsfav.php?select=<?php echo $brd_bid[$j]; ?>&up=-1" target="f3"><img src="/images/kfolder1.gif" width="16" height="16" border="0" align="absmiddle"><?php echo $brd_desc[$j]; ?></a>
</td>
</tr>
<tr id="submenu_fav_<?php echo $brd_bid[$j]; ?>" style="display:none">
<td background="/images/line3.gif"> </td>
<td id="submenu_fav_<?php echo $brd_bid[$j]; ?>_td">
<DIV></DIV>
</td>
</tr>
<?php
				}
				else
				{
?>
<tr>
<td width="16" align="right">
<?php			  		
			  		$brd_link="/bbsdoc.php?board=" . urlencode($brd_name[$j]);

					if( $j != $rows-1 )
					{
?>
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<?php
					}
					else
					{
?>
<img src="images/line1.gif" width="11" height="16" align="absmiddle">
<?php
					}
?>
</td><td>
<A href="<?php echo $brd_link; ?>" target="f3"><?php echo $brd_desc[$j]; ?></A>
</td></tr>
<?php
				}
			}
                        
                }
?>
</table>
<?php     
	}
	
	function display_mail_menu($userid)
	{
?>
&nbsp;
<img src="/images/line.gif" border="0" align="absmiddle">
<a href="/bbsnewmail.php" target="f3">阅览新邮件</a><br>
&nbsp;
<img src="/images/line.gif" border="0" align="absmiddle">
<a href="/bbsmailbox.php?path=.DIR&title=<?php echo rawurlencode("收件箱"); ?>" target="f3">收件箱</a><br>
&nbsp;
<img src="/images/line.gif" border="0" align="absmiddle">
<a href="/bbsmailbox.php?path=.SENT&title=<?php echo rawurlencode("发件箱"); ?>" target="f3">发件箱</a><br>
&nbsp;
<img src="/images/line.gif" border="0" align="absmiddle">
<a href="/bbsmailbox.php?path=.DELETED&title=<?php echo rawurlencode("垃圾箱"); ?>" target="f3">垃圾箱</a><br>
<?php
		//custom mailboxs
		$mail_cusbox = bbs_loadmaillist($userid);
		if ($mail_cusbox != -1)
		{
			foreach ($mail_cusbox as $mailbox)
			{
				echo "&nbsp;\n".
					"<img src=\"/images/line.gif\" border=\"0\" align=\"absmiddle\">\n".
					"<a href=\"/bbsmailbox.php?path=".$mailbox["pathname"]."&title=".urlencode($mailbox["boxname"])."\" target=\"f3\">".htmlspecialchars($mailbox["boxname"])."</a><br>\n";
			}
		}
?>
&nbsp;
<img src="/images/line1.gif" border="0" align="absmiddle">
<a href="/bbspstmail.php" target="f3">发送邮件</a>
<?php		
	}
		
	function display_blog_menu($userid,$userfirstlogin)
	{
		$db["HOST"]=bbs_sysconf_str("MYSQLHOST");
		$db["USER"]=bbs_sysconf_str("MYSQLUSER");
		$db["PASS"]=bbs_sysconf_str("MYSQLPASSWORD");
		$db["NAME"]=bbs_sysconf_str("MYSQLSMSDATABASE");
		
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
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<A href="/pc/pcapp0.html" target="f3">申请BLOG</A><BR>
<?php
		}
		else
		{
?>
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<A href="/pc/index.php?id=<?php echo $userid; ?>" target="f3">我的Blog</A><BR>
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<A href="/pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=0" target="f3">公开区</A><BR>
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<A href="/pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=1" target="f3">好友区</A><BR>
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<A href="/pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=2" target="f3">私人区</A><BR>
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<A href="/pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=3" target="f3">收藏区</A><BR>
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<A href="/pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=4" target="f3">删除区</A><BR>
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<A href="/pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=5" target="f3">好友管理</A><BR>
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<A href="/pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=6" target="f3">分类管理</A><BR>
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<A href="/pc/pcdoc.php?userid=<?php echo $userid; ?>&tag=7" target="f3">参数设定</A><BR>
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<A href="/pc/pcfile.php?userid=<?php echo $userid; ?>" target="f3">个人空间</A><BR>
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<A href="/pc/pcmanage.php?userid=<?php echo $userid; ?>&act=post&tag=0&pid=0" target="_blank">添加文章</A><BR>
<?php		
		}	
	}
		
	if ($loginok != 1)
		html_nologin();
	else{
		html_init("gb2312","","",9);

?>
<script src="bbsleft.js"></script>
<script language="javascript">
function bbs_auto_reload() {
    top.f4.location.reload();
<?php
    if($currentuser["userid"]!="guest"){
?>
    top.fmsg.location.reload();
<?php
    }
?>
    setTimeout('bbs_auto_reload()',540000);
}
setTimeout('bbs_auto_reload()',540000);
</script>
<body  TOPMARGIN="0" leftmargin="0">
<iframe id="hiddenframe" name="hiddenframe" width="0" height="0"></iframe>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
<tr>
	<td>
		<table width="100%" border="0" cellspacing="0" cellpadding="0">
		<tr>
			<td colspan="2">
			<img src="/images/t1.gif" border="0">
			</td>
		</tr>
<?php
		if($currentuser["userid"]=="guest")
		{
?>
<form action="/bbslogin.php" method="post" name="form1" target="_top">
<tr>
			<td align="center" width="10%" class="t2" height="25" valign="middle">
			&nbsp;&nbsp;
			<img src="/images/u1.gif" border="0" alt="登录用户名" align="absmiddle" width="54" height="21">
			</td>
			<td align="left" class="t2">
			<INPUT TYPE=text STYLE="width:80px;height:18px;font-size: 12px;color: #000D3C;border-color: #718BD6;border-style: solid;border-width: 1px;background-color:  #D2E1FE;" LENGTH="10" onMouseOver="this.focus()" onFocus="this.select()" name="id" >
			</td>
</tr>
<tr>
			<td align="center" width="10%" class="t2" height="25" valign="middle">
			&nbsp;&nbsp;
			<img src="/images/u3.gif" border="0" alt="用户密码" align="absmiddle" width="54" height="21">
			</td>
			<td align="left" class="t2">
			<INPUT TYPE=password  STYLE="width:80px;height:18px;font-size: 12px;color: #000D3C;border-color: #718BD6;border-style: solid;border-width: 1px;background-color:  #D2E1FE;" LENGTH="10" name="passwd" maxlength="39">
			</td>
</tr>
<tr>
			<td align="center" width="10%" colspan="2" class="t2" height="25" valign="middle">
			<input type="image" name="login" src="/images/l1.gif" alt="登录进站">
			<a href="/bbsreg0.html" target="_top"><img src="/images/l3.gif" border="0" alt="注册新用户"></a>
			</td>
</tr>
</form>
<?php
		}
		else
		{
?>
		<tr>
			<td align="center" width="10%" class="t2" height="25" valign="middle">
			&nbsp;&nbsp;
			<img src="/images/u1.gif" border="0" alt="登录用户名" align="absmiddle" width="54" height="21">
			</td>
			<td align="left" class="t2">
			&nbsp;&nbsp;
			<?php	echo $currentuser["userid"];	?>
			</td>
		</tr>
<?php
		}
?>
		<tr>
			<td colspan="2" class="t2">
			<img src="/images/t2.gif" border="0">
			</td>
		</tr>
		</table>
	</td>
</tr>
<tr>
	<td height="5"> </td>
</tr>
<tr>
	<td align="center">
		<table width="90%" border="0" cellspacing="0" cellpadding="1" class="b1">
		<tr>
			<td width="16"><img src="/images/open.gif" border="0"></td>
			<td><a href="<?php echo MAINPAGE_FILE; ?>" target="f3"><img src="<?php echo $img_subdir; ?>m1.gif" border="0" alt="首页" align="absmiddle"> 首页导读</a></td>
		</tr>
		<tr>
			<td width="16"><img src="/images/open.gif" border="0"></td>
			<td><a href="/bbs0an.php" target="f3"><img src="<?php echo $img_subdir; ?>m2.gif" border="0" alt="精华区" align="absmiddle"> 精华区</a></td>
		</tr>
		<tr>
			<td width="16">
				<DIV class="r" id="divboarda">
				<a href='javascript:changemn("board");'><img id="imgboard" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td><a href="/bbssec.php" target="f3"><img src="<?php echo $img_subdir; ?>mfolder0.gif" border="0" alt="分类讨论区" align="absmiddle"> 分类讨论区</a></td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<DIV class="s" id="divboard">
<?php
	display_board_list($section_names,$section_nums);
?>
				</DIV>
			</td>
		</tr>
		<tr>
			<td width="16"><img src="/images/open.gif" border="0"></td>
			<form action="/bbssel.php" method="get" target="f3">
			<td><nobr>
			<img src="<?php echo $img_subdir; ?>m5.gif" border="0" alt="搜索讨论区" align="absmiddle">
			<input name="board" type="text" class="f2" value="搜索讨论区" size="12" onmouseover="this.focus()" onfocus="this.select()" /> 
<input name="submit" type="submit" value="GO" style="width:25px;height:20px;font-size: 12px;color: #ffffff;border-style: none;background-color: #718BD6;" />
			</nobr></td>
			</form>
		</tr>
<?php
	if($currentuser["userid"]!="guest"){
?>
		<tr>
			<td width="16">
				<DIV class="r" id="divfava">
				<a href='javascript:changemn("fav");'><img id="imgfav" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td><a href="bbsfav.php?select=0" target="f3"><img src="<?php echo $img_subdir; ?>mfolder3.gif" border="0" alt="我的收藏夹" align="absmiddle"> 我的收藏夹</a></td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<DIV class="s" id="divfav">
<?php
	display_my_favorite();
?>
				</DIV>
			</td>
		</tr>
<?php
		}
?>
<?php
	if (defined("HAVE_PC"))
	{
?>
		<tr>
			<td width="16">
				<DIV class="r" id="divpca">
				<a href='javascript:changemn("pc");'><img id="imgpc" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td>
			<a href='/pc/pcmain.php' target='f3'>
			<img src="<?php echo $img_subdir; ?>m3.gif" border="0" alt="<?php echo BBS_FULL_NAME; ?>Blog" align="absmiddle"> 水木Blog
			</a>
			</td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<DIV class="s" id="divpc">
<?php
		if($currentuser["userid"]!="guest")
			display_blog_menu($currentuser["userid"],$currentuser["firstlogin"]);
?>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/pc/pcmain.php" target="f3">Blog首页</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/pc/pc.php" target="f3">用户列表</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/pc/pcreco.php" target="f3">推荐文章</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/pc/pclist.php" target="f3">热门排行</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/pc/pcsec.php" target="f3">分类目录</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/pc/pcnew.php" target="f3">最新日志</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/pc/pcnew.php?t=c" target="f3">最新评论</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/pc/pcsearch2.php" target="f3">博客搜索</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/pc/pcnsearch.php" target="f3">日志搜索</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbsdoc.php?board=SMTH_blog" target="f3">Blog论坛</a><br>
					&nbsp;
					<img src="/images/line1.gif" border="0" align="absmiddle">
					<a href="/pc/index.php?id=SYSOP" target="f3">帮助主题</a><br>
				</DIV>
			</td>
		</tr>
		
<?php
	} // defined(HAVE_PC)
?>
<?php
	if($currentuser["userid"]!="guest"){
?>
		<tr>
			<td width="16">
				<DIV class="r" id="divmaila">
				<a href='javascript:changemn("mail");'><img id="imgmail" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td><a href="/bbsmail.php" target="f3"><img src="<?php echo $img_subdir; ?>m4.gif" border="0" alt="我的信箱" align="absmiddle"> 我的信箱</a></td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<DIV class="s" id="divmail">
<?php
	display_mail_menu($currentuser["userid"]);
?>					
				</DIV>
			</td>
		</tr>
<?php
		}
?>
		<tr>
			<td width="16">
				<DIV class="r" id="divchata">
				<a href='javascript:changemn("chat");'><img id="imgchat" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td>
			<a href='javascript:changemn("chat");'>
			<img src="<?php echo $img_subdir; ?>m8.gif" border="0" alt="谈天说地" align="absmiddle"> 谈天说地
			</a>
			</td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<DIV class="s" id="divchat">
<?php
    if (!defined("SITE_SMTH")) { // Smth不提供在线用户列表 add by windinsn, May 5,2004
?>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">&nbsp;
					<a href="bbsuser.php" target="f3">在线用户</a><br>
<?php
        }
	if($currentuser["userid"]=="guest"){
?>					
					&nbsp;
					<img src="/images/line1.gif" border="0" align="absmiddle">
					<a href="/bbsqry.php" target="f3">查询网友</a>
<?php
		}					
	else{
?>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbsqry.php" target="f3">查询网友</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbsfriend.php" target="f3">在线好友</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbssendsms.php" target="f3">发送短信</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbsmsg.php" target="f3">查看所有讯息</a><br>
					&nbsp;
					<img src="/images/line1.gif" border="0" align="absmiddle">
					<a href="/bbssendmsg.php" target="f3">发送讯息</a>
<?php
		}
?>	
				</DIV>
			</td>
		</tr>
		<tr>
			<td width="16"><img src="/images/open.gif" border="0"></td>
			<td><a href="/bbsstyle0.php" target="f3"><img src="<?php echo $img_subdir; ?>m6.gif" border="0" alt="界面方案" align="absmiddle"> 界面方案</a>
		</tr>
<?php
	if($currentuser["userid"]!="guest")
	{
?>
		<tr>
			<td width="16">
				<DIV class="r" id="divtoola">
				<a href='javascript:changemn("tool");'><img id="imgtool" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td>
			<a href='javascript:changemn("tool");'>
			<img src="<?php echo $img_subdir; ?>m7.gif" border="0" alt="个人参数设置" align="absmiddle"> 个人参数设置
			</a>
			</td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<DIV class="s" id="divtool">
<?php
	if(!($currentuser["userlevel"]&BBS_PERM_LOGINOK) )
	{
?>					
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbsnew.php" target="f3">新用户须知</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbssendacode.php" target="f3">发送激活码</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbsfillform.html" target="f3">填写注册单</a><br>
<?php
	}
?>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbsinfo.php" target="f3">个人资料</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="bbsplan.php" target="f3">改说明档</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="bbssig.php" target="f3">改签名档</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbspwd.php" target="f3">修改密码</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="cgi-bin/bbs/bbsparm" target="f3">修改个人参数</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbsal.php" target="f3">通讯录</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbsrsmsmsg.php" target="f3">短信管理器</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbsnick.php" target="f3">临时改昵称</a><br>
					&nbsp;
					<img src="/images/line1.gif" border="0" align="absmiddle">
					<a href="/bbsfall.php" target="f3">设定好友</a><br>
				</DIV>
			</td>
		</tr>
<?php
	}
?>
		<tr>
			<td width="16">
				<DIV class="r" id="divexpa">
				<a href='javascript:changemn("exp");'><img id="imgexp" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td>
			<a href='javascript:changemn("exp");'>
			<img src="<?php echo $img_subdir; ?>m6.gif" border="0" alt="水木特刊Web版" align="absmiddle"> 水木特刊Web版
			</a>
			</td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<DIV class="s" id="divexp">
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/express/1103/smth_express.htm" target="f3">2003年11月号</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/express/0903/smth_express.htm" target="f3">2003年9月号</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/express/0703/smth_express.htm" target="f3">2003年7月号</a><br>
					&nbsp;
					<img src="/images/line1.gif" border="0" align="absmiddle">
					<a href="/express/0603/smth_express.htm" target="f3">2003年6月号</a><br>
				</DIV>
			</td>
		</tr>
		<tr>
			<td width="16">
				<DIV class="r" id="divsera">
				<a href='javascript:changemn("ser");'><img id="imgser" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td>
			<a href='javascript:changemn("ser");'>
			<img src="<?php echo $img_subdir; ?>m7.gif" border="0" alt="文件下载及其他" align="absmiddle"> 文件下载及其他
			</a>
			</td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<DIV class="s" id="divser">
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/games/index.html" target="f3">休闲娱乐</a><br>
<?php
    if (defined("SERVICE_QUIZ")) {
?>
                    &nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/games/quiztop.php" target="f3">开心辞典</a><br>
<?php
    }
?>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/data/fterm-smth.zip" target="_blank">Fterm下载</a><br>
				<?php /*
					&nbsp;
					<img src="/images/line1.gif" border="0" align="absmiddle">
					<a href="/data/FeedDemon-rc4a.exe" target="_blank">FeedDemon下载</a><br>
				*/ ?>
				</DIV>
			</td>
		</tr>
		<tr>
			<td width="16"><img src="/images/open.gif" border="0"></td>
			<td><a href="telnet:smth.org"><img src="<?php echo $img_subdir; ?>m6.gif" border="0" alt="telnet登录" align="absmiddle"> Telnet登录</a>
		</tr>
<?php
    if (defined("SITE_SMTH")) {
        if ($currentuser["userlevel"]&BBS_PERM_SYSOP) {
?>
<script src="/bbsleftmenu.js"></script>
<?php
        }
    }
?>
<!--
		<tr>
			<td width="16"><img src="/images/open.gif" border="0"></td>
			<td><img src="/images/t10.gif" border="0" alt="查看帮助信息" align="absmiddle"> 查看帮助
		</tr>
-->
		<tr>
			<td width="16"><img src="/images/open.gif" border="0"></td>
			<td><a href="/bbslogout.php" target="_top"><img src="<?php echo $img_subdir; ?>m7.gif" border="0" alt="离开本站" align="absmiddle"> 离开本站</a>
		</tr>
		
		</table>
	</td>
</tr>
</table>
<script type="text/javascript" src="/bbsleftad.js"> </script>
<P aling="center">
<?php
		powered_by_smth();
?>
</P>
<?php
		html_normal_quit();
		}
?>
