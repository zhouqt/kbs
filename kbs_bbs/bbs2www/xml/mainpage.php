<?php
require("site.php");
if (!bbs_ext_initialized())
	bbs_init_ext();

# iterate through an array of nodes
# looking for a text node
# return its content
function get_content($parent)
{
    $nodes = $parent->child_nodes();
    while($node = array_shift($nodes))
        if ($node->node_type() == XML_TEXT_NODE)
            return $node->node_value();
    return "";
}

# get the content of a particular node
function find_content($parent,$name)
{
    $nodes = $parent->child_nodes();
    while($node = array_shift($nodes))
        if ($node->node_name() == $name)
            return get_content($node);
    return "";
}

# get an attribute from a particular node
function find_attr($parent,$name,$attr)
{
    $nodes = $parent->child_nodes();
    while($node = array_shift($nodes))
        if ($node->node_name() == $name)
            return $node->get_attribute($attr);
    return "";
}

?>
<?php
function gen_hot_subjects_html()
{
# load xml doc
$hotsubject_file = BBS_HOME . "/xml/day.xml";
$doc = domxml_open_file($hotsubject_file) or die("Can't open hot subject file!");

$root = $doc->document_element();
$boards = $root->child_nodes();


$brdarr = array();
?>
	<table width="600" border="0" cellpadding="0" cellspacing="0" background="images/lan1.gif" class="title">
        <tr> 
		  <td width="23">&nbsp;</td>
          <td>&gt;&gt;本日热点话题讨论&gt;&gt;</td>
        </tr>
	</table>
	<table border="0" cellpadding="0" cellspacing="0" width="600">
<?php
# shift through the array
while($board = array_shift($boards))
{
    if ($board->node_type() == XML_TEXT_NODE)
        continue;

    $hot_title = find_content($board, "title");
    $hot_author = find_content($board, "author");
    $hot_board = find_content($board, "board");
    $hot_time = find_content($board, "time");
    $hot_number = find_content($board, "number");
    $hot_groupid = find_content($board, "groupid");

	$brdnum = bbs_getboard($hot_board, $brdarr);
	if ($brdnum == 0)
		continue;
	$brd_encode = urlencode($brdarr["NAME"]);
?>
              <tr height="22"> 
<td width="15" class="MainContentText"></td>
                <td class="MainContentText"><li><a href="/cgi-bin/bbs/bbstfind?board=<?php echo $brd_encode; ?>&title=<?php echo urlencode(iconv("UTF-8", "GBK", $hot_title)); ?>"><?php echo htmlspecialchars(iconv("UTF-8", "GBK", $hot_title)); ?></a>&nbsp;&nbsp;[作者: <a href="/cgi-bin/bbs/bbsqry?userid=<?php echo $hot_author; ?>"><?php  echo $hot_author; ?></a>]&nbsp;&nbsp;&lt;<a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>"><?php  echo htmlspecialchars($brdarr["DESC"]); ?></a>&gt;</li></td>
              </tr>
<?php
}
?>
		</table>
<?php
}

