<?php
/*
** @id:windinsin nov 29,2003
*/
require_once("pcfuncs.php");

function getNewBlogs($link,$pno=1,$etemnum=0)
{
	global $pcconfig;
	if($pno < 1)
		$pno = 1;
	
	$etemnum = intval( $etemnum );
	if($etemnum <= 0 )
		$etemnum = $pcconfig["NEWS"];
	
	$newBlogs = array();
	$newBlogs["channel"] = array(
			"siteaddr" => "http://".$pcconfig["SITE"],
			"title" => $pcconfig["BBSNAME"]."即时Blog日志" ,
			"pcaddr" => "http://".$pcconfig["SITE"],
			"desc" => $pcconfig["BBSNAME"]."最新".$etemnum."个日志",
			"email" => $pcconfig["BBSNAME"],
			"publisher" => $pcconfig["BBSNAME"],
			"creator" => $pcconfig["BBSNAME"],
			"rights" => $pcconfig["BBSNAME"],
			"date" => date("Y-m-d"),
			"updatePeriod" => "10分钟更新一次",
			"updateFrequency" => "最新的".$etemnum."个Blog日志",
			"updateBase" => date("Y-m-d H:i:s"),
			
			);
	
	$query = "SELECT users.visitcount,description,corpusname,username,created,subject,body,htmltag,nodes.uid,nid,tid,users.theme,nodescount ".
	         "FROM nodes force index(PRIMARY) ,users ".
	         "WHERE `access` = 0 ".
	         "   AND nodes.uid = users.uid ".
	         "   AND pctype < 6 ".
	         "   AND nodetype = 0 ".
	         "ORDER BY `nid` DESC ".
	         "LIMIT ".(($pno - 1) * $etemnum)." , ".$etemnum." ;";
	$result = mysql_query($query,$link);
	$j = 0;
	while($rows=mysql_fetch_array($result))
	{
		$body = "<br>\n".
			"来自: ".html_format($rows["corpusname"])."<br>\n".
			"作者: ".html_format($rows["username"])."<br>\n".
			"发信站: ".$pcconfig["BBSNAME"]."<br>\n".
			"时间: ".time_format($rows["created"])."<br>\n".
			"<hr size=1>\n".
			html_format($rows["body"],TRUE,$rows["htmltag"]).
			"<hr size=1>\n".
			"(<a href=\"http://".$pcconfig["SITE"]."/pc/pccon.php?id=".$rows["uid"]."&nid=".$rows["nid"]."&tid=".$rows["tid"]."&s=all\">浏览全文</a>\n".
			"<a href=\"http://".$pcconfig["SITE"]."/pc/pccom.php?act=pst&nid=".$rows["nid"]."\">发表评论</a>)<br>\n".
			"<a href=\"http://".$pcconfig["SITE"]."/pc/rss.php?userid=".html_format($rows["username"])."\"><img src=\"http://".$pcconfig["SITE"]."/pc/images/xml.gif\" border=\"0\" align=\"absmiddle\" alt=\"XML\">Blog地址：http://".$pcconfig["SITE"]."/pc/rss.php?userid=".html_format($rows["username"])."</a>";
		$newBlogs["useretems"][$j] = array(
					"addr" => "http://".$pcconfig["SITE"]."/pc/pccon.php?id=".$rows["uid"]."&amp;nid=".$rows["nid"]."&amp;tid=".$rows["tid"],
					"subject" => htmlspecialchars($rows["subject"]),
					"desc" => $body,
					"tid" => $rows["tid"],
					"nid" => $rows["nid"],
					"publisher" => $pcconfig["BBSNAME"],
					"creator" => html_format($rows["username"]),
					"pc" => $rows["uid"],
					"created" => time_format($rows["created"]),
					"rights" => html_format($rows["username"]).".bbs@".$pcconfig["SITE"],
					"theme" => pc_get_theme($rows["theme"]),
					"pcdesc" => $rows["description"],
					"pcname" => $rows["corpusname"],
					"pcvisit" => $rows[0],
					"pcnodes" => $rows["nodescount"]
					);
		$j ++;
	}
	mysql_free_result($result);
	
	return $newBlogs;
}

