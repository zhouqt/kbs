<?php
	require("inc/funcs.php");
	
	function display_board_list($section_names,$section_nums)
	{
?>
<table width="100%" border="0" cellspacing="0" cellpadding="1">
<col width="16px"/><col align="left"/>
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
<td>
<a href="javascript:submenu(0,0,<?php echo $group; ?>,0,0)">
<img id="submenuimg_brd_<?php echo $group; ?>_0" src="pic/plus.gif" border="0">
</a>
</td>
<td>
<a href="section.php?sec=<?php echo $group; ?>" target="main"><?php echo $secname[0]; ?></a>
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
</table>
<?php
	}
	
	function display_my_favorite()
	{
?>
<table width="100%" border="0" cellspacing="0" cellpadding="1">
<col width="16px"/><col align="left"/>
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
<td>
<a href="javascript:submenu(1,<?php echo $brd_bid[$j]; ?>,0,0,0)">
<img id="submenuimg_fav_<?php echo $brd_bid[$j]; ?>" src="pic/plus.gif" border="0">
</a>
</td>
<td>
<a href="favboard.php?select=<?php echo $brd_bid[$j]; ?>&up=-1" target="main"><?php echo $brd_desc[$j]; ?></a>
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
<td>·</td>
<td><nobr><a href="<?php echo "board.php?name=" . urlencode($brd_name[$j]); ?>" target="main"><?php echo $brd_desc[$j]; ?></a></nobr></td>
</tr>
<?php
                    }
                }
            }
?>
</table>
<?php
        }

        html_init();
?>
<script src="bbsleft.js" language="JavaScript"></script>
<script language="JavaScript">
<!--
    colsDefine = top.document.getElementById('mainframe').cols;
//-->
</script>
<body  topmargin="0" leftmargin="0" onMouseOver='doMouseOver()' onMouseEnter='doMouseOver()' onMouseOut='doMouseOut()'>
<!--
    站点标题图片等等。请有需要的站点自行修改 - atppp
-->
<br/>
<form method="post" target="main" action="logon.php">
<?php
    /* 登录注销部分，功能测试通过，但是好像显得多余，所以注释掉。有需要的站点自己打开，注意 CSS 风格没有加！- atppp */
    /*
    if ($loginok != 1) {
?>
&nbsp;帐号：<input name="id" type="text" size="12"/><br/>
&nbsp;密码：<input name="passwd" type="password" size="12"/><br/>
&nbsp;&nbsp;&nbsp;<input type="submit" value="登录"/>&nbsp;&nbsp;<input type="button" value="注册" onclick="javascript:top.main.location.href='register.php'"/>
<?php
    } else {
?>
&nbsp;&nbsp;欢迎 <?php echo $currentuser["userid"]; ?> <a href="logout.php" target="main">注销</a>
<?php
    }
    */
    /* 登录注销部分结束 */
?>
</form>
<table border="0" cellspacing="0" cellpadding="1" width="100%">
		<col width="16px"/><col align="left"/>
		<tr>
			<td>·</td>
			<td><a href="elite.php" target="main"> 精华区</a></td>
		</tr>
		<tr>
			<td>·</td>
			<td><a href="index.php" target="main"> 全部讨论区</a></td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<div id="divboard">
<?php
	display_board_list($section_names,$section_nums);
?>
				</div>
			</td>
		</tr>
		<tr>
			<td>·</td>
			<form action="searchboard.php" method="get" target="main">
			<td>
			    <table width="100px"><tr><td>
			        <input name="board" class="TableBorder2" type="text" value="搜索讨论区" size="12" onmouseover="this.focus()" onfocus="this.select()" />
			    </td></tr></table>
			</td>
			</form>
		</tr>
<?php
	if($loginok==1){
?>
		<tr>
			<td>
				<div id="divfava">
				<a href='javascript:changemn("fav");'><img id="imgfav" src="pic/plus.gif" border="0"></a>
				</div>
			</td>
			<td><a href="favboard.php" target="main"> 我的收藏夹</a></td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<div class="s" id="divfav">
<?php
	display_my_favorite();
?>
				</div>
			</td>
		</tr>
		<tr>
			<td>·</td>
			<td><a href="usermailbox.php?boxname=inbox" target="main"> 我的信箱</a></td>
		</tr>
		<tr>
			<td>
				<div id="divmanagea">
				<a href='javascript:changemn("manage");'><img id="imgmanage" src="pic/plus.gif" border="0"></a>
				</div>
			</td>
			<td>
			<a href='javascript:changemn("manage");'> 用户功能</a>
			</td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<div class="s" id="divmanage">
					<table width="100%" border="0" cellspacing="0" cellpadding="1"><col width="16px"/><col align="left"/>
                        <tr><td>·</td><td><a href="usermanagemenu.php" target="main">用户控制面板</a></td></tr>
                        <tr><td>·</td><td><a href="modifyuserdata.php" target="main">基本资料修改</a></td></tr>
    					<tr><td>·</td><td><a href="userparam.php" target="main">用户自定义参数</a></td></tr>
	    				<tr><td>·</td><td><a href="bbssig.php" target="main">用户签名档</a></td></tr>
	    				<tr><td>·</td><td><a href="changepasswd.php" target="main">修改密码</a></td></tr>
                    </table>
				</div>
			</td>
		</tr>
<?php
	}
?>
		<tr>
			<td>
				<div id="divchata">
				<a href='javascript:changemn("chat");'><img id="imgchat" src="pic/plus.gif" border="0"></a>
				</div>
			</td>
			<td>
			<a href='javascript:changemn("chat");'> 谈天说地</a>
			</td>
		</tr>
		<tr>
			<td> </td>
			<td>
				<div class="s" id="divchat">
					<table width="100%" border="0" cellspacing="0" cellpadding="1"><col width="16px"/><col align="left"/>
<?php
	if($loginok == 1){
?>
                        <tr><td>·</td><td><a href="showmsgs.php" target="main">察看在线短信</a></td></tr>
                        <tr><td>·</td><td><a href="javascript:sendMsg()" target="main">发短信</a></td></tr>
<?php
    if (SMS_SUPPORT) {
?>
                        <tr><td>·</td><td><a href="javascript:sendSMSMsg()" target="main">发送手机短信</a></td></tr>
<?php
    }
?>
                        <tr><td>·</td><td><a href="friendlist.php" target="main">编辑好友列表</a></td></tr>
                        <tr><td>·</td><td><a href="showonlinefriend.php" target="main">在线好友</a></td></tr>
<?php
	}
?>
    					<tr><td>·</td><td><a href="showonlineuser.php" target="main">在线用户</a></td></tr>
	    				<tr><td>·</td><td><a href="dispuser.php" target="main">查询用户</a></td></tr>
                    </table>
				</div>
			</td>
		</tr>
		<tr>
			<td>·</td>
			<td><a href="javascript:switchAutoHide()" id="autoHideDiv"> 激活自动隐藏</a></td>
		</tr>
</table>
<iframe id="hiddenframe" name="hiddenframe" width="0" height="0"></iframe>
</body>
</html>