<?php
require("pcfuncs.php");
require("pcstat.php");
require("pcmainfuncs.php");

function pcmain_blog_statistics_list()
{
	global $link;
?>
�û���<strong><?php echo getUsersCnt($link); ?></strong> ��<br />
��־��<strong><?php echo getNodesCnt($link); ?></strong> ��<br />
���ۣ�<strong><?php echo getCommentsCnt($link); ?></strong> ƪ<br />
<?php
}

function pcmain_blog_new_user()
{
	global $pcconfig,$link;
	$newUsers = getNewUsers($link,_PCMAIN_USERS_NUM_);
?>

<?php
	foreach($newUsers as $newUser)
	{
		echo "<li><a href=\"index.php?id=".$newUser[username]."\"><span title=\"".html_format($newUser[description])."\">".html_format($newUser[corpusname])."</span></a>&nbsp;<a href=\"/bbsqry.php?userid=".$newUser[username]."\"><font class=\"low\">".$newUser[username]."</font></a></li>";	
	}
?>				

<?php
}

function pcmain_blog_top_ten()
{
	global $pcconfig,$link;
	$mostVstUsers = getMostVstUsers($link,_PCMAIN_USERS_NUM_);
?>
		
<?php
	foreach($mostVstUsers as $mostVstUser)
	{
		echo "<li><a href=\"index.php?id=".$mostVstUser[username]."\"><span title=\"".html_format($mostVstUser[description])."\">".html_format($mostVstUser[corpusname])."</span></a>&nbsp;<a href=\"/bbsqry.php?userid=".$mostVstUser[username]."\"><font class=\"low\">".$mostVstUser[username]."</font></a></li>";	
	}
?>				
		
<?php
}

function pcmain_blog_last_update()
{
	global $pcconfig,$link;
	$lastUpdates = getLastUpdates($link,_PCMAIN_USERS_NUM_);
?>
				
<?php
	foreach($lastUpdates as $lastUpdate)
	{
		echo "<li><a href=\"index.php?id=".$lastUpdate[username]."\"><span title=\"".html_format($lastUpdate[description])."\">".html_format($lastUpdate[corpusname])."</span></a>&nbsp;<a href=\"/bbsqry.php?userid=".$lastUpdate[username]."\"><font class=\"low\">".$lastUpdate[username]."</font></a></li>";	
	}
?>				
				
<?php
}

function pcmain_annouce()
{
	global $pcconfig,$link;
	$anns = getPcAnnounce($link, _PCMAIN_ANNS_NUM_ );
?>
<table width="100%" cellspacing="0" cellpadding="3" border="0" class="table">
	<tr><td class="channelback"><font class="channel">ˮľ��̬</font></td></tr>
	<tr><td align="left" valign="top" class="td">
<?php
	foreach($anns as $ann)
		echo "<li><a href=\"/pc/pccon.php?id=".$ann[0]."&nid=".$ann[nid]."&s=all\">".html_format($ann[subject])."</a></li>";
?>	
	<li><a href="/pc/index.php?id=<?php echo $pcconfig["ADMIN"]; ?>">&gt;&gt; ����</a></li>
	</td></tr>
</table>
<?php	
}

