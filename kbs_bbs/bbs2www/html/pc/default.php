<?php
require("pcfuncs.php");
require("pcstat.php");

function pcmain_blog_statistics_list()
{
	global $pcconfig;
	$query = "SELECT COUNT(*) FROM users;";
	$result = mysql_query($query);
	$rows = mysql_fetch_row($result);
	mysql_free_result($result);
	$totaluser = $rows[0];
	
	$query = "SELECT COUNT(*) FROM nodes WHERE type != 1;";
	$result = mysql_query($query);
	$rows = mysql_fetch_row($result);
	mysql_free_result($result);
	$totalnode = $rows[0];
	
	$query = "SELECT COUNT(*) FROM comments;";
	$result = mysql_query($query);
	$rows = mysql_fetch_row($result);
	mysql_free_result($result);
	$totalcomment = $rows[0];
?>
用户：<strong><?php echo $totaluser; ?></strong> 人<br />
日志：<strong><?php echo $totalnode; ?></strong> 个<br />
评论：<strong><?php echo $totalcomment; ?></strong> 篇<br />
<?php
}

function pcmain_blog_new_user()
{
	global $pcconfig,$link;
	$query = "SELECT username,corpusname,description FROM users ORDER BY createtime DESC LIMIT 0,20;";
	$result = mysql_query($query,$link);
	$num = mysql_num_rows($result);
?>
<ul>
<?php
	for($i = 0;$i<$num;$i++)
	{
		$rows = mysql_fetch_array($result);
		echo "<li><a href=\"index.php?id=".$rows[username]."\"><span title=\"".html_format($rows[description])."\">".html_format($rows[corpusname])."</span></a>&nbsp;<a href=\"/bbsqry.php?userid=".$rows[username]."\"><font class=\"low\">".$rows[username]."</font></a></li>";	
	}
?>				
</ul>
<?php
}

function pcmain_blog_top_ten()
{
	global $pcconfig,$link;
	$query = "SELECT username , corpusname , description FROM users ORDER BY visitcount DESC LIMIT 0,20;";
	$result = mysql_query($query,$link);
	$num = mysql_num_rows($result);
?>
		<ul>
<?php
	for($i = 0;$i<$num;$i++)
	{
		$rows = mysql_fetch_array($result);
		echo "<li><a href=\"index.php?id=".$rows[username]."\"><span title=\"".html_format($rows[description])."\">".html_format($rows[corpusname])."</span></a>&nbsp;<a href=\"/bbsqry.php?userid=".$rows[username]."\"><font class=\"low\">".$rows[username]."</font></a></li>";	
	}
?>				
				</ul>
<?php
}

function pcmain_blog_last_update()
{
	global $pcconfig,$link;
	$query = "SELECT username , corpusname , description FROM users WHERE createtime != modifytime ORDER BY modifytime DESC LIMIT 0,20;";
	$result = mysql_query($query,$link);
	$num = mysql_num_rows($result);
?>
				<ul>
<?php
	for($i = 0;$i<$num;$i++)
	{
		$rows = mysql_fetch_array($result);
		echo "<li><a href=\"index.php?id=".$rows[username]."\"><span title=\"".html_format($rows[description])."\">".html_format($rows[corpusname])."</span></a>&nbsp;<a href=\"/bbsqry.php?userid=".$rows[username]."\"><font class=\"low\">".$rows[username]."</font></a></li>";	
	}
?>				
				</ul>
<?php
}