function getNewComments($link,$pno=1,$etemnum=0)
{
	global $pcconfig;
	if($pno < 1)	$pno = 1;
	$etemnum = intval( $etemnum );
	if($etemnum <= 0 )
		$etemnum = $pcconfig["NEWS"];
	
	$newComments = array();
	$query = "SELECT cid , comments.uid , comments.subject , comments.created , comments.username , nodes.subject , nodes.created , nodes.visitcount , commentcount , nodes.nid ".
	         "FROM comments, nodes , users ".
	         "WHERE comments.nid = nodes.nid ".
	         "  AND nodes.uid = users.uid ".
	         "  AND pctype < 6 ".
	         "  AND access = 0 ".
	         "  AND comment = 1 ".
	         "  AND nodetype = 0 ".
	         "ORDER BY cid DESC ".
	         "LIMIT ".(($pno - 1) * $etemnum)." , ".$etemnum." ;";
	$result = mysql_query($query,$link);
	for($i = 0; $i < mysql_num_rows($result) ; $i ++ )
	{
		$rows = mysql_fetch_array($result);
		$newComments[$i] = array(
					"CID" => $rows["cid"],
					"UID" => $rows["uid"],
					"CCREATED" => time_format($rows[3]),
					"NCREATED" => time_format($rows["created"]),
					"VISITCOUNT" => $rows["visitcount"],
					"COMMENTCOUNT" => $rows["commentcount"],
					"CSUBJECT" => html_format($rows[2]),
					"NSUBJECT" => html_format($rows["subject"]),
					"POSTER" => $rows["username"],
					"NID" => $rows["nid"]
					);	
	}
	mysql_free_result($result);
	return $newComments;
}

//nodes without body and user information
function getRecommendNodes($link,$num)
{
	$num = intval($num);
	$query = "SELECT nid , subject , uid FROM recommend ORDER BY state DESC, rid DESC LIMIT 0 , ".$num.";";
	$result = mysql_query($query,$link);	
	$nodes = array();
	while($rows = mysql_fetch_array($result))
		$nodes[] = $rows;
	mysql_free_result($result);
	return $nodes;
}


