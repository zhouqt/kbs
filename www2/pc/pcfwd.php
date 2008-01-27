<?php
require("pcfuncs.php");

if ($loginok != 1)
	html_nologin();

html_init("gb2312");

if(!strcmp($currentuser["userid"],"guest"))
{
	html_error_quit("guest 不能转载文章!");
	exit();
}

$nid = intval($_GET["nid"]);
if(!$nid)
{
	html_error_quit("对不起，您要转载的文章不存在");
	exit();
}	

$link = pc_db_connect();
if($_POST["board"])
	$query = "SELECT nodes.uid,nid,subject,htmltag,body,users.username,corpusname ".
	         "FROM nodes,users ".
	         "WHERE nodes.uid = users.uid ".
	         "  AND access = 0 ".
	         "  AND type = 0 ".
	         "  AND nid = ".$nid." ".
	         "LIMIT 0 , 1;";
else
	$query = "SELECT uid FROM nodes WHERE access = 0 AND type = 0 AND nid = ".$nid." LIMIT 0 , 1;";
$result = mysql_query($query , $link);
$node = mysql_fetch_array($result);

if(!$node)
{
	html_error_quit("对不起，您要转载的文章不存在");
	exit();
}

if($_POST["board"])
{
	$brdarr = array();
	$brdnum = bbs_getboard($_POST["board"], $brdarr);
	if ($brdnum == 0){
		html_error_quit("错误的讨论区1");
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $brdnum) == 0){
		html_error_quit("错误的讨论区2");
	}
	if( isset($_POST["big5"]) )
		$big5 = $_POST["big5"];
	else
		$big5=0;
	settype($big5, "integer");
	
	if( isset($_POST["noansi"]) )
		$noansi = $_POST["noansi"];
	else
		$noansi=0;
	settype($noansi, "integer");

	$ret = bbs_postarticle($brdarr["NAME"], preg_replace("/\\\(['|\"|\\\])/","$1",pc_fwd_getsubject($node)), 
			preg_replace("/\\\(['|\"|\\\])/","$1",pc_fwd_getbody($node)),0,0,1, 0);
	switch ($ret) {
		case -1:
			html_error_quit("错误的讨论区名称!");
			break;
		case -2: 
			html_error_quit("本版为二级目录版!");
			break;
		case -3: 
			html_error_quit("标题为空!");
			break;
		case -4: 
			html_error_quit("此讨论区是唯读的, 或是您尚无权限在此发表文章!");
			break;		
		case -5:	
			html_error_quit("很抱歉, 你被版务人员停止了本版的post权利!");
			break;	
		case -6:
			html_error_quit("两次发文间隔过密,请休息几秒再试!");	
			break;
		case -7: 
			html_error_quit("无法读取索引文件! 请通知站务人员, 谢谢! ");
			break;
		case -8:
			html_error_quit("本文不可回复!");
			break;
		case -9:
			html_error_quit("系统内部错误, 请迅速通知站务人员, 谢谢!");
			break;
        case -21:
            html_error_quit("您的积分不符合当前讨论区的设定, 暂时无法在当前讨论区发表文章...");
            break;
	}
?>
<br /><br /><br />
<p align=center>转载成功!</p>
<p align=center>返回<a href="pccon.php?id=<?php echo $node[uid]; ?>&nid=<?php echo $node[nid]; ?>&s=all"><?php echo html_format($node[subject]); ?></a></p>
<p align=center>进入<a href="/bbsdoc.php?board=<?php echo $brdarr["NAME"]; ?>"><?php echo $brdarr["DESC"]; ?>讨论区</a></p>
<p align=center><a href="javascript:history.go(-1)">快速返回</a></p>
<?php	
}
else
{

?>
<br /><br /><center>
<form action="pcfwd.php?nid=<?php echo $nid; ?>" method="post">
请输入您要转入的讨论区：
<input type="text" name="board" size="20" maxlength="20" />
<input type="submit" value="转载" />
</form>
</center>
<?php
}
pc_db_close($link);
?>
<p align="center">
<?php
    /**
     *    水木的web代码bbslib和cgi部分是修改于NJUWWWBBS-0.9，此部分
     * 代码遵循原有的nju www bbs的版权声明（GPL）。php部分的代码（
     * phplib以及php页面）不再遵循GPL，正在考虑使用其他开放源码的版
     * 权声明（BSD或者MPL之类）。
     *
     *   希望使用水木代码的Web站点加上powered by kbs的图标.该图标
     * 位于html/images/poweredby.gif目录,链接指向http://dev.kcn.cn
     * 使用水木代码的站点可以通过dev.kcn.cn获得代码的最新信息.
     *
     */
    powered_by_smth();
?>
</p>
<?php
html_normal_quit();
?>