function  pcmain_blog_most_hot()
{
	global $pcconfig,$link;
	$query = "SELECT nid , subject , uid FROM nodes WHERE access = 0 AND type = 0 AND recommend != 2 AND created > ".date("YmdHis",time()-604800)." ORDER BY commentcount DESC , nid DESC LIMIT 0 , 40;";
	$result = mysql_query($query,$link);
	$num = mysql_num_rows($result);
?>
<table cellspacing=0 cellpadding=3 width=100%>
<?php
	for($i = 0;$i < $num ;$i ++)
	{
		if($i % 4 == 0 ) 
		{
			echo "<tr>";
			$tdclass = "td2";
		}
		elseif($i % 4 == 2 )
		{
			echo "<tr>";
			$tdclass ="td1";
		}
		
		echo "<td class=\"".$tdclass."\" width=\"33%\">";
		$rows = mysql_fetch_array($result);
		$pcinfor = pc_load_infor($link,"",$rows[uid]);
		echo "|&nbsp;<a href=\"pccon.php?id=".$rows[uid]."&nid=".$rows[nid]."&s=all\">";
		$subject = "<span title=\"".html_format($rows[subject])."\">".html_format(substr($rows[subject],0,20));
		if(strlen($rows[subject]) > 20 )
			$subject .= "...";
		$subject .= "</span>";
		echo $subject."</a>\n".
			"&nbsp;<a href=\"index.php?id=".$pcinfor[USER]."\"><font class=low>".$pcinfor[NAME]."</font></a>&nbsp;<a href=\"/bbsqry.php?userid=".$pcinfor["USER"]."\"><font class=low2>".$pcinfor["USER"]."</font></a>\n";
		echo "</td>\n";	
		if($i % 2 == 1 ) echo "</tr>";
	}
?>				
</table>
<?php		
}

function  pcmain_blog_most_trackback()
{
	global $pcconfig,$link;
	$query = "SELECT nid , subject , uid FROM nodes WHERE access = 0 AND type = 0 AND recommend != 2 AND created > ".date("YmdHis",time()-1209600)." AND trackbackcount != 0 ORDER BY trackbackcount DESC , nid DESC LIMIT 0 , 20;";
	$result = mysql_query($query,$link);
	$num = mysql_num_rows($result);
?>
<table cellspacing=0 cellpadding=3 width=98%>
<tr><td style="line-height:16px " align="left" width="50%">
<?php
	for($i = 0;$i < $num ;$i ++)
	{
		if( $i == 10 ) echo "</td><td align=\"left\" style=\"line-height:16px\" width=\"50%\">";
		$rows = mysql_fetch_array($result);
		$pcinfor = pc_load_infor($link,"",$rows[uid]);
		echo "<li><a href=\"pccon.php?id=".$rows[uid]."&nid=".$rows[nid]."&s=all\">";
		$subject = "<span title=\"".html_format($rows[subject])."\">".html_format(substr($rows[subject],0,20));
		if(strlen($rows[subject]) > 20 )
			$subject .= "...";
		$subject .= "</span>";
		echo $subject."</a>\n".
			"&nbsp;<a href=\"index.php?id=".$pcinfor[USER]."\"><font class=low>".$pcinfor[NAME]."</font></a>&nbsp;<a href=\"/bbsqry.php?userid=".$pcinfor[USER]."\"><font class=low2>".$pcinfor[USER]."</font></a>";
		echo "</li>\n";	
	}
?>				
</td>
</tr>
</table>
<?php		
}

function  pcmain_blog_most_view()
{
	global $pcconfig,$link;
	$query = "SELECT nid , subject , uid  FROM nodes WHERE access = 0 AND type = 0 AND recommend != 2 AND created > ".date("YmdHis",time()-604800)." AND visitcount != 0 ORDER BY visitcount DESC , nid DESC LIMIT 0 , 20;";
	$result = mysql_query($query,$link);
	$num = mysql_num_rows($result);
?>
<table cellspacing=0 cellpadding=3 width=98%>
<tr><td style="line-height:16px " align="left" width="50%">
<?php
	for($i = 0;$i < $num ;$i ++)
	{
		if( $i == 10 ) echo "</tr><td style=\"line-height:16px \" align=left width=\"50%\">";
		$rows = mysql_fetch_array($result);
		$pcinfor = pc_load_infor($link,"",$rows[uid]);
		echo "<li><a href=\"pccon.php?id=".$rows[uid]."&nid=".$rows[nid]."&s=all\">";
		$subject = "<span title=\"".html_format($rows[subject])."\">".html_format(substr($rows[subject],0,20));
		if(strlen($rows[subject]) > 20 )
			$subject .= "...";
		$subject .= "</span>";
		echo $subject."</a>\n".
			"&nbsp;<a href=\"index.php?id=".$pcinfor[USER]."\"><font class=low>".$pcinfor[NAME]."</font></a>&nbsp;<a href=\"/bbsqry.php?userid=".$pcinfor[USER]."\"><font class=low2>".$pcinfor[USER]."</font></a>\n";
		echo "</li>\n";	
	}
?>
</td></tr>				
</table>
<?php		
}

