<?php
//require_once("pcfuncs.php");
require_once("pcstat.php");
require_once("pcmainfuncs.php");
if(defined("_PCMAIN_RECOMMEND_QUEUE_"))
	@include(_PCMAIN_RECOMMEND_QUEUE_);

if(pc_update_cache_header())
	return;

function pcmain_blog_statistics_list()
{
	global $link;
?>
用户：<strong><?php echo getUsersCnt($link); ?></strong> 人<br />
日志：<strong><?php echo getNodesCnt($link); ?></strong> 个<br />
评论：<strong><?php echo getCommentsCnt($link); ?></strong> 篇<br />
<?php
}

function pcmain_blog_new_user()
{
	global $pcconfig,$link;
	$newUsers = getNewUsers($link,_PCMAIN_USERS_NUM_);
?>
<ul>
<?php
	foreach($newUsers as $newUser)
	{
		echo "<li><a href=\"index.php?id=".$newUser["username"]."\"><span title=\"".html_format($newUser["description"])."\">".html_format($newUser["corpusname"])."</span></a>&nbsp;<a href=\"/bbsqry.php?userid=".$newUser["username"]."\"><font class=\"low\">".$newUser["username"]."</font></a></li>";	
	}
?>				
</ul>
<?php
}

function pcmain_blog_top_ten()
{
	global $pcconfig,$link;
	//$mostVstUsers = getHotUsersByPeriod($link,'day',_PCMAIN_USERS_NUM_); //每日热门
	if(defined("_BLOG_SCORE_STAT_"))
		$mostVstUsers = getScoreTopUsers($link, _PCMAIN_USERS_NUM_);
	else
		$mostVstUsers = getMostVstUsers($link,_PCMAIN_USERS_NUM_);
?>
<ul>
<?php
	foreach($mostVstUsers as $mostVstUser)
	{
		echo "<li><a href=\"index.php?id=".$mostVstUser["username"]."\"><span title=\"".html_format($mostVstUser["description"])."\">".html_format($mostVstUser["corpusname"])."</span></a>&nbsp;<a href=\"/bbsqry.php?userid=".$mostVstUser["username"]."\"><font class=\"low\">".$mostVstUser["username"]."</font></a></li>";	
	}
?>				
</ul>
<?php
}

function pcmain_blog_last_update()
{
	global $pcconfig,$link;
	$lastUpdates = getLastUpdates($link,_PCMAIN_USERS_NUM_);
?>
<ul>
<?php
	foreach($lastUpdates as $lastUpdate)
	{
		echo "<li><a href=\"index.php?id=".$lastUpdate["username"]."\"><span title=\"".html_format($lastUpdate["description"])."\">".html_format($lastUpdate["corpusname"])."</span></a>&nbsp;<a href=\"/bbsqry.php?userid=".$lastUpdate["username"]."\"><font class=\"low\">".$lastUpdate["username"]."</font></a></li>";	
	}
?>
</ul>
<?php
}

function pcmain_annouce()
{
	global $pcconfig,$link;
	$anns = getPcAnnounce($link, _PCMAIN_ANNS_NUM_ );
?>
<table width="100%" cellspacing="0" cellpadding="3" border="0" class="table">
	<tr><td class="channelback"><font class="channel"><?php echo BBS_NICKNAME; ?>动态</font></td></tr>
	<tr><td align="left" valign="top" class="td"><ul>
<?php
	foreach($anns as $ann)
		echo "<li><a href=\"/pc/pccon.php?id=".$ann[0]."&nid=".$ann["nid"]."&s=all\">".html_format($ann["subject"])."</a></li>";
?>	
	<li><a href="/pc/index.php?id=<?php echo $pcconfig["ADMIN"]; ?>">&gt;&gt; 更多</a></li>
	</ul></td></tr>
</table>
<?php	
}