//nodes with body and user information
function getRecommendBlogs($link,$pno=1,$etemnum=0)
{
	global $pcconfig;
	if($pno < 1)
		$pno = 1;
	
	$etemnum = intval( $etemnum );
	if($etemnum <= 0 )
		$etemnum = $pcconfig["NEWS"];
	
	$start = ( $pno - 1 ) * $etemnum ;
	
	$query = "SELECT recommend.uid  , subject , body , htmltag , emote , hostname , created , recuser , nid , username , corpusname , description  ".
		 "FROM recommend , users ".
		 "WHERE recommend.uid = users.uid ".
		 "ORDER BY state DESC , rid DESC ".
		 "LIMIT ".$start." , ".$etemnum." ;";
	$result = mysql_query($query,$link);
	$num_rows = mysql_num_rows($result);
	
	$recommendBlogs = array();
	$recommendBlogs["channel"] = array(
			"siteaddr" => "http://".$pcconfig["SITE"],
			"title" => $pcconfig["BBSNAME"]."推荐Blog日志" ,
			"pcaddr" => "http://".$pcconfig["SITE"],
			"desc" => $pcconfig["BBSNAME"]."最新".$etemnum."个推荐日志",
			"email" => $pcconfig["BBSNAME"],
			"publisher" => $pcconfig["BBSNAME"],
			"creator" => $pcconfig["BBSNAME"],
			"rights" => $pcconfig["BBSNAME"],
			"date" => date("Y-m-d"),
			"updatePeriod" => "10分钟更新一次",
			"updateFrequency" => "最新的".$etemnum."个推荐日志",
			"updateBase" => date("Y-m-d H:i:s"),
			);
	for( $i = 0 ; $i < $num_rows ; $i ++ )
	{
		$rows = mysql_fetch_array($result);
		$body = "<br>\n".
			"来自: ".$rows["corpusname"]."<br>\n".
			"作者: ".$rows["username"]."<br>\n".
			"发信站: ".$pcconfig["BBSNAME"]."<br>\n".
			"时间: ".time_format($rows["created"])."<br>\n".
			"<hr size=1>\n".
			html_format($rows["body"],TRUE,$rows["htmltag"]).
			"<hr size=1>\n".
			"(<a href=\"http://".$pcconfig["SITE"]."/pc/pccon.php?id=".$rows[0]."&nid=".$rows["nid"]."&s=all\">浏览全文</a>\n".
			"<a href=\"http://".$pcconfig["SITE"]."/pc/pccom.php?act=pst&nid=".$rows["nid"]."\">发表评论</a>)<br>\n".
			"<a href=\"http://".$pcconfig["SITE"]."/pc/rss.php?userid=".$rows["username"]."\"><img src=\"http://".$pcconfig["SITE"]."/pc/images/xml.gif\" border=\"0\" align=\"absmiddle\" alt=\"XML\">Blog地址：http://".$pcconfig["SITE"]."/pc/rss.php?userid=".$rows["user"]."</a>";
		$recommendBlogs["useretems"][$i] = array(
					"addr" => "http://".$pcconfig["SITE"]."/pc/pccon.php?id=".$rows[0]."&amp;nid=".$rows[nid],
					"subject" => htmlspecialchars($rows["subject"]),
					"desc" => $body,
					"tid" => 0,
					"nid" => $rows["nid"],
					"publisher" => $pcconfig["BBSNAME"],
					"creator" => $rows["username"],
					"pc" => $rows[0],
					"created" => time_format($rows["created"]),
					"rights" => $rows["username"].".bbs@".$pcconfig["SITE"]
					);
	}
	mysql_free_result($result);
	return $recommendBlogs;
}

function getNewUsers($link,$userNum=0)
{
	$userNum = intval( $userNum );
	if(!$userNum) $userNum = 10;
	$query = "SELECT username,corpusname,description FROM users WHERE pctype < 2 ORDER BY createtime DESC LIMIT 0,".intval($userNum).";";
	$result = mysql_query($query,$link);
	$newUsers = array();
	while($rows = mysql_fetch_array($result))
		$newUsers[] = $rows;
	mysql_free_result($result);
	return $newUsers;
}

function getMostVstUsers($link,$userNum=0)
{
	$userNum = intval( $userNum );
	if(!$userNum) $userNum = 10;
	$query = "SELECT username , corpusname , description FROM users WHERE pctype < 2 ORDER BY visitcount DESC LIMIT 0,".intval($userNum).";";
	$result = mysql_query($query,$link);
	$mostVstUsers = array();
	while($rows = mysql_fetch_array($result))
		$mostVstUsers[] = $rows;
	mysql_free_result($result);
	return $mostVstUsers;
}
	
function getLastUpdates($link,$userNum=0)
{
	$userNum = intval( $userNum );
	if(!$userNum) $userNum = 10;
	$query = "SELECT username , corpusname , description FROM users WHERE createtime != modifytime AND pctype < 2 ORDER BY modifytime DESC LIMIT 0,".intval($userNum).";";
	$result = mysql_query($query,$link);
	$lastUpdates = array();
	while($rows = mysql_fetch_array($result))
		$lastUpdates[] = $rows;
	mysql_free_result($result);
	return $lastUpdates;
}

function getUsersCnt($link)
{
	$query = "SELECT COUNT(*) FROM users;";
	$result = mysql_query($query);
	$rows = mysql_fetch_row($result);
	mysql_free_result($result);
	return $rows[0];
}
	
function getNodesCnt($link)
{
	$query = "SELECT COUNT(*) FROM nodes WHERE type != 1;";
	$result = mysql_query($query);
	$rows = mysql_fetch_row($result);
	mysql_free_result($result);
	return $rows[0];
}

function getCommentsCnt($link)
{
	$query = "SELECT COUNT(*) FROM comments;";
	$result = mysql_query($query);
	$rows = mysql_fetch_row($result);
	mysql_free_result($result);
	return $rows[0];
}