function pcmain_blog_new_nodes()
{
	global $link;
	$newBlogs = getNewBlogs($link,1,90);
	$newNum = count($newBlogs[useretems]);
?>
<table cellspacing=0 cellpadding=3 width=98%>
<?php
	for($i = 0;$i < $newNum ;$i ++)
	{
		if($i % 4 == 0 ) 
		{
			echo "<tr>";
			$tdclass = "td2";
		}
		elseif($i % 4 == 2 )
		{
			echo "<tr>";
			$tdclass ="td1";
		}
		echo "<td class=".$tdclass." width=\"33%\">[<span title=\"".$newBlogs[useretems][$i][pc][DESC]."\"><a href=\"index.php?id=".$newBlogs[useretems][$i][pc][USER]."\">".$newBlogs[useretems][$i][pc][NAME]."</a></span>]".
			 "<a href='pccon.php?id=".$newBlogs[useretems][$i][pc][UID]."&tid=".$newBlogs[useretems][$i][tid]."&nid=".$newBlogs[useretems][$i][nid]."&s=all'>";
		echo "<span title=\"".$newBlogs[useretems][$i][subject]."\">";
		echo substr($newBlogs[useretems][$i][subject],0,20);
		if(strlen($newBlogs[useretems][$i][subject])>20) echo "...";
		echo "</span></a>&nbsp;<a href='/bbsqry.php?userid=".$newBlogs[useretems][$i][pc][USER]."'><font class=low>".$newBlogs[useretems][$i][pc][USER]."</font></a></td>";
		if($i % 2 == 1 ) echo "</tr>";
	}
?>
</td></tr>				
</table>
<?php	
}

function  pcmain_blog_recommend_nodes()
{
	global $pcconfig,$link;
	$query = "SELECT nid , subject , uid FROM recommend ORDER BY state DESC, rid DESC LIMIT 0 , 20;";
	$result = mysql_query($query,$link);
	$num = mysql_num_rows($result);
?>
<table cellspacing=0 cellpadding=3 width=98%>
<tr><td style="line-height:16px " align="left" width="50%">
<?php
	for($i = 0;$i < $num ;$i ++)
	{
		if( $i == 10 ) echo "</td><td align=\"left\" style=\"line-height:16px\" width=\"50%\">";
		$rows = mysql_fetch_array($result);
		$pcinfor = pc_load_infor($link,"",$rows[uid]);
		echo "<li><a href=\"pccon.php?id=".$rows[uid]."&nid=".$rows[nid]."&s=all\">".html_format($rows[subject])."</a>\n".
			"&nbsp;<a href=\"index.php?id=".$pcinfor[USER]."\"><font class=low>".$pcinfor[NAME]."</font></a>&nbsp;<a href=\"/bbsqry.php?userid=".$pcinfor["USER"]."\"><font class=low2>".$pcinfor["USER"]."</font></a>";
		echo "</li>\n";	
	}
?>
</td>
</tr>			
</table>
<?php		
}

function pcmain_blog_sections()
{
	global $pcconfig;
	$secNum = count($pcconfig["SECTION"]);
	$secKeys = array_keys($pcconfig["SECTION"]);
?>
<table width="100%" cellpadding="2" cellspacing="0">
<?php
	for($i = 0 ; $i < $secNum ; $i ++ )
	{
		if( $i % 9 == 0 ) echo "<tr>";		
		echo "<td align=center><a href=\"pcsec.php?sec=".htmlspecialchars($secKeys[$i])."\">".htmlspecialchars($pcconfig["SECTION"][$secKeys[$i]])."</a></td>";
		if( $i % 9 == 8 ) echo "</tr>";
	}
?>
	</table>
<?php
}