function pcmain_recommend_blogger()
{
	global $link;
	mt_srand();
	$pos = mt_rand(0,50);//排名前50的BLOG随机抽取一个
	if(defined("_PCMAIN_RECOMMEND_BLOGGER_"))
		$query = "SELECT uid , username , corpusname , description FROM users WHERE username = '".addslashes(_PCMAIN_RECOMMEND_BLOGGER_)."' LIMIT 1;";
	else
		$query = "SELECT uid , username , corpusname , description FROM users ORDER BY visitcount DESC LIMIT ".$pos.",1;";
	$result = mysql_query($query,$link);
	$pc = mysql_fetch_array($result);
	mysql_free_result($result);
	if (!$pc) return;
	//提取该用户最热门的5篇文章
	$query = "SELECT nid,subject FROM nodes WHERE access = 0 AND uid = '".$pc["uid"]."' ORDER BY visitcount DESC LIMIT 0 , 5;";
	$result = mysql_query($query,$link);
	$nodes = array();
	while($rows = mysql_fetch_array($result))
		$nodes[] = $rows;
	mysql_free_result($result);	
?>
<table width="100%" cellspacing="0" cellpadding="3" border="0" class="table">
	<tr><td class="channelback" align="right"><font class="channel">BLOG推荐</font></td></tr>
	<tr><td align="left" valign="top" class="td" bgcolor="#E8FFEE">
	<table width="100%" cellspacing="0" cellpadding="3" border="0">
	<tr>
	<td valign="top" align="left">
	[<b><a href="index.php?id=<?php echo $pc["username"]; ?>"><?php echo html_format($pc["corpusname"]); ?></a></b>]
	<a href="/bbsqry.php?userid=<?php echo $pc["username"]; ?>"><font class="low2"><?php echo $pc["username"]; ?></font></a></td></tr>
	<tr><td>
	<?php echo html_format($pc["description"]); ?>
	</td>
	</tr>
	</table>
	</td></tr>
	<tr>
	<td align="left" valign="top" class="td"><ul>
<?php
	foreach($nodes as $node)
		echo "<li><a href=\"pccon.php?id=".$pc["uid"]."&nid=".$node["nid"]."&s=all\">".html_format($node["subject"])."</a></li>";
?>
	</ul></td>
	</tr>
</table>
<?php
	
}