/*
function pcmain_recommend_blogger()
{
	global $link;
	mt_srand();
	$pos = mt_rand(0,50);//����ǰ50�Ĳ��������ȡһ��
	$query = "SELECT username , corpusname , description FROM users ORDER BY visitcount DESC LIMIT ".$pos.",1;";
	$result = mysql_query($query,$link);
	$pc = mysql_fetch_array($result);
?>
<table width="100%" cellspacing="0" cellpadding="3" border="0" class="table">
	<tr><td class="channelback" align="right"><font class="channel">�����Ƽ�</font></td></tr>
	<tr><td align="left" valign="top" bgcolor="#ECF5FF" class="td">
	<table width="100%" cellspacing="0" cellpadding="3" border="0" class="table">
	
	</table>
	</td></tr>
</table>
<?php
	mysql_free_result($result);	
}
*/

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
		$pcinfor = pc_load_infor($link,"",$rows[uid]);
		echo "|&nbsp;<a href=\"pccon.php?id=".$rows[uid]."&nid=".$rows[nid]."&s=all\">";
		$subject = "<span title=\"".html_format($rows[subject])."\">".html_format(substr($rows[subject],0,20));
		if(strlen($rows[subject]) > 20 )
			$subject .= "...";
		$subject .= "</span>";
		echo $subject."</a>\n&nbsp;<a href=\"index.php?id=".$pcinfor[USER]."\">";
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
<tr><td style="line-height:16px " align="left" width="50%">
<?php
	for($i = 0;$i < $num ;$i ++)
	{
		if( $i == intval($num / 2) ) echo "</td><td align=\"left\" style=\"line-height:16px\" width=\"50%\">";
		$rows = $nodes[$i];
		$pcinfor = pc_load_infor($link,"",$rows[uid]);
		echo "<li><a href=\"pccon.php?id=".$rows[uid]."&nid=".$rows[nid]."&s=all\">";
		$subject = "<span title=\"".html_format($rows[subject])."\">".html_format(substr($rows[subject],0,20));
		if(strlen($rows[subject]) > 20 )
			$subject .= "...";
		$subject .= "</span>";
		echo $subject."</a>\n&nbsp;<a href=\"index.php?id=".$pcinfor[USER]."\">";
		//echo "<font class=low>".$pcinfor[NAME]."</font></a>&nbsp;<a href=\"/bbsqry.php?userid=".$pcinfor[USER]."\">";
		echo "<font class=low2>".$pcinfor[USER]."</font></a>";
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
	$nodes = getHotNodes($link, "visits" , _PCMAIN_TIME_LONG_ ,_PCMAIN_NODES_NUM_ );
	$num = count($nodes);
?>
<table cellspacing=0 cellpadding=3 width=98%>
<tr><td style="line-height:16px " align="left" width="50%">
<?php
	for($i = 0;$i < $num ;$i ++)
	{
		if( $i == intval( $num / 2 ) ) echo "</tr><td style=\"line-height:16px \" align=left width=\"50%\">";
		$rows = $nodes[$i];
		$pcinfor = pc_load_infor($link,"",$rows[uid]);
		echo "<li><a href=\"pccon.php?id=".$rows[uid]."&nid=".$rows[nid]."&s=all\">";
		$subject = "<span title=\"".html_format($rows[subject])."\">".html_format(substr($rows[subject],0,20));
		if(strlen($rows[subject]) > 20 )
			$subject .= "...";
		$subject .= "</span>";
		echo $subject."</a>\n&nbsp;<a href=\"index.php?id=".$pcinfor[USER]."\">";
		//echo "<font class=low>".$pcinfor[NAME]."</font></a>&nbsp;<a href=\"/bbsqry.php?userid=".$pcinfor[USER]."\">";
		echo "<font class=low2>".$pcinfor[USER]."</font></a>\n";
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
	$newBlogs = getNewBlogs($link,1, _PCMAIN_NEW_NODES_ );
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
		echo "<td class=".$tdclass." width=\"33%\">[<span title=\"".$newBlogs[useretems][$i][pc][DESC]."\"><a href=\"index.php?id=".$newBlogs[useretems][$i][pc][USER]."\"><font class=low2>".$newBlogs[useretems][$i][pc][NAME]."</font></a></span>]".
			"&nbsp;<a href='/bbsqry.php?userid=".$newBlogs[useretems][$i][pc][USER]."'><font class=low>".$newBlogs[useretems][$i][pc][USER]."</font></a><br />".
			"<a href='pccon.php?id=".$newBlogs[useretems][$i][pc][UID]."&tid=".$newBlogs[useretems][$i][tid]."&nid=".$newBlogs[useretems][$i][nid]."&s=all'>".
			"<span title=\"".$newBlogs[useretems][$i][subject]."\">";
		echo substr($newBlogs[useretems][$i][subject],0,36);
		if(strlen($newBlogs[useretems][$i][subject])>36) echo "...";
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
<tr><td style="line-height:16px " align="left" width="50%">
<?php
	for($i = 0;$i < $num ;$i ++)
	{
		if( $i == intval( $num / 2) ) echo "</td><td align=\"left\" style=\"line-height:16px\" width=\"50%\">";
		$rows = $nodes[$i];
		$pcinfor = pc_load_infor($link,"",$rows[uid]);
		echo "<li><a href=\"pccon.php?id=".$rows[uid]."&nid=".$rows[nid]."&s=all\">";
		$subject = "<span title=\"".html_format($rows[subject])."\">".html_format(substr($rows[subject],0,20));
		if(strlen($rows[subject]) > 20 )
			$subject .= "...";
		$subject .= "</span>";
		echo $subject."</a>\n&nbsp;<a href=\"index.php?id=".$pcinfor[USER]."\">";
		//echo "<font class=low>".$pcinfor[NAME]."</font></a>&nbsp;<a href=\"/bbsqry.php?userid=".$pcinfor["USER"]."\">";
		echo "<font class=low2>".$pcinfor["USER"]."</font></a>";
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
		$query = "SELECT nodes.uid , nid , subject , theme , username , corpusname  ".
			 "FROM nodes , users ".
			 "WHERE nodes.uid = users.uid AND access = 0 AND type = 0 AND recommend != 2 AND created > ".date("YmdHis",time()- _PCMAIN_TIME_LONG_ )." AND theme = '".$section."' ".
			 "GROUP BY nodes.uid ".
			 "ORDER BY nodes.visitcount DESC , nid DESC ".
			 "LIMIT 0 , 12 ;";
		$result = mysql_query($query,$link);
		$num_rows = mysql_num_rows($result);
		if($num_rows)
		{
			$nodes = array();
			$totallength = 0;
			while($rows = mysql_fetch_array($result) )
			{
				if( $totallength + strlen( $rows[subject] ) > 65 )
					continue;
				$nodes[] = $rows;
				$totallength += strlen( $rows[subject] );
			}
			mysql_free_result($result);
			//$subjectlength = ( $totallength > 65 )?13:65;
			$nodesNum = max(1,count($nodes) - 1);
?>
<tr><td align="left">
[<strong><a href="/pc/pcsec.php?sec=<?php echo $section; ?>"><font class=low2><?php echo $pcconfig["SECTION"][$section]; ?></font></a></strong>]&nbsp;
<?php
			for( $i = 0 ; $i < $nodesNum ; $i ++ )
			{
				echo "<a href=\"/pc/pccon.php?id=".$nodes[$i][0]."&nid=".$nodes[$i][nid]."&s=all\">".
				     "<span title=\"".html_format($nodes[$i][subject])."(".$nodes[$i][username]."'s BLOG:".html_format($nodes[$i][corpusname]).")\">";
				//$subject = substr( $nodes[$i][subject] , 0 , $subjectlength );
				//if( strlen( $nodes[$i][subject] ) > $subjectlength ) $subject .= "...";
				//echo html_format($subject)."</span></a>";
				echo html_format($nodes[$i][subject])."</span></a>";
				if( $i < $nodesNum - 1 ) echo " | ";
			}
?>
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
			echo "[<a href=\"/pc/pcsec.php?sec=".$section."\"><font class=low2>".$pcconfig["SECTION"][$section]."</class></a>]\n";
?>
</td></tr>
</table>
<?php
}

if(pc_update_cache_header())
	return;

$link = pc_db_connect();
pcmain_html_init();
?>
  <tr>
    <td><table width="100%" border="0" cellspacing="0" cellpadding="3">
      <tr>
        <td width="200" align="center" valign="top"><table width="100%"  border="0" cellpadding="5" cellspacing="0" class="table">
          <tr>
            <td class="channelback"><table width="100%"  border="0" cellspacing="0" cellpadding="0">
                <tr>
                  <td width="73%" class="channel">ˮľͳ��</td>
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
                  <td class="channel">�������</td>
                  <td align="right" class="more"><a href="pc.php?order=visitcount&order1=DESC"><font class="more">����</font></a></td>
              </tr>
              </table></td>
            </tr>
            <tr>
              <td align="left" valign="top" bgcolor="#F6F6F6" class="td">
		<?php pcmain_blog_top_ten(); ?>
	      <center>
	      [<a href="pc.php?order=visitcount&order1=DESC" class="low2">����</a>]
	      [<a href="opml.php?t=1" class="low2">OPML</a>]
	      </center>
	      </td>
          </tr>
          </table>
          <br />
            <table width="100%"  border="0" cellpadding="5" cellspacing="0" class="table">
            <tr>
                <td class="channelback"><table width="100%"  border="0" cellspacing="0" cellpadding="0">
                  <tr>
                    <td width="70%" class="channel">�������</td>
                    <td width="30%" align="right" class="more"><a href="pc.php?order=modifytime&order1=DESC"><font class="more">����</font></a></td>
                  </tr>
                </table></td>
            </tr>
              <tr>
                <td align="left" valign="top" bgcolor="#E8FFEE" class="td">
				<?php pcmain_blog_last_update(); ?>
		<center>
	        [<a href="pc.php?order=modifytime&order1=DESC" class="low2">����</a>]
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
                    <td width="74%" class="channel">��������</td>
                    <td width="26%" align="right" class="more"><a href="pc.php?order=createtime&order1=DESC"><font class="more">����</font></a></td>
                  </tr>
                </table></td>
            </tr>
              <tr>
                <td align="left" valign="top" bgcolor="#FFFFE6" class="td">
				<?php pcmain_blog_new_user(); ?>
		<center>
	        [<a href="pc.php?order=createtime&order1=DESC" class="low2">����</a>]
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
                    <td width="73%" class="channel">����ģ��</td>
                    <td width="27%" align="right" class="more">����</td>
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
                <td class="channelback">RSSƵ��</td>
              </tr>
              <tr>
                <td align="left" valign="top" bgcolor="#F6F6F6" class="td">
				  �Ƽ���־
				  <a href="/pc/rssrec.php"><img src="/pc/images/xml.gif" border="0" alt="�Ƽ���־" /></a>
				<br />
                  ������־
				  <a href="/pc/rssnew.php"><img src="/pc/images/xml.gif" border="0" alt="������־" /></a>
				<br /><br /><br />
		  <b>OPMLƵ��Ⱥ��</b><br /><br />
		  <a href="opml.php">��������û���</a><br />
		  <a href="opml.php?t=1">��������û���</a><br />
		  <a href="opml.php?t=2">���������û���</a>  
				  
				  				  </td>
            </tr>
          </table></td><td align="center" valign="top"><table width="100%"  border="0" cellspacing="0" cellpadding="3">
          <tr>
            <td>
               <table cellspacing=0 cellpadding=0 width=100% border=0>
                 <tr>
                   <td width="100%">
                   <?php pcmain_annouce(); ?>
                   </td>
                   <td>
                   <?php /* pcmain_recommend_blogger(); */ ?>
                   </td>
                 </tr>
               </table>
            </td>
          </tr>
          <tr>
            <td><table width="100%"  border="0" cellspacing="0" cellpadding="3">
              <tr>
                <td class="topic">ˮľ�Ƽ�</td>
                </tr>
              <tr>
                <td bgcolor="#ECF5FF"><?php pcmain_blog_recommend_nodes(); ?></td>
              </tr>
            </table></td>
          </tr>
          <tr>
            <td><table width="100%"  border="0" cellpadding="3" cellspacing="0">
              <tr>
                <td class="topic">���Ż���</td>
              </tr>
              <tr>
                <td><?php pcmain_blog_most_hot(); ?></td>
              </tr>
            </table></td>
          </tr>
          <tr>
            <td><table width="100%"  border="0" cellpadding="3" cellspacing="0">
              <tr>
                <td class="topic">�������</td>
                </tr>
              <tr>
                <td bgcolor="#FCFCFC"><?php pcmain_blog_most_view(); ?></td>
              </tr>
            </table></td>
          </tr>
		  <tr>
            <td><table width="100%"  border="0" cellpadding="3" cellspacing="0">
              <tr>
                <td class="topic">��������</td>
                </tr>
              <tr>
                <td bgcolor="#F6F6F6"><?php pcmain_blog_most_trackback(); ?></td>
              </tr>
            </table></td>
          </tr>
		  <form action="pcsearch.php" method="get" onsubmit="if(this.keyword.value==''){alert('������ؼ���');return false;}">
          <tr>
            <td align="center" background="images/bg.gif">
			BLOG����
			<input name="keyword" type="text" class="textinput" size="20"> 
			<input type="hidden" name="exact" value="0">
			<input type="radio" name="key" value="u" checked>�û���
			<input type="radio" name="key" value="c">Blog��
			<input type="radio" name="key" value="d">Blog����
			<input type="submit" class="textinput" value="GO">			</td>
          </tr>
		  </form>
          <tr>
            <td><table width="100%"  border="0" cellspacing="0" cellpadding="3">
              <tr>
                <td class="topic">��������</td>
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
                <td class="topic">������־</td>
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