if(pc_update_cache_header())
	return;

$link = pc_db_connect();
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312" />
<title><?php $pcconfig["BBSNAME"]; ?>BLOG</title>
<style type="text/css">
<!--
.table {
	border-top-width: 1px;
	border-left-width: 1px;
	border-top-style: solid;
	border-left-style: solid;
	border-top-color: #999999;
	border-left-color: #999999;
}
.channel {
	font-size: 12px;
	font-weight: bold;
	color: #FFFFFF;
	text-align: left;
	text-indent: 3pt;
}
.more {
	font-size: 11px;
	color: #ffffff;
}
.channelback {
	background-color: #1F66A7;
	border-right-width: 1px;
	border-bottom-width: 1px;
	border-right-style: solid;
	border-bottom-style: solid;
	border-right-color: #999999;
	border-bottom-color: #999999;
	font-size: 12px;
	font-weight: bold;
	color: #FFFFFF;
	text-align: left;
	text-indent: 3pt;
}
.td {
	border-right-width: 1px;
	border-bottom-width: 1px;
	border-right-style: solid;
	border-bottom-style: solid;
	border-right-color: #999999;
	border-bottom-color: #999999;
	line-height: 16px;
	word-wrap:break-word;
	word-break:break-all;
}
body {
	font-size: 12px;
}
a:link {
	color: #000000;
	text-decoration: none;
}
a:visited {
	color: #003333;
	text-decoration: none;
}
a:hover {
	color: #003366;
	text-decoration: underline;
}
.topic {
	color: #666666;
	background-color: #D2E9FF;
	text-align: left;
	text-indent: 5pt;
	font-size: 12px;
	font-weight: bold;
	border: 1px solid #999999;
}
input {
	font-size: 12px;
	font-weight: lighter;
}
.textinput {
	background-color: #F6F6F6;
	border: 1px solid #999999;
}
.low {
	font-size: 12px;
	color: #666666;
}
.td1 {
	border-bottom-width: 1px;
	border-bottom-style: dashed;
	border-bottom-color: #999999;
	text-align:left;
	line-height: 16px;
	background-color: #FCFCFC;
	word-wrap:break-word;
	word-break:break-all;
}
.td2 {
	border-bottom-width: 1px;
	border-bottom-style: dashed;
	border-bottom-color: #999999;
	text-align:left;
	background-color: #F0F0F0;
	line-height: 16px;
	word-wrap:break-word;
	word-break:break-all;
}
.low2 {
	color: #3399CC;
}
-->
</style>
</head>

