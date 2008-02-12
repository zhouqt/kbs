<?php
	require("www2-funcs.php");
	require("www2-board.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_READING);
	
	if(isset($_GET["board"]))
		$board = $_GET["board"];
	else
		html_error_quit("错误的讨论区");
	$brdarr = array();
	$bid = bbs_getboard($board, $brdarr);
	if ($bid == 0)
		html_error_quit("错误的讨论区");
	if(isset($_GET["page"]))
		$page = $_GET["page"];
	else
		$page = 0;
	if(isset($_GET["mode"]))
		$mode = $_GET["mode"];
	else
		$mode = 0;
	if($mode!=0)
		$mode = 1;
	$clubmode = bbs_club_flag($board);
	if($clubmode == "1")
		$mode = 0;
	else if($clubmode == "2")
		$mode = 1;
	
	// here process the submit data
	if(isset($_POST["clubop"]))
	{
		$clubop = $_POST["clubop"];
		$info = $_POST["info"];
		$clubops = explode("\n", $clubop);
		$numop = count($clubops);
		for($i=0; $i<$numop; $i++)
		{
			$ret = bbs_club_write($board, trim($clubops[$i]), $mode, $info);
			switch($ret)
			{
			case -1:
				html_error_quit("错误的讨论区");
			case -2:
				html_error_quit("你不是版主");
			case -3:
				html_error_quit("没有这个俱乐部");
			}
		}
	}
	
	$pagesize = 20;		
	$userlist = array();
	$total = bbs_club_read($board, $mode, $page * $pagesize, $pagesize, $userlist);
	switch($total)
	{
	case -1:
		html_error_quit("系统错误");
	case -2:
		html_error_quit("错误的讨论区");
	case -3:
		html_error_quit("你不是版主");
	case -4:
		html_error_quit("没有这个俱乐部");
	}
	$count = count($userlist);
	
	
	$pagecount = intval($total/$pagesize);
	if($total > $pagecount*$pagesize)
		$pagecount++;
	
	bbs_board_nav_header($brdarr, "俱乐部成员管理");	
?>
<script type="text/javascript">
function clubDeleteUser(userid)
{
	document.getElementById("clubop").value += "\r\n-" + userid;
}
function submitclub()
{
	if(confirm("确定修改这些用户的俱乐部<?php echo $mode?"写":"读"; ?>权限吗？"))
	{
		form1.btnsubmit.disabled = true;
		form1.btnsubmit.value = "正在操作，请等待";
		form1.submit();
	}
}
</script>
<table class="main wide adj">
<caption>俱乐部<?php echo $mode?"写":"读"; ?>权限成员名单</caption>
<col class="center" width="80"/><col class="center" width="*"/><col class="center" width="120"/>
<tbody><tr><th>序号</th><th>用户名</th><th>操作</th></tr>
<?php
	$num = $page * $pagesize + 1;
	$ret = "";
	for($i=0; $i<$count; $i++)
	{
		$ret .= "<tr><td>{$num}</td><td>{$userlist[$i]}</td>";
		$ret .= "<td><a href=\"javascript:clubDeleteUser('{$userlist[$i]}');\">删除</a></td></tr>";
		$num++;
	}
	print($ret);
?>
</tbody></table>
<div align="right">
<?php
	$ret = "";
	for($i=1; $i<=$pagecount; $i++)
	{
		if($i != $page+1)
			$ret .= "<a href=\"bbsclub.php?board={$board}&mode={$mode}&page=".($i-1)."\">{$i}</a> ";
		else
			$ret .= "{$i} ";
	}
	$ret .= "&nbsp;&nbsp;&nbsp;当前第" . ($page+1) . "页/共{$pagecount}页";
	if($clubmode == 3)
		$ret .= " [<a href=\"bbsclub.php?board={$board}&mode=".($mode?"0":"1")."\">查看" . ($mode?"读":"写") . "权限名单</a>]";
	//$ret .= " [<a href=\"bbsclub.php?board={$board}&mode={$mode}&action=clear\">清空</a>]";
	//$ret .= " [<a href=\"#\">群信</a>]";
	$ret .= "&nbsp;&nbsp;";
	print($ret);
?>
</div>
<table width="90%" align="center">
<tr><td width="50%"><form id="form1" action="<?php echo "bbsclub.php?board={$board}&mode={$mode}" ?>" method="post">
修改成员名单：<br><div align="center"><textarea id="clubop" name="clubop" style="width:90%;height:140px"></textarea><br></div>
附加说明：<input type="text" size="20" name="info"><br><div align="center">
<input type="button" value="确定" onclick="submitclub();" id="btnsubmit"></div>
</form></td><td width="50%">
　　在左边的框中输入要操作的用户名，每行写一个，带“+”
号前缀的表示添加该用户，带“-”号前缀的表示删除该用户，不带前缀的默认为添加用户。以“#”号开头的行为注释，不起作用。用户名后可加空格并附加说明，不附加说明的用户将采用下面指定的全局附加说明。例如：
<br><br>
#this line has no effect<br>
+pig2532 这是猪猪猪<br>
-atppp<br>
fancyrabbit 兔子兔子！<br>
</td></tr>
</table><br><br>
<?php
	page_footer();
?>
