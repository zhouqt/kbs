<?php
	/*This file shows user tools. windinsn Oct 27,2003*/
	
	require("funcs.php");
	
	function display_board_group($section_names,$section_nums,$group_name,$group_id,$totle_group,$group,$group2,$level){
		$yank = 0;
		settype($group, "integer");
		settype($group2, "integer");
		settype($yank, "integer");
		if ($group < 0)
			$group = 0;
		if ($group <= sizeof($section_nums)){
			$boards = bbs_getboards($section_nums[$group], $group2, $yank);
			$brd_name = $boards["NAME"]; // 英文名
			$brd_desc = $boards["DESC"]; // 中文描述
			$brd_flag = $boards["FLAG"]; //flag
			$brd_bid = $boards["BID"]; //flag
			$rows = sizeof($brd_name);			
?>
<table width="100%" border="0" cellspacing="0" cellpadding="1" class="b1">
<tr> 
	<td width="16" align="right">
	<DIV class=r id=divb<?php echo $level.$group_id; ?>a>
	<A href='javascript:changemn("b<?php echo $level.$group_id; ?>");'>
	<img id="imgb<?php echo $level.$group_id; ?>" src="images/close.gif" width="16" height="16" border="0" align="absmiddle"> 
	</A></DIV>
	</td>
	<td>
	<a href="/bbsboa.php?group=<?php echo $group; ?>" target="f3"><img src="images/folder1.gif" width="16" height="16" border="0" align="absmiddle"><?php echo $group_name; ?></a>
	</td>
</tr>
<tr>
<?php
	if($group_id != $totle_group - 1){
?>
	<td  width="16" background="/images/line3.gif"> </td>
<?php
	}
	else{
?>
	<td width="16"></td>
<?php
	}
?>
	<td class="b1">
	<DIV class=s id=divb<?php echo $level.$group_id; ?>>
	
<?php
			for ($j = 0; $j < $rows; $j++)	
			{
				if ($brd_flag[$j]&BBS_BOARD_GROUP){
					$brd_link="/bbsboa.php?group=" . $group . "&group2=" . $brd_bid[$j];
					$level++;
					display_board_group($section_names,$section_nums,$brd_desc[$j],$j,$rows,$group,$brd_bid[$j],$level);
					$level--;
					}
				else{
			  		$brd_link="/bbsdoc.php?board=" . urlencode($brd_name[$j]);

				if( $j != $rows-1 )
				{
?>
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<?php
				}
				else
				{
?>
&nbsp;
<img src="images/line1.gif" width="11" height="16" align="absmiddle">
<?php
				}
?>
<A href="<?php echo $brd_link; ?>" target="f3"><?php echo $brd_desc[$j]; ?></A><BR>
<?php
				}
			}
?>
</DIV>
</td>
</tr>
</table>
<?php		
		}
	}
	
	
	
	function display_board_list($section_names,$section_nums){
		$i = 0;
		foreach ($section_names as $secname){
			$i++;
			$group=$i-1;
			$group2 = $yank = 0;
			$level = 0;
			display_board_group($section_names,$section_nums,$secname[0],$group,count($section_names),$group,$group2,$level);
			}
		}
	
	
	function display_fav_group($boards,$group_name,$group_id,$totle_group,$level,$up=-1){
		$brd_name = $boards["NAME"]; // 英文名
		$brd_desc = $boards["DESC"]; // 中文描述
		$brd_flag = $boards["FLAG"]; //flag
		$brd_bid = $boards["BID"]; 
		$rows = sizeof($brd_bid);
			
?>
<table width="100%" border="0" cellspacing="0" cellpadding="1" class="b1">
<tr> 
	<td width="16" align="right"> 
	<DIV class=r id=divf<?php echo $level.$group_id; ?>a><A href='javascript:changemn("f<?php echo $level.$group_id; ?>");'> 
	<img id="imgf<?php echo $level.$group_id; ?>" src="images/close.gif" width="16" height="16" border="0" align="absmiddle"> 
	</A></DIV>
	</td>
	<td>
	<img src="images/folder1.gif" width="16" height="16" border="0" align="absmiddle"><a href="bbsfav.php?select=<?php echo $level; ?>&up=<?php echo $up; ?>" target="f3"><?php echo $group_name; ?></a>
	</td>
</tr>
<tr>
<?php
	if($group_id != $totle_group - 1){
?>
	<td background="/images/line3.gif"> </td>
<?php
	}
	else{
?>
	<td> </td>
<?php
	}
?>
	<td>
	<DIV class=s id=divf<?php echo $level.$group_id; ?>>
<?php
			for ($j = 0; $j < $rows; $j++)	
			{
				if ($brd_flag[$j]&BBS_BOARD_GROUP){
					if( bbs_load_favboard($brd_bid[$j])!=-1 && $fav_boards = bbs_fav_boards($brd_bid[$j], 1) && $brd_bid[$j]!= -1) {
	                                	$fav_boards = bbs_fav_boards($brd_bid[$j], 1);
	                                	display_fav_group($fav_boards,$brd_desc[$j],$j,$rows,$brd_bid[$j],$level);
	                                	}
	                                }
				else{
			  		$brd_link="/bbsdoc.php?board=" . urlencode($brd_name[$j]);

				if( $j != $rows-1 )
				{
?>
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<?php
				}
				else
				{
?>
&nbsp;
<img src="images/line1.gif" width="11" height="16" align="absmiddle">
<?php
				}
?>
<A href="<?php echo $brd_link; ?>" target="f3"><?php echo $brd_desc[$j]; ?></A><BR>
<?php
				}
			}
?>
</DIV>
</td>
</tr>
</table>
<?php		
	}
	
	
	
	function display_my_favorite(){
 		$select = -1; 
 		$yank = 0;
 		 		
                if( bbs_load_favboard($select)!=-1 && $boards = bbs_fav_boards($select, 1)) {
			$brd_name = $boards["NAME"]; // 英文名
	                $brd_desc = $boards["DESC"]; // 中文描述
	                $brd_flag = $boards["FLAG"]; 
	                $brd_bid = $boards["BID"];  //版 ID 或者 fav dir 的索引值 
        		$rows = sizeof($brd_name);
                	
                	for ($j = 0; $j < $rows; $j++)	
                        {
				if ($brd_flag[$j]==-1){//&BBS_BOARD_GROUP){
					$fav_boards = bbs_fav_boards($brd_bid[$j], 1); 
	                                display_fav_group($fav_boards,$brd_desc[$j],$j,$rows,$brd_bid[$j]);
					}
				else{
			  		$brd_link="/bbsdoc.php?board=" . urlencode($brd_name[$j]);

				if( $j != $rows-1 )
				{
?>
&nbsp;
<img src="images/line.gif" width="11" height="16" align="absmiddle">
<?php
				}
				else
				{
?>
&nbsp;
<img src="images/line1.gif" width="11" height="16" align="absmiddle">
<?php
				}
?>
<A href="<?php echo $brd_link; ?>" target="f3"><?php echo $brd_desc[$j]; ?></A><BR>
<?php
				}
			}
                        bbs_release_favboard(); 
                        
                	}
               
		}	
		
	if ($loginok != 1)
		html_nologin();
	else{
		html_init("gb2312","","",9);

?>
<script src="bbsleft.js"></script>
<body>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
<tr>
	<td>
		<table width="100%" border="0" cellspacing="0" cellpadding="0">
		<tr>
			<td colspan="2" background="/images/back1.gif">
			<img src="/images/t1.gif" border="0">
			</td>
		</tr>
		<tr>
			<td align="center" width="10%" class="t2" height="25" valign="middle">
			&nbsp;&nbsp;
			<img src="/images/u1.gif" border="0" alt="登录用户名" align="absmiddle" width="54" height="21">
			</td>
			<td align="left" class="t2">
			&nbsp;&nbsp;
			<?php	
				if($currentuser["userid"]=="guest")
					echo "游客参观";
				else
					echo $currentuser["userid"];	?>
			</td>
		</tr>
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
			<td><a href="/mainpage.php" target="f3"><img src="/images/home.gif" border="0" alt="首页" align="absmiddle"> 首页导读</a></td>
		</tr>
		<tr>
			<td width="16"><img src="/images/open.gif" border="0"></td>
			<td><a href="/cgi-bin/bbs/bbs0an" target="f3"><img src="/images/t3.gif" border="0" alt="精华公布栏" align="absmiddle"> 精华公布栏</a></td>
		</tr>
		<tr>
			<td width="16">
				<DIV class="r" id="divexpa">
				<a href='javascript:changemn("exp");'><img id="imgexp" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td><img src="/images/t7.gif" border="0" alt="水木特刊Web版" align="absmiddle"> 水木特刊Web版</a></td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<DIV class="s" id="divexp">
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
		
<?php
	if($currentuser["userid"]!="guest"){
		if (bbs_getmailnum($currentuser["userid"],$total,$unread, 0, 0)) {
			if( $unread != 0 ){
?>
<script>alert('您有新信件!')</script>
<?php
			}
		}
?>
		<tr>
			<td width="16">
				<DIV class="r" id="divtoola">
				<a href='javascript:changemn("tool");'><img id="imgtool" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td><img src="/images/t4.gif" border="0" alt="个人工具箱" align="absmiddle"> 个人工具箱</td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<DIV class="s" id="divtool">
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">&nbsp;
					<a href="/bbsfillform.html" target="f3">填写注册单</a><br>
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
					<a href="cgi-bin/bbs/bbspwd" target="f3">修改密码</a><br>
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
		<tr>
			<td width="16">
				<DIV class="r" id="divmaila">
				<a href='javascript:changemn("mail");'><img id="imgmail" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td><a href="/bbsmailbox.php" target="f3"><img src="/images/t5.gif" border="0" alt="处理信件" align="absmiddle"> 处理信件</a></td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<DIV class="s" id="divmail">
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">&nbsp;
					<a href="/bbsnewmail.php" target="f3">阅览新邮件</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbsreadmail.php" target="f3">所有邮件</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbsmailbox_system.php" target="f3">系统预定义邮箱</a><br>
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbsmailbox_custom.php" target="f3">自定义邮箱</a><br>
					&nbsp;
					<img src="/images/line1.gif" border="0" align="absmiddle">
					<a href="/bbspstmail.php" target="f3">发送邮件</a>
				</DIV>
			</td>
		</tr>
<?php
		}
?>
		<tr>
			<td width="16">
				<DIV class="r" id="divboarda">
				<a href='javascript:changemn("board");'><img id="imgboard" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td><a href="/bbssec.php" target="f3"><img src="/images/folder4.gif" border="0" alt="分类讨论区" align="absmiddle"> 分类讨论区</a></td>
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
<?php
	if($currentuser["userid"]!="guest"){
?>
		<tr>
			<td width="16">
				<DIV class="r" id="divfava">
				<a href='javascript:changemn("fav");'><img id="imgfav" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td><a href="cgi-bin/bbs/bbsfav?select=-1" target="f3"><img src="/images/folder3.gif" border="0" alt="个人定制区" align="absmiddle"> 个人定制区</a></td>
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
		<tr>
			<td width="16">
				<DIV class="r" id="divchata">
				<a href='javascript:changemn("chat");'><img id="imgchat" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td><img src="/images/t6.gif" border="0" alt="谈天说地" align="absmiddle"> 谈天说地</td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<DIV class="s" id="divchat">
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">&nbsp;
					<a href="bbsuser.php" target="f3">在线用户</a><br>
<?php
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
<!--
		<tr>
			<td width="16">
				<DIV class="r" id="divstylea">
				<a href='javascript:changemn("style");'><img id="imgstyle" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td><img src="/images/t7.gif" border="0" alt="界面风格" align="absmiddle"> 界面风格</td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<DIV class="s" id="divstyle">
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/bbsstyle.php?s=0">大字体</a><br>
					&nbsp;
					<img src="/images/line1.gif" border="0" align="absmiddle">
					<a href="/bbsstyle.php?s=1">小字体</a><br>
				</DIV>
			</td>
		</tr>
-->
		<tr>
			<td width="16"><img src="/images/open.gif" border="0"></td>
			<form action="cgi-bin/bbs/bbssel" target="f3">
			<td>
			<img src="/images/t8.gif" border="0" alt="选择讨论区" align="absmiddle">
			<input name="board" type="text" class="f2" value="选择讨论区" size="12" onmouseover="this.focus()" onfocus="this.select()"> 
			</td>
			</form>
		</tr>
		<tr>
			<td width="16">
				<DIV class="r" id="divsera">
				<a href='javascript:changemn("ser");'><img id="imgser" src="/images/close.gif" border="0"></a>
				</DIV>
			</td>
			<td><img src="/images/t9.gif" border="0" alt="系统资讯和服务" align="absmiddle"> 系统资讯和服务</td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<DIV class="s" id="divser">
					&nbsp;
					<img src="/images/line.gif" border="0" align="absmiddle">
					<a href="/games/index.html" target="f3">休闲娱乐</a><br>
					&nbsp;
					<img src="/images/line1.gif" border="0" align="absmiddle">
					<a href="/data/fterm-smth.zip" target="_blank">Fterm下载</a><br>
				</DIV>
			</td>
		</tr>
		<tr>
			<td width="16"><img src="/images/open.gif" border="0"></td>
			<td><a href="telnet:smth.org"><img src="/images/t11.gif" border="0" alt="telnet登录" align="absmiddle"> Telnet登录</a>
		</tr>
		<tr>
			<td width="16"><img src="/images/open.gif" border="0"></td>
			<td><img src="/images/t10.gif" border="0" alt="查看帮助信息" align="absmiddle"> 查看帮助
		</tr>
		<tr>
			<td width="16"><img src="/images/open.gif" border="0"></td>
			<td><a href="/bbslogout.php" target="_top"><img src="/images/leave.gif" border="0" alt="离开本站" align="absmiddle"> 离开本站</a>
		</tr>
		
		</table>
	</td>
</tr>
</table>
<p align="center"><a href="http://www.dawning.com.cn/" target="_blank"><img src="/images/dawning.gif" width="120" height="53" border="0" alt="曙光公司"></a></p>
<?php
		html_normal_quit();
		}
?>