<body topmargin="0" leftmargin="0"><center>
<table width="750"  border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td colspan="3" align="left" background="images/bg.gif">
	<?php
	    echo $pcconfig["BBSNAME"];
	?>
	BLOG
	<a href="/pc/pcapp0.html"><font color="red">立即申请BLOG</font></a>
	</td>
    </tr>
  <tr>
    <td colspan="3"><table width="100%" border="0" cellspacing="0" cellpadding="5">
      <tr>
        <td width="180"><img src="/images/logo.gif" width="180" height="77" /></td>
        
        <td>
      	&nbsp;
        </td>
        <td width="80" align="center" valign="middle" style="line-height:25px ">
		<a href="/mainpage.html">本站首页</a>
		<br/>
		<a href="/bbsdoc.php?board=<?php echo $pcconfig["BOARD"]; ?>">博客论坛</a>
		<br />
		<a href="/pc/index.php?id=<?php echo $pcconfig["ADMIN"]; ?>">帮助主题</a>
		</td>
      </tr>
    </table></td>
    </tr>
  <tr>
    <td colspan="3" height="1" bgcolor="#06337D"> </td>
  </tr>
  <tr>
    <td colspan="3"><table width="100%" border="0" cellspacing="0" cellpadding="3">
      <tr>
        <td width="200" align="center" valign="top"><table width="100%"  border="0" cellpadding="5" cellspacing="0" class="table">
          <tr>
            <td class="channelback"><table width="100%"  border="0" cellspacing="0" cellpadding="0">
                <tr>
                  <td width="73%" class="channel">水木统计</td>
                  <td width="27%" align="right" class="more">&nbsp;</td>
                </tr>
            </table></td>
          </tr>
          <tr>
            <td align="left" valign="top" bgcolor="#ECF5FF" class="td">
			<?php pcmain_blog_statistics_list(); ?>			</td>
          </tr>
        </table>          <br />                    <table width="100%"  border="0" cellpadding="5" cellspacing="0" class="table">
          <tr>
              <td class="channelback"><table width="100%"  border="0" cellspacing="0" cellpadding="0">
                <tr>
                  <td class="channel">点击排行</td>
                  <td align="right" class="more"><a href="pc.php?order=visitcount&order1=DESC" class="more">更多</a></td>
              </tr>
              </table></td>
            </tr>
            <tr>
              <td align="left" valign="top" bgcolor="#F6F6F6" class="td">
			  <?php pcmain_blog_top_ten(); ?>			  </td>
          </tr>
          </table>
          <br />
            <table width="100%"  border="0" cellpadding="5" cellspacing="0" class="table">
            <tr>
                <td class="channelback"><table width="100%"  border="0" cellspacing="0" cellpadding="0">
                  <tr>
                    <td width="70%" class="channel">最近更新</td>
                    <td width="30%" align="right" class="more"><a href="pc.php?order=modifytime&order1=DESC" class="more">更多</a></td>
                  </tr>
                </table></td>
            </tr>
              <tr>
                <td align="left" valign="top" bgcolor="#E8FFEE" class="td">
				<?php pcmain_blog_last_update(); ?>
				</td>
            </tr>
            </table>
          <br />
            <table width="100%"  border="0" cellpadding="5" cellspacing="0" class="table">
            <tr>
                <td class="channelback"><table width="100%"  border="0" cellspacing="0" cellpadding="0">
                  <tr>
                    <td width="74%" class="channel">最新申请</td>
                    <td width="26%" align="right" class="more"><a class="more" href="pc.php?order=createtime&order1=DESC">更多</a></td>
                  </tr>
                </table></td>
            </tr>
              <tr>
                <td align="left" valign="top" bgcolor="#FFFFE6" class="td">
				<?php pcmain_blog_new_user(); ?>				</td>
            </tr>
            </table>
          <br />
            <table width="100%"  border="0" cellpadding="5" cellspacing="0" class="table">
            <tr>
                <td class="channelback"><table width="100%"  border="0" cellspacing="0" cellpadding="0">
                  <tr>
                    <td width="73%" class="channel">新增模板</td>
                    <td width="27%" align="right" class="more">更多</td>
                  </tr>
                </table></td>
            </tr>
              <tr>
                <td align="left" valign="top" bgcolor="#ECF5FF" class="td">&nbsp;</td>
            </tr>
            </table>
          <br />
            <table width="100%"  border="0" cellpadding="5" cellspacing="0" class="table">
            <tr>
                <td class="channelback">RSS频道</td>
              </tr>
              <tr>
                <td align="left" valign="top" bgcolor="#F6F6F6" class="td">
				  推荐日志
				  <a href="/pc/rssrec.php"><img src="/pc/images/xml.gif" border="0" alt="推荐日志" /></a>
				<br />
                  最新日志
				  <a href="/pc/rssnew.php"><img src="/pc/images/xml.gif" border="0" alt="最新日志" /></a>				  </td>
            </tr>
          </table></td><td align="center" valign="top"><table width="100%"  border="0" cellspacing="0" cellpadding="3">
          <tr>
            <td align="center" bgcolor="#F6F6F6"><a href="/bbsdoc.php?board=<?php echo $pcconfig["BOARD"]; ?>">博客论坛</a> | <a href="/pc/pc.php">用户列表</a> | <a href="/pc/pcreclist.php">水木推荐</a> | <a href="/pc/pcnew.php">最新日志</a> | <a href="/pc/pcnew.php?t=c">最新评论</a> | <a href="/pc/pcsec.php">分类列表</a> | <a href="/pc/pcnsearch.php">日志搜索</a> | <a href="/pc/pcmain_o.php">老版本</a></td>
          </tr>
          <tr>
            <td><table width="100%"  border="0" cellpadding="3" cellspacing="0">
              <tr>
                <td class="topic">热门话题</td>
              </tr>
              <tr>
                <td><?php pcmain_blog_most_hot(); ?></td>
              </tr>
            </table></td>
          </tr>
          <tr>
            <td><table width="100%"  border="0" cellspacing="0" cellpadding="3">
              <tr>
                <td class="topic">水木推荐</td>
                </tr>
              <tr>
                <td bgcolor="#ECF5FF"><?php pcmain_blog_recommend_nodes(); ?></td>
              </tr>
            </table></td>
          </tr>
          <tr>
            <td><table width="100%"  border="0" cellpadding="3" cellspacing="0">
              <tr>
                <td class="topic">点击排行</td>
                </tr>
              <tr>
                <td bgcolor="#FCFCFC"><?php pcmain_blog_most_view(); ?></td>
              </tr>
            </table></td>
          </tr>
		  <tr>
            <td><table width="100%"  border="0" cellpadding="3" cellspacing="0">
              <tr>
                <td class="topic">引用排行</td>
                </tr>
              <tr>
                <td bgcolor="#F6F6F6"><?php pcmain_blog_most_trackback(); ?></td>
              </tr>
            </table></td>
          </tr>
		  <form action="pcsearch.php" method="get" onsubmit="if(this.keyword.value==''){alert('请输入关键字');return false;}">
          <tr>
            <td align="center" background="images/bg.gif">
			BLOG搜索
			<input name="keyword" type="text" class="textinput" size="20"> 
			<input type="hidden" name="exact" value="0">
			<input type="radio" name="key" value="u" checked>用户名
			<input type="radio" name="key" value="c">Blog名
			<input type="radio" name="key" value="d">Blog描述
			<input type="submit" class="textinput" value="GO">			</td>
          </tr>
		  </form>
          <tr>
            <td><table width="100%"  border="0" cellspacing="0" cellpadding="3">
              <tr>
                <td class="topic">分类主题</td>
                </tr>
              <tr>
                <td>
				<?php pcmain_blog_sections(); ?>				</td>
                </tr>
            </table></td>
          </tr>
          <tr>
            <td><table width="100%"  border="0" cellpadding="3" cellspacing="0">
              <tr>
                <td class="topic">最新日志</td>
                </tr>
              <tr>
                <td><?php pcmain_blog_new_nodes(); ?></td>
                </tr>
            </table></td>
          </tr>
        </table></td>
      </tr>
    </table></td>
    </tr>
          <tr>
    <td colspan="3" height="1" bgcolor="#06337D"> </td>
  </tr>
  <tr><td height="5"> </td></tr>
         <tr>
            <td align="center" bgcolor="#F6F6F6"><a href="/bbsdoc.php?board=<?php echo $pcconfig["BOARD"]; ?>">博客论坛</a> | <a href="/pc/pc.php">用户列表</a> | <a href="/pc/pcreclist.php">水木推荐</a> | <a href="/pc/pcnew.php">最新日志</a> | <a href="/pc/pcnew.php?t=c">最新评论</a> | <a href="/pc/pcsec.php">分类列表</a> | <a href="/pc/pcnsearch.php">日志搜索</a> | <a href="/pc/pcmain_o.php">老版本</a></td>
          </tr>
          <tr><td height="5"> </td></tr>
  <tr>
    <td colspan="3" align="center">版权所有
	&copy;<?php echo $pcconfig["BBSNAME"]; ?></td>
    </tr>
</table>
</center>
<?php
pc_db_close($link);
html_normal_quit();
?>