function  pcmain_blog_most_hot()
{
	global $pcconfig,$link;
	$nodes = getHotNodes($link, "comments" , _PCMAIN_TIME_LONG_ ,_PCMAIN_NODES_NUM_ );
	$num = count($nodes);
?>
<table cellspacing=0 cellpadding=3 width=100%>
<?php
	for($i = 0;$i < $num ;$i ++)
	{
		$rows = $nodes[$i];
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
		$pcinfor = pc_load_infor($link,"",$rows["uid"]);
		echo "|&nbsp;<a href=\"pccon.php?id=".$rows["uid"]."&nid=".$rows["nid"]."&s=all\">";
		$subject = "<span title=\"".html_format($rows["subject"])."\">".html_format(substr($rows["subject"],0,20));
		if(strlen($rows["subject"]) > 20 )
			$subject .= "...";
		$subject .= "</span>";
		echo $subject."</a>\n&nbsp;<a href=\"index.php?id=".$pcinfor["USER"]."\">";
		//echo "<font class=low>".$pcinfor[NAME]."</font></a>&nbsp;<a href=\"/bbsqry.php?userid=".$pcinfor["USER"]."\">";
		echo "<font class=low2>".$pcinfor["USER"]."</font></a>\n";
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
	$nodes = getHotNodes($link, "trackbacks" , _PCMAIN_TIME_LONG_ * 2 ,_PCMAIN_NODES_NUM_ );
	$num = count($nodes);
?>
<table cellspacing=0 cellpadding=3 width=98%>
<tr><td style="line-height:16px " align="left" width="50%"><ul>
<?php
	for($i = 0;$i < $num ;$i ++)
	{
		if( $i == intval($num / 2) ) echo "</td><td align=\"left\" style=\"line-height:16px\" width=\"50%\">";
		$rows = $nodes[$i];
		$pcinfor = pc_load_infor($link,"",$rows["uid"]);
		echo "<li><a href=\"pccon.php?id=".$rows["uid"]."&nid=".$rows["nid"]."&s=all\">";
		$subject = "<span title=\"".html_format($rows["subject"])."\">".html_format(substr($rows["subject"],0,20));
		if(strlen($rows["subject"]) > 20 )
			$subject .= "...";
		$subject .= "</span>";
		echo $subject."</a>\n&nbsp;<a href=\"index.php?id=".$pcinfor["USER"]."\">";
		//echo "<font class=low>".$pcinfor[NAME]."</font></a>&nbsp;<a href=\"/bbsqry.php?userid=".$pcinfor[USER]."\">";
		echo "<font class=low2>".$pcinfor["USER"]."</font></a>";
		echo "</li>\n";	
	}
?>				
</ul></td>
</tr>
</table>
<?php		
}

function  pcmain_blog_most_view()
{
	global $pcconfig,$link;
	$nodes = getHotNodes($link, "visits" , _PCMAIN_TIME_LONG_ ,_PCMAIN_NODES_NUM_ );
	$num = count($nodes);
?>
<table cellspacing=0 cellpadding=3 width=98%>
<tr><td style="line-height:16px " align="left" width="50%"><ul>
<?php
	for($i = 0;$i < $num ;$i ++)
	{
		if( $i == intval( $num / 2 ) ) echo "</td><td style=\"line-height:16px \" align=left width=\"50%\">";
		$rows = $nodes[$i];
		$pcinfor = pc_load_infor($link,"",$rows["uid"]);
		echo "<li><a href=\"pccon.php?id=".$rows["uid"]."&nid=".$rows["nid"]."&s=all\">";
		$subject = "<span title=\"".html_format($rows["subject"])."\">".html_format(substr($rows["subject"],0,20));
		if(strlen($rows["subject"]) > 20 )
			$subject .= "...";
		$subject .= "</span>";
		echo $subject."</a>\n&nbsp;<a href=\"index.php?id=".$pcinfor["USER"]."\">";
		//echo "<font class=low>".$pcinfor[NAME]."</font></a>&nbsp;<a href=\"/bbsqry.php?userid=".$pcinfor[USER]."\">";
		echo "<font class=low2>".$pcinfor["USER"]."</font></a>\n";
		echo "</li>\n";	
	}
?>
</ul></td></tr>
</table>
<?php		
}

function pcmain_blog_new_nodes()
{
	global $link;
	$newBlogs = getNewBlogs($link,1, _PCMAIN_NEW_NODES_ );
	$newNum = count($newBlogs["useretems"]);
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
		echo "<td class=".$tdclass." width=\"33%\">[<span title=\"".$newBlogs["useretems"][$i]["pcdesc"]."\"><a href=\"index.php?id=".$newBlogs["useretems"][$i]["creator"]."\"><font class=low2>".$newBlogs["useretems"][$i]["pcname"]."</font></a></span>]".
			"&nbsp;<a href='/bbsqry.php?userid=".$newBlogs["useretems"][$i]["creator"]."'><font class=low>".$newBlogs["useretems"][$i]["creator"]."</font></a><br />".
			"<a href='pccon.php?id=".$newBlogs["useretems"][$i]["pc"]."&nid=".$newBlogs["useretems"][$i]["nid"]."&s=all&tid=".$newBlogs["useretems"][$i]["tid"]."'>".
			"<span title=\"".$newBlogs["useretems"][$i]["subject"]."\">";
		echo substr($newBlogs["useretems"][$i]["subject"],0,36);
		if(strlen($newBlogs["useretems"][$i]["subject"])>36) echo "...";
		echo "</span></a></td>";
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
	$nodes = getRecommendNodes($link, _PCMAIN_REC_NODES_ );
	$num = count($nodes);
?>
<table cellspacing=0 cellpadding=3 width=98%>
<tr><td style="line-height:16px " align="left" width="50%"><ul>
<?php
	for($i = 0;$i < $num ;$i ++)
	{
		if( $i == intval( $num / 2) ) echo "</td><td align=\"left\" style=\"line-height:16px\" width=\"50%\">";
		$rows = $nodes[$i];
		$pcinfor = pc_load_infor($link,"",$rows["uid"]);
		echo "<li><a href=\"pccon.php?id=".$rows["uid"]."&nid=".$rows["nid"]."&s=all\">";
		$subject = "<span title=\"".html_format($rows["subject"])."\">".html_format(substr($rows["subject"],0,20));
		if(strlen($rows["subject"]) > 20 )
			$subject .= "...";
		$subject .= "</span>";
		echo $subject."</a>\n&nbsp;<a href=\"index.php?id=".$pcinfor["USER"]."\">";
		//echo "<font class=low>".$pcinfor[NAME]."</font></a>&nbsp;<a href=\"/bbsqry.php?userid=".$pcinfor["USER"]."\">";
		echo "<font class=low2>".$pcinfor["USER"]."</font></a>";
		echo "</li>\n";	
	}
?>
</ul></td>
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

function pcmain_section_top_view()
{
	global $pcconfig,$link;
?>
<table cellspacing=0 cellpadding=3 width="100%">
<?php
	$sections = array_keys($pcconfig["SECTION"]);
	$othersections = array();
	foreach( $sections as $section )
	{
		$nodes = getSectionHotNodes($link,$section,_PCMAIN_TIME_LONG_,_PCMAIN_SECTION_NODES_);
		if($nodes)
		{
		    $nodesNum = sizeof($nodes);
?>
<tr><td align="left"><table cellspacing="0" cellpadding="2" border="0" class="f1"><tr>
<td align="left" valign="top" width="75">
[<strong><a href="/pc/pcsec.php?sec=<?php echo $section; ?>"><font class=low2><?php echo $pcconfig["SECTION"][$section]; ?></font></a></strong>]
</td><td>
<?php
			for( $i = 0 ; $i < $nodesNum ; $i ++ )
			{
				echo "<a href=\"/pc/pccon.php?id=".$nodes[$i][0]."&nid=".$nodes[$i]["nid"]."&s=all\">".
				     "<span title=\"".html_format($nodes[$i]["subject"])."(".$nodes[$i]["username"]."'s BLOG:".html_format($nodes[$i]["corpusname"]).")\">";
				echo html_format($nodes[$i]["subject"])."</span></a>";
				if( $i < $nodesNum - 1 ) echo " | ";
			}
?>
</td></tr></table>
</td></tr>
<?php			
		}
		else
			$othersections[] = $section;
	}
?>
<tr><td>
<?php
		foreach( $othersections as $section )
			echo "[<a href=\"/pc/pcsec.php?sec=".$section."\"><font class=low2>".$pcconfig["SECTION"][$section]."</font></a>]\n";
?>
</td></tr>
</table>
<?php
}

function pcmain_blogger_area()
{
	global $pcconfig;
	$all = $pcconfig["ALLCHARS"];
	$all_num = strlen($all);
	echo "<font class=low>[BLOG小区]</font>&nbsp;";
	for($i = 0 ; $i < $all_num ; $i ++)
		echo "<a href=\"pc.php?char=".$all[$i]."\"><font class=\"low2\">".$all[$i]."</font></a>&nbsp;";
	echo "<a href=\"pclist.php\"><font color=red>热门BLOG</font></a>";
}

$link = pc_db_connect();
pcmain_html_init();
if (defined("SITE_SMTH")) {
?>
<script type="text/javascript"><!--
reg = new RegExp("^https?://[^/]*smth\\.[^/]+/");
if (top.location != self.location && !top.location.href.match(reg)) {
	top.location = self.location;
}
// --></script>
<?php
}
?>
  <tr>
    <td><table width="100%" border="0" cellspacing="0" cellpadding="3">
      <tr>
        <td width="200" align="center" valign="top"><table width="100%"  border="0" cellpadding="5" cellspacing="0" class="table">
          <tr>
            <td class="channelback"><table width="100%"  border="0" cellspacing="0" cellpadding="0">
                <tr>
                  <td width="73%" class="channel"><?php echo BBS_NICKNAME; ?>统计</td>
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
                  <td class="channel">热门BLOG</td>
                  <td align="right" class="more"><a href="pc.php?order=<?php echo defined("_BLOG_SCORE_STAT_")?"score":"visitcount"; ?>&order1=DESC"><font class="more">更多</font></a></td>
              </tr>
              </table></td>
            </tr>
            <tr>
              <td align="left" valign="top" bgcolor="#F6F6F6" class="td">
		<?php pcmain_blog_top_ten(); ?>
	      <center>
	      [<a href="pc.php?order=<?php echo defined("_BLOG_SCORE_STAT_")?"score":"visitcount"; ?>&order1=DESC" class="low2">更多</a>]
	      [<a href="opml.php?t=<?php echo defined("_BLOG_SCORE_STAT_")?"3":"1"; ?>" class="low2">OPML</a>]
	      </center>
	      </td>
          </tr>
          </table>
          <br />
            <table width="100%"  border="0" cellpadding="5" cellspacing="0" class="table">
            <tr>
                <td class="channelback"><table width="100%"  border="0" cellspacing="0" cellpadding="0">
                  <tr>
                    <td width="70%" class="channel">最近更新</td>
                    <td width="30%" align="right" class="more"><a href="pc.php?order=modifytime&order1=DESC"><font class="more">更多</font></a></td>
                  </tr>
                </table></td>
            </tr>
              <tr>
                <td align="left" valign="top" bgcolor="#E8FFEE" class="td">
				<?php pcmain_blog_last_update(); ?>
		<center>
	        [<a href="pc.php?order=modifytime&order1=DESC" class="low2">更多</a>]
	        [<a href="opml.php" class="low2">OPML</a>]
	        </center>
		</td>
            </tr>
            </table>
          <br />
            <table width="100%"  border="0" cellpadding="5" cellspacing="0" class="table">
            <tr>
                <td class="channelback"><table width="100%"  border="0" cellspacing="0" cellpadding="0">
                  <tr>
                    <td width="74%" class="channel">最新申请</td>
                    <td width="26%" align="right" class="more"><a href="pc.php?order=createtime&order1=DESC"><font class="more">更多</font></a></td>
                  </tr>
                </table></td>
            </tr>
              <tr>
                <td align="left" valign="top" bgcolor="#FFFFE6" class="td">
				<?php pcmain_blog_new_user(); ?>
		<center>
	        [<a href="pc.php?order=createtime&order1=DESC" class="low2">更多</a>]
	        [<a href="opml.php?t=2" class="low2">OPML</a>]
	        </center>
		</td>
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
				  <a href="/pc/rssnew.php"><img src="/pc/images/xml.gif" border="0" alt="最新日志" /></a>
				<br /><br /><br />
		  <b>OPML频道群组</b><br /><br />
		  <a href="opml.php">最近更新用户组</a><br />
		  <a href="opml.php?t=1">访问最多用户组</a><br />
		  <a href="opml.php?t=2">最新申请用户组</a>  
				  
				  				  </td>
            </tr>
          </table></td><td align="center" valign="top"><table width="100%"  border="0" cellspacing="0" cellpadding="3">
          <tr>
            <td>
               <table cellspacing=0 cellpadding=1 width=100% border=0>
                 <tr>
<?php
		if (defined("_PCMAIN_RECOMMEND_")) 
		{
?>
                   <td align="center" valign="top" width="50%">
                   <?php
			pcmain_recommend_blogger();  
                   ?>
                   </td>
<?php
		}
?>	
                 <td align="center" valign="top">
                   <?php pcmain_annouce(); ?>
                   </td>
                 </tr>
               </table>
            </td>
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
                <td class="topic"><?php echo BBS_NICKNAME; ?>推荐</td>
                </tr>
              <tr>
                <td bgcolor="#ECF5FF"><?php pcmain_blog_recommend_nodes(); ?></td>
              </tr>
            </table></td>
          </tr>
          <tr>
          <td align="center" background="images/bg.gif">
          <?php pcmain_blogger_area(); ?>
          </td>
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
	  <tr>
            <td><table width="100%"  border="0" cellspacing="0" cellpadding="3">
              <tr>
                <td class="topic">分类主题</td>
                </tr>
              <tr>
                <td bgcolor="#E8FFEE">
				<?php pcmain_section_top_view(); ?>				</td>
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
<?php
pc_db_close($link);
pcmain_html_quit()
?>
