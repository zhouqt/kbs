<?php
    require("funcs.php");
    if ($loginok != 1)
		html_nologin();
    else
    {
        html_init("gb2312");

		if( !isset($_GET["board"]) && !isset($_POST["board"]))
			html_error_quit("错误的讨论区");
		if( isset($_GET["board"]) )
			$board = $_GET["board"];
		else
			$board = $_POST["board"];

		$brdarr = array();
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0){
			html_error_quit("错误的讨论区1");
		}
		$usernum = $currentuser["index"];
		if (bbs_checkreadperm($usernum, $brdnum) == 0){
			html_error_quit("错误的讨论区2");
		}
?>
<center>
<?php echo BBS_FULL_NAME; ?> -- 版内文章搜索 [使用者: <?php echo $currentuser["userid"];?>]
<hr color="green"><br>
<?php
		if( !isset($_GET["submit"]) && !isset($_POST["submit"]) ){
?>
<table><form action="/bbsbfind.php" method=post>
<tr><td>版面名称: <input type="text" maxlength="24" size="24" name="board" value="<?php echo $brdarr["NAME"];?>"><br>
<tr><td>标题含有: <input type="text" maxlength="50" size="20" name="title"> AND <input type="text" maxlength="50" size="20" name="title2">
<tr><td>标题不含: <input type="text" maxlength="50" size="20" name="title3">
<tr><td>作者帐号: <input type="text" maxlength="12" size="12" name="userid"><br>
<tr><td>时间范围: <input type="text" maxlength="4"  size="4"  name="dt" value="7"> 天以内<br>
<tr><td>精华文章:<input type="checkbox" name="mg">  带附件文章:<input type="checkbox" name="ag">  不含跟贴:<input type="checkbox" name="og"><br><br>
<tr><td><input type="submit" name="submit" value="递交查询结果">
</form></table>[<a href="/bbsdoc.php?board=<?php echo $brdarr["NAME"];?>">本讨论区</a>]
</html>
<?php
			html_normal_quit();
		}

		if( isset( $_POST["title"] ) ){
			$title = $_POST["title"];
		}else
			$title="";

		if( isset( $_POST["title2"] ) ){
			$title2 = $_POST["title2"];
		}else
			$title2="";

		if( isset( $_POST["title3"] ) ){
			$title3 = $_POST["title3"];
		}else
			$title3="";

		if( isset( $_POST["userid"] ) ){
			$userid = $_POST["userid"];
		}else
			$userid="";

		if( isset( $_POST["dt"] ) ){
			$dt = $_POST["dt"];
		}else
			$dt=0;
		settype($dt, "integer");
		if($dt <= 0)
			$dt = 7;
		else if($dt > 9999)
			$dt = 9999;

		if( isset( $_POST["mg"] ) ){
			$mg = $_POST["mg"];
		}else
			$mg = "";
		if($mg != "")
			$mgon=1;
		else
			$mgon=0;

		if( isset( $_POST["og"] ) ){
			$og = $_POST["og"];
		}else
			$og = "";
		if($og != "")
			$ogon=1;
		else
			$ogon=0;

		if( isset( $_POST["ag"] ) ){
			$ag = $_POST["ag"];
		}else
			$ag = "";
		if($ag != "")
			$agon=1;
		else
			$agon=0;

		$articles = bbs_search_articles($board, $title, $title2, $title3, $userid, $dt, $mgon, $ogon, $agon);

		if( $articles <= 0 ){
			html_error_quit("系统错误:".$articles);
		}
?>
查找讨论区'<?php echo $brdarr["NAME"];?>'内, 标题含: '<?php echo htmlspecialchars($title);?>'<?php if($title2!="") echo " 和'".htmlspecialchars($title2)."'";?><?php if($title3!="") echo ",不含'".htmlspecialchars($title3)."'";?> 作者为: '<?php if($userid!="") echo $userid; else echo "所有者";?>', '<?php echo $dt;?>'天以内的<?php if($mgon) echo "精华"; if($agon) echo "附件"; if($ogon) echo "主题";?>文章<br>
<table width=610>
<tr><td>编号</td><td>标记</td><td>作者</td><td>日期</td><td>标题</td></tr>
<?php
		$i=0;
		foreach ($articles as $article)
		{
			$i++;
			$flags = $article["FLAGS"];
?>
<tr><td><?php echo $article["NUM"];?></td>
<td><?php echo $flags[0]; echo $flags[3];?></td>
<td><a href="/bbsqry.php?userid=<?php echo $article["OWNER"];?>"><?php echo $article["OWNER"];?></a></td>
<td><?php echo strftime("%b&nbsp;%e", $article["POSTTIME"]); ?></td>
<td><a href="/bbscon.php?board=<?php echo $brdarr["NAME"];?>&id=<?php echo $article["ID"];?>"><?php echo htmlspecialchars($article["TITLE"]); ?></a></td></tr>
<?php
		}
?>
</table>
<br>
共找到<?php echo $i;?>篇文章符合条件<?php if($i>=999) echo "(匹配结果过多, 省略第1000以后的查询结果)";?>
<br>
[<a href="/bbsdoc.php?board=<?php echo $brdarr["NAME"];?>">返回本讨论区</a>] [<a href="javascript:history.go(-1)">返回上一页]</a>
<?php

		html_normal_quit();
    }
?>