function getHotUsersByPeriod($link,$period,$num=10)
{
	$num = intval( $num );
	if($period=="day")
		$queryTime = time () - 24*3600;
	elseif($period=="month")
		$queryTime = time () - 30*24*3600;
	else
		$queryTime = 0;
	
	if($queryTime)
	    $query = "SELECT COUNT(pri_id) , uid , users.username , corpusname , description ".
	             "FROM logs , users ".
	             "WHERE ".//ACTION LIKE '%\'s Blog(www)' ".
	             "       pri_id = users.username ".
	             "      AND logtime > FROM_UNIXTIME(".$queryTime.")".
	             "      AND pctype < 2 ".
		     "GROUP BY pri_id ".
		     "ORDER BY 1 DESC ".
		     "LIMIT 0 , ".$num." ;";
	else
	    $query = "SELECT corpusname , uid , username , description ".
	    	     "FROM users WHERE pctype < 2 ORDER BY visitcount DESC LIMIT 0 , ".$num." ;";
	$result = mysql_query($query,$link);
	$users = array();
	while($rows = mysql_fetch_array($result))
		$users[] = $rows;	
	mysql_free_result($result);
	return $users;	
}

function getScoreTopUsers($link, $num=10)
{
	$query = "SELECT `corpusname`,`uid`,`username`,`description` FROM `users` WHERE `pctype`<2 ORDER BY `score` DESC LIMIT 0,{$num}";
	$result = mysql_query($query, $link);
	$users = array();
	while($rows = mysql_fetch_array($result))
		$users[] = $rows;
	mysql_free_result($result);
	return $users;
}

function getHotNodesByPeriod($link,$period,$num=10)
{
	$num = intval( $num );
	if($period=="day")
		$queryTime = time () - 24*3600;
	elseif($period=="month")
		$queryTime = time () - 30*24*3600;
	else
		$queryTime = 0;

	// kxn: 这里需要判断一下，如果是没有时间限制的，那么应该强制 nodes 使用 visitcount 做索引，
	//      如果有时间限制的话，就不要去动他，用时间限制做主  key , 效果会好
	$nodesindex = $queryTime == 0 ? " use index(visitcount) " : "";
		
	$query = "SELECT nodes.uid , nid , subject ".
		 "FROM nodes ".$nodesindex.",users ".
		 "WHERE access = 0 ".
		 "   AND nodes.uid = users.uid ".
		 "   AND nodetype = 0 ".
		 "   AND pctype < 4 ";
	
	if($queryTime)
	$query.= " AND created > FROM_UNIXTIME(".$queryTime . ")";
	//$query.= "GROUP BY uid ";
	$query.= "ORDER BY nodes.visitcount DESC ".
		 "LIMIT 0 , ".$num." ;";
	$result = mysql_query($query,$link);
	$nodes = array();
	while($rows = mysql_fetch_array($result))
		$nodes[] = $rows;	
	mysql_free_result($result);
	return $nodes;	
}

function getHotTopicsByPeriod($link,$period,$num=10)
{
	$num = intval( $num );
	if($period=="day")
		$queryTime = time () - 24*3600;
	elseif($period=="month")
		$queryTime = time () - 30*24*3600;
	else
		$queryTime = 0;
	
	$query =  "SELECT COUNT(nid) , topics.tid , topics.uid , topicname , username ".
	          "FROM topics , nodes , users ".
		  "WHERE topics.access = 0 ".
		  "      AND topics.uid = users.uid ".
		  "      AND topics.tid = nodes.tid ".
		  "      AND nodetype = 0 ".
		  "      AND pctype < 4 ";
	if($queryTime)
	$query.=  "      AND nodes.created > FROM_UNIXTIME(".$queryTime.")";
	$query.=  "GROUP BY nodes.tid ".
		  "ORDER BY 1 DESC ".
		  "LIMIT 0 , ".$num." ;";
        $result = mysql_query( $query , $link );
        $topics = array();
        while( $rows = mysql_fetch_array( $result ))
               $topics[] = $rows;
        mysql_free_result($result);
        return $topics;
}