function gen_sections_html()
{
global $section_nums;
global $section_names;

# load xml doc
$boardrank_file = BBS_HOME . "/xml/board.xml";
$doc = domxml_open_file($boardrank_file) or die("What boards?");


$root = $doc->document_element();
$boards = $root->child_nodes();

$sec_count = count($section_nums);
$sec_boards = array();
$sec_boards_num = array();
for ($i = 0; $i < $sec_count; $i++)
{
	$sec_boards[$i] = array();
	$sec_boards_num[$i] = 0;
}

# shift through the array
while($board = array_shift($boards))
{
    if ($board->node_type() == XML_TEXT_NODE)
        continue;

    $ename = find_content($board, "EnglishName");
    $cname = find_content($board, "ChineseName");
    $visittimes = find_content($board, "VisitTimes");
    $staytime = find_content($board, "StayTime");
    $secid = find_content($board, "SecId");
	$sec_boards[$secid][$sec_boards_num[$secid]]["EnglishName"] = $ename;
	$sec_boards[$secid][$sec_boards_num[$secid]]["ChineseName"] = iconv("UTF-8", "GBK", $cname);
	$sec_boards[$secid][$sec_boards_num[$secid]]["VisitTimes"] = $visittimes;
	$sec_boards[$secid][$sec_boards_num[$secid]]["StayTime"] = $staytime;
	$sec_boards_num[$secid]++;
}
?>
	<table width="600" border="0" cellpadding="0" cellspacing="0" background="images/lan3.gif" class="title">
        <tr> 
		  <td width="23">&nbsp;</td>
          <td>&gt;&gt;分类精彩讨论区&gt;&gt;</td>
        </tr>
	</table>
		<table border="0" cellpadding="0" cellspacing="0" width="600">
<?php
	for ($i = 0; $i < $sec_count; $i += 3)
	{
?>
        <tr> 
<?php
		for ($j = $i; $j < $i + 3; $j++)
		{
			if ($j < $sec_count)
			{
?>
          <td width="200" height="107" valign="top"  class="MainContentText"> 
<strong><?php echo $section_nums[$j]; ?>. [<a href="bbsboa.php?group=<?php echo $j; ?>"><?php echo htmlspecialchars($section_names[$j][0]); ?></a>]</strong><br><br>
<?php
			$brd_count = $sec_boards_num[$j] > 5 ? 5 : $sec_boards_num[$j];
			for ($k = 0; $k < $brd_count; $k++)
			{
?>
&nbsp;&lt;<a href="bbsdoc.php?board=<?php echo urlencode($sec_boards[$j][$k]["EnglishName"]); ?>"><?php echo $sec_boards[$j][$k]["ChineseName"]; ?></a>&gt;<br>
<?php
			}
?>
<br>
<div align="right"><a href="bbsboa.php?group=<?php echo $j; ?>">更多版面&gt;&gt;</a>&nbsp;&nbsp;</div>
</td>
<?php
			}
			else
			{
?>
<td width="33%" height="107" valign="top"  class="MainContentText">&nbsp;</td>
<?php
			}
			if ($j != $i + 2)
			{
?>
<td width="1" bgcolor="FFFFFF"></td>
<?php
			}
		}
?>
        </tr>
<?php
		if ($sec_count - $i > 3)
		{
?>
        <tr> 
          <td colspan="5" height="1" bgcolor="FFFFFF"></td>
        </tr>
<?php
		}
	}
?>
      </table>
<?php
}

function gen_recommend_boards_html()
{
	$rcmd_boards = array(
		"PieBridge",
		"Movie",
		"Love",
		"AdvancedEDU",
		"Game"
		);
?>
      <table width="150" height="18" border="0" cellpadding="0" cellspacing="0" class="helpert">
        <tr> 
          <td width="16" background="images/lt.gif">&nbsp;</td>
          <td width="66" bgcolor="#0066CC">推荐版面</td>
          <td width="16" background="images/rt.gif"></td>
          <td>&nbsp;</td>
        </tr>
      </table>
      <table width="150" border="0" cellpadding="0" cellspacing="0" class="helper">
<?php
	$brdarr = array();
	for ($i = 0; $i < count($rcmd_boards); $i++)
	{
			$brdnum = bbs_getboard($rcmd_boards[$i], $brdarr);
			if ($brdnum == 0)
				continue;
			$brd_encode = urlencode($brdarr["NAME"]);
?>
              <tr> 
                <td width="170" height="20" class="MainContentText"><li>&lt;<a href="bbsdoc.php?board=<?php echo $brd_encode; ?>"><?php echo htmlspecialchars($brdarr["DESC"]); ?></a>&gt;</li></td>
              </tr>
<?php
	}
?>
      </table>
      <br>
<?php
}

