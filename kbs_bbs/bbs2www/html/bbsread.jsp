<?
require("funcs.jsp");
$boardname=$_GET["board"];
$filename=$_GET["file"];
$num=$_GET["num"];
$currboard=array ();
$ret=bbs_getboard($boardname,$currboard);
if ($ret==0)
	return error_alert("错误的讨论区");
if (($currboard["level"]==0)||($currboard["level"] & BBS_PERM_POSTMASK)||
		($currboard["level"] & BBS_PERM_NOZAP))
	$normalboard=1;
else 
if (!($currentuser["userlevel"]&$currboard["level"]))
	return error_alert("错误的讨论区");

if (!valid_filename($filename))
	return error_alert("错误的文章名");
$fullpath=getboardfilename($boardname,$filename);
if (($modifytime=filemtime($fullpath))==FALSE)
	return error_alert("文章不存在或者已被删除");
if ($nomalboard) { //need to do cache check
	session_cache_limiter("public");
	$oldmidofied=$_SERVER["HTTP_IF_MODIFIED_SINCE"];
	if ($oldmidofied!="") {
		list($dayobweek,$day,$month,$year,$hour,$minute,$second)=
			sscanf($oldmidofied,"%s, %d %d %d %d:%d:%d");
		$oldtime=gmmktime($hour,$minute,$second,$month,$day,$year);
	} else $oldtime=0;
	if ($oldtime==$modifytime) {
		header("HTTP/1.1 304 Not Modified");
		return;
	}
	header("Last-Modified: " . gmdate("D, d M Y H:i:s", $modifytime) . "GMT");
	header("Expires: " . gmdate("D, d M Y H:i:s", $modifytime+300) . "GMT");
	header("Cache-Control: max-age=300, must-revalidate");
} else {
	header("Expires: Fri, 12 12 1930 00:00:00 GMT");
	header("Cache-Control: no-cache");
}
?>
<html>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<link rel="stylesheet" type="text/css" href="/bbs.css">
<center>
水木清华 -- 文章阅读 [讨论区: <? echo $boardname; ?>]<hr color="green"><table width="610" border="1">
<tr><td>
<pre>
<? 
	bbs_printansifile($fullpath);
?>
</pre></td></tr>
</table><hr>
[<a href="bbsfwd?board=&file=M.1020239019.i0">转寄/推荐</a>][<a href="bbsccc?board=vote&file=M.1020239019.i0">转贴</a>][<a onclick="return confirm('你真的要删除本文吗?')" href="bbsdel?board=vote&file=M.1020239019.i0">删除文章</a>][<a href="bbsedit?board=vote&file=M.1020239019.i0">修改文章</a>][<a href="bbscon?board=vote&file=M.1020228664.80&num=4100">上一篇</a>][<a href="bbsdoc?board=vote">本讨论区</a>][<a href="bbscon?board=vote&file=M.1020250273.10&num=4102">下一篇</a>][<a href="bbspst?board=vote&file=M.1020239019.i0&userid=deliver&title=Re: [通知] Mj 举办投票：关于创建国标技术交流排行榜的提案 ">回文章</a>][<a href="bbstfind?board=vote&title=[通知] Mj 举办投票：关于创建国标技术交流排行榜的提案 ">同主题阅读</a>]
</center>
</html>