function getPcAnnounce($link,$num=5)
{
	global $pcconfig;
	$num = intval($num);
	$query = "SELECT users.uid , subject , nid FROM nodes ,users WHERE access = 0 AND nodes.uid = users.uid AND username = '".$pcconfig["ADMIN"]."' ORDER BY nid DESC LIMIT 0 , " . $num . ";";
	$result = mysql_query($query,$link);
	$anns = array();
	while($rows = mysql_fetch_array($result))
		$anns[] = $rows;
	mysql_free_result($result);
	return $anns;
}

function getHotNodes($link,$type,$timeLong=259200,$num=20)
{
	$timeLong = intval($timeLong);
	$num = intval($num);
	
	if("comments" == $type)
		$query = "SELECT nid , subject , nodes.uid FROM nodes,users WHERE nodes.uid = users.uid AND nodetype = 0 AND pctype < 4 AND access = 0 AND type = 0 AND recommend != 2 AND created > FROM_UNIXTIME(".(time()-  $timeLong ).") ORDER BY commentcount DESC , nid DESC LIMIT 0 , ".$num.";";
	elseif("trackbacks" == $type)
		$query = "SELECT nid , subject , nodes.uid FROM nodes,users WHERE nodes.uid = users.uid AND nodetype = 0 AND pctype < 4 AND access = 0 AND type = 0 AND recommend != 2 AND created > FROM_UNIXTIME(".(time()-  $timeLong ).") AND trackbackcount != 0 ORDER BY trackbackcount DESC , nid DESC LIMIT 0 , ".$num.";";
	else
		$query = "SELECT nid , subject , nodes.uid  FROM nodes,users WHERE nodes.uid = users.uid AND nodetype = 0 AND pctype < 4 AND access = 0 AND type = 0 AND recommend != 2 AND created > FROM_UNIXTIME(".(time()- $timeLong ).") AND nodes.visitcount != 0 ORDER BY nodes.visitcount DESC , nid DESC LIMIT 0 , ".$num.";";
	
	$result = mysql_query($query,$link);	
	$nodes = array();
	while($rows = mysql_fetch_array($result))
		$nodes[] = $rows;
	mysql_free_result($result);
	return $nodes;
}

function getRecommendNodesByTopic($link,$topic,&$nodes)
{
    global $pcconfig;
    if (!$topic || !$pcconfig["SECTION"][$topic])
        return false;
    $nodes = array();
    
    $query = 'SELECT * FROM recommend WHERE topic = \''.addslashes($topic).'\' ORDER BY state DESC , rid DESC LIMIT 0 , 10;';
    $result = mysql_query($query,$link);
    if (!mysql_num_rows($result))
        return false;
    while($rows=mysql_fetch_array($result))
        $nodes[] = $rows;
    mysql_free_result($result);
    return true;
}

function getSectionHotNodes($link,$section,$timeLong,$num)
{
    global $pcconfig;
    if (!$section || !$pcconfig["SECTION"][$section])
        return false;    
    $query = "SELECT nodes.uid , nid , subject , username , corpusname  ".
             " FROM nodes use index(created) ,users ".
             " WHERE nodes.uid = users.uid ".
             "   AND nodetype = 0 ".
             "   AND pctype <= 4 ".
             "   AND access = 0 ".
             "   AND type = 0 ".
             "   AND recommend != 2 ".
             "   AND nodes.theme = '".addslashes($section)."'".
             "   AND created > FROM_UNIXTIME(".(time()- $timeLong ).")".
             "   AND nodes.visitcount != 0 ".
             " ORDER BY nodes.visitcount DESC , nid DESC ".
             " LIMIT 0 , ".intval($num).";";
    $result = mysql_query($query,$link);	
    if (mysql_num_rows($result)==0)
	    return false;
	$nodes = array();
	while($rows = mysql_fetch_array($result))
		$nodes[] = $rows;
	mysql_free_result($result);
	return $nodes;
}


?>