function gen_board_rank_html()
{
# load xml doc
$boardrank_file = BBS_HOME . "/xml/board.xml";
$doc = domxml_open_file($boardrank_file) or die("What boards?");


$root = $doc->document_element();
$boards = $root->child_nodes();

?>
      <table width="150" height="18" border="0" cellpadding="0" cellspacing="0" class="helpert">
        <tr> 
          <td width="16" background="images/lt.gif">&nbsp;</td>
          <td width="66" bgcolor="#0066CC">人气排名</td>
          <td width="16" background="images/rt.gif"></td>
          <td>&nbsp;</td>
        </tr>
      </table>
      <table width="150" border="0" cellpadding="0" cellspacing="0" class="helper">
<?php
$i = 0;
# shift through the array
while($board = array_shift($boards))
{
	if ($board->node_type() == XML_TEXT_NODE)
		continue;

	$ename = find_content($board, "EnglishName");
	$cname = find_content($board, "ChineseName");
?>
              <tr> 
                <td height="20" class="MainContentText"><?php echo $i+1; ?>. <a href="bbsdoc.php?board=<?php echo urlencode($ename); ?>"><?php echo htmlspecialchars(iconv("UTF-8", "GBK", $cname)); ?></a></td>
              </tr>
<?php
	$i++;
	if ($i == 5)
		break;
}
?>
      </table>
	  <br>
<?php
}
?>
<html>
<head>
<title></title>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<link href="mainpage.css" rel="stylesheet" type="text/css">
</head>
<body leftmargin="5" topmargin="0" marginwidth="0" marginheight="0">
<table border="0" cellpadding="0" cellspacing="0" width="800">
  <tr> 
    <td colspan="2" height="77"><img src="images/logo.gif" width="144" height="71"></td>
    <td colspan="6" ></td>
  </tr>
  <tr> 
    <td height="18" width="84" class="header" align="center">系统公告</td>
    <td width="84" class="header" align="center">推荐文章</td>
    <td width="80" class="header" align="center">分类讨论区</td>
    <td width="80" class="header" align="center">推荐版面</td>
    <td width="81" class="header" align="center">人气排名</td>
    <td width="79" class="header" align="center">本日祝福</td>
    <td width="56" class="header"></td>
    <td class="header" align="right" width="315"> <input type="text" name="bsearch" size="15" maxlength="30" value="版面搜索" class="text"> 
      <input type="button" name="search" size="15" value="GO" class="button"> 
    </td>
  </tr>
</table>
<table border="0" cellpadding="0" cellspacing="0" width="800">
  <tr>
    <td colspan="5" height="8"></td>
  </tr>
  <tr>
    <td width="600">
<?php
	gen_hot_subjects_html();
?>
<br>
<?php
	//gen_recommend_boards_html();
	if (0)
	{
?>
<!-- 推荐文章 暂时去掉
	<table width="626" border="0" cellpadding="0" cellspacing="0" background="images/lan2.gif" class="title">
        <tr> 
		  <td width="23">&nbsp;</td>
          <td>&gt;&gt;推荐文章&gt;&gt;</td>
        </tr>
	</table>
		<table border="0" cellpadding="0" cellspacing="0" width="626">
              <tr>
                <td valign="top" class="MainContentText"><p><img src="images/xia.gif" width="9" height="7"><a href="#">一个清华弃子的本科生活<br>
              </a>大学四年匆匆而逝，有得有失，喜忧参半。入学的迷茫，大一的忙碌，大二的郁闷，大三的彷徨，还都历历在目，也将铭刻在心。直到将要告别清华，才有了一丝丝的惆怅， 
              才发现自己深深地爱上了清华。<br>
              <br>
            </p>
                  
            </td>
              </tr>
              <tr>
                <td valign="top" class="MainContentText"><p><img src="images/xia.gif" width="9" height="7"><a href="#">刚刚看完钢琴家，很感动！<br>
              </a>它讲述了一个真实的故事，一个犹太钢琴师在华沙的逃亡历程。影片将战争的残酷、德军 的野蛮以及犹太人的悲惨遭遇反映的一览无遗。在德国人统治的华沙城里，犹太人过着猪 
              狗不如的生活，影片中到处都能看到饿死或被枪杀的犹太人的尸体，50万犹太人被分批送<br>
              <br>
            </p>
                  
            </td>
              </tr>
              <tr>
                <td valign="top" class="MainContentText"><p><img src="images/xia.gif" width="9" height="7"><a href="#">[Weekend]兄弟们，C++的美好时代就要来临了!!!</a><br>
              这期程序员上说 borland要推出100%符合标准的编译器 另外，ms的vc.2004也是宣传为100%支持加上，gcc的良好发展势头，C++终于要彻底出头了!<br>
              <br>
            </p>
                  
            </td>
              </tr>
              <tr>
                <td valign="top" class="MainContentText"><p><img src="images/xia.gif" width="9" height="7"><a href="#">省钱大法——让你少花一半生活费</a><br>
              首先，找一个两人上班骑车距离都在半小时内的房子，最好是消费水平不高但生活便利 的小区，不远就能有菜市场。样的房子很多，平均房租1500 
              买两辆自行车，连锁带筐各种杂碎最多300，保管得好至少一年内不会丢，平均下来每 月交通费25<br>
              <br>
            </p> 
            </td>
              </tr>
              <tr>
                <td valign="top" class="MainContentText"><p><img src="images/xia.gif" width="9" height="7"><a href="#">人行横道线的由来</a><br>
              在古罗马时代，意大利庞培市的一些街道上，人、马、车混行，交通经常堵塞。为了解决这个问题，人们把人行道加高，使人与马车分离。然后，又在接近马路口的 
              地方，横砌起一块块凸出路面的石头——跳石，作为指示行人过街的标志行人可以踩着 跳石穿过马路。<br>
              <br>
            </p> 
            </td>
              </tr>
		</table>
推荐文章 -->
<?php
	}
	gen_sections_html();
