<?php
	/**
	 * This file lists articles to user.
	 * $Id$
	 */
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
		if (isset($_GET["board"]))
			$board = $_GET["board"];
		else
			html_error_quit("错误的讨论区");
		// 检查用户能否阅读该版
		$brdarr = array();
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0)
			html_error_quit("错误的讨论区");
		$usernum = $currentuser["index"];
		if (bbs_checkreadperm($usernum, $brdnum) == 0)
			html_error_quit("错误的讨论区");
		// TODO: Added bbs_check_post_perm();
		if (isset($_GET["reid"]))
			$reid = $_GET["reid"];
		else {
			$reid = 0;
		}
		settype($reid, "integer");
		if ($reid > 0)
		{
			$articles = bbs_get_records_from_id($brdarr["NAME"], $reid, 
					$dir_modes["NORMAL"]);
			if ($articles == FALSE)
			{
				html_error_quit("错误的 Re 文编号");
			}
		}
		$brd_encode = urlencode($brdarr["NAME"]);
	}
?>
<body>
<center>
<?php echo $BBS_FULL_NAME; ?> -- 发表文章 [使用者: <?php echo $currentuser["userid"]; ?>]
<hr class="default" />
<form name="postform" method="post" action="/cgi-bin/bbs/bbssnd?board=<?php echo $brd_encode; ?>&reid=<?php echo $reid; ?>">
<table border="1">
<tr>
<td>
<?php
		$top_file= bbs_get_vote_filename($brdarr["NAME"], "notes");
		$fp = fopen($top_file, "r");
		if ($fp == FALSE)
		{
			//html_error_quit("现在没有备忘录");
		}
		fclose($fp);
?>
</td>
</tr>
<tr><td>
作者: <script language="JavaScript">document.write(getCookie("UTMPUSERID"));</script><br />
使用标题: <input type="text" name="title" size="40" maxlength="100" value="">
讨论区: [<script language="JavaScript">document.write(queryString("board"));</script>]<br />
<script language="JavaScript">
<!--
    if (queryString("attach",0)==1)
    	document.write("附件：<input type=\"text\" name=\"attachname\" size=\"50\" value=\"\" disabled > ");
-->
</script>
使用签名档 <select name="signature">
<script language="JavaScript">
<!--
var num=queryString("totalsig",5);
var sel=queryString("sig",1);
if (sel>99) sel=1;
if (sel>num) num=sel;
if (sel==0)
    document.write("<option value=\"0\" selected>不使用签名档</option>");
else
    document.write("<option value=\"0\">不使用签名档</option>");
for (i=1;i<=num;i++) {
  if (i==sel)
    document.write("<option value=\""+i+"\" selected>第 "+i+" 个</option>");
  else
    document.write("<option value=\""+i+"\">第 "+i+" 个</option>");
}
-->
</script>
</select>
 [<a target="_balnk" href="bbssig.php">查看签名档</a>] 
<input type="checkbox" name="outgo" value="1">转信
<br>
<textarea name="text" rows="20" cols="80" wrap="physical">

</textarea></td></tr>
<tr><td class="post" align="center">
<input type="submit" value="发表"> 
<input type="reset" value="清除">
<script language="JavaScript">
<!--
   function GoAttachWindow(){     
	
   	var hWnd = window.open("bbsupload.php","_blank","width=600,height=300,scrollbars=yes");  

	if ((document.window != null) && (!hWnd.opener))  

		   hWnd.opener = document.window;  

	hWnd.focus();  

   	return false;  

   }  

    if (queryString("attach",0)!=0)
    	document.write("<input type=\"button\" name=\"attach22\" value=\"附件\" onclick=\"return GoAttachWindow()\">");
-->
</script>
</td></tr>
</table></form>

</html>
<?php
?>