?>
</td>
    <td width="1" bgcolor="0066CC"></td>
    <td width="18">&nbsp;</td>
    <td width="150" align="left" valign="top"> 
<!-- 系统公告开始 暂时屏蔽掉
      <table width="150" height="18" border="0" cellpadding="0" cellspacing="0" class="helpert">
        <tr> 
          <td width="16" background="images/lt.gif">&nbsp;</td>
          <td width="66" bgcolor="#0066CC">系统公告</td>
          <td width="16" background="images/rt.gif"></td>
          <td>&nbsp;</td>
        </tr>
      </table>
      <table width="150" border="0" cellpadding="0" cellspacing="0" class="helper">
        <tr> 
                <td height="20" class="MainContentText"><font color="#6E9E54"><img src="images/wen.gif" width="9" height="7"> 
                  本站开通国际访问接口</font></td>
              </tr>
              <tr> 
                <td height="20" class="MainContentText"><font color="#6E9E54"><img src="images/wen.gif" width="9" height="7"> 
                  新版WEB界面启用</font></td>
              </tr>
              <tr> 
                <td height="20" class="MainContentText"><img src="images/wen.gif" width="9" height="7"></td>
              </tr>
              <tr> 
                <td height="20" class="MainContentText"><img src="images/wen.gif" width="9" height="7"></td>
              </tr>
              <tr> 
                <td height="20" class="MainContentText"><img src="images/wen.gif" width="9" height="7"></td>
        </tr>
      </table>
      <br>
系统公告结束   -->
<?php
	gen_recommend_boards_html();
	gen_board_rank_html();
?>
      <table width="150" height="18" border="0" cellpadding="0" cellspacing="0" class="helpert">
        <tr> 
          <td width="16" background="images/lt.gif">&nbsp;</td>
          <td width="66" bgcolor="#0066CC">今日祝福</td>
          <td width="16" background="images/rt.gif"></td>
          <td>&nbsp;</td>
        </tr>
      </table>
      <table width="150" border="0" cellpadding="0" cellspacing="0" class="helper">
              <tr> 
                <td width="170" height="20" class="MainContentText"><font color="#FF6600"><img src="images/xin.gif" width="9" height="7"> 
                  祝伟大祖国繁荣富强</font></td>
              </tr>
              <tr> 
                <td height="20" class="MainContentText"><font color="#FF6600"><img src="images/xin.gif" width="9" height="7"> 
                  祝水木清华蒸蒸日上</font></td>
              </tr>
              <tr> 
                <td height="20" class="MainContentText"><img src="images/xin.gif" width="9" height="7"></td>
              </tr>
              <tr> 
                <td height="20" class="MainContentText"><img src="images/xin.gif" width="9" height="7"></td>
              </tr>
              <tr> 
                <td height="20" class="MainContentText"><img src="images/xin.gif" width="9" height="7"></td>
              </tr>
      </table>
	  </td>
    <td width="10">&nbsp;</td>
  </tr>
</table>
<hr class="smth">
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td align="center" class="smth">版权所有 &copy; BBS 水木清华站 1995-2003 <a href="certificate.html">京ICP备02002号</a></td>
  </tr>
</table>
<br>
