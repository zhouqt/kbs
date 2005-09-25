<?php
function bbs_boards_navigation_bar()
{
?>
<p align="center">
[<a href="/<?php echo MAINPAGE_FILE; ?>">首页导读</a>]
[<a href="/bbssec.php">分类讨论区</a>]
[<a href="/bbsnewbrd.php">新开讨论区</a>]
[<a href="/bbsrecbrd.php">推荐讨论区</a>]
[<a href="/bbsbrdran.php">讨论区人气排名</a>]
[<a href="/bbs0an.php">精华公布栏</a>]
[<a href="javascript:history.go(-1)">快速返回</a>]
<br />
</p>
<?php	
}

function undo_html_format($str)
{
	$str = preg_replace("/&apos;/i", "'", $str);
	$str = preg_replace("/&gt;/i", ">", $str);
	$str = preg_replace("/&lt;/i", "<", $str);
	$str = preg_replace("/&quot;/i", "\"", $str);
	$str = preg_replace("/&amp;/i", "&", $str);
	return $str;
}

if (version_compare(PHP_VERSION,'5','>='))
	require_once('domxml-php4-to-php5.inc.php'); //Load the PHP5 converter

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
            return undo_html_format(urldecode(get_content($node)));
    return "";
}

function bbs_board_header($brdarr,$articles=0)
{
	global $section_names,$currentuser;
	$brd_encode = urlencode($brdarr["NAME"]);
	$ann_path = bbs_getannpath($brdarr["NAME"]);
	
	$bms = explode(" ", trim($brdarr["BM"]));
	$bm_url = "";
	if (strlen($bms[0]) == 0 || $bms[0][0] <= chr(32))
		$bm_url = "诚征版主中";
	else
	{
		if (!ctype_alpha($bms[0][0]))
			$bm_url = $bms[0];
		else
		{
			foreach ($bms as $bm)
			{
				$bm_url .= sprintf("<a class=\"b3\" href=\"/bbsqry.php?userid=%s\"><font class=\"b3\">%s</font></a> ", $bm, $bm);
			}
			$bm_url = trim($bm_url);
		}
	}
	
?>
<body topmargin="0" leftmargin="15">
<a name="listtop"></a>
<table width="100%" border="0" cellspacing="0" cellpadding="3">
<tbody>  <tr> 
    <td colspan="2" class="b2">
	    <a href="<?php echo MAINPAGE_FILE; ?>" class="b2"><font class="b2"><?php echo BBS_FULL_NAME; ?></font></a>
	    -
	    <?php
	    	$sec_index = get_secname_index($brdarr["SECNUM"]);
		if ($sec_index >= 0)
		{
	    ?>
		<a href="/bbsboa.php?group=<?php echo $sec_index; ?>" class="b2"><font class="b2"><?php echo $section_names[$sec_index][0]; ?></font></a>
	    <?php
		}
	    ?>
	    -
	    <a href="/bbsdoc.php?board=<?php echo $brdarr["NAME"]; ?>"><?php echo $brdarr["NAME"]." (".$brdarr["DESC"].")"; ?></a>版(<a href="bbsnot.php?board=<?php echo $brd_encode; ?>" class="b2"><font class="b2">进版画面</font></a>
	    |
	    <a href="/bbsfav.php?bname=<?php echo $brdarr["NAME"]; ?>&select=0" class="b2"><font class="b2">添加到收藏夹</font></a>
	    |
	    <?php bbs_add_super_fav ($brdarr['DESC'], '/bbsdoc.php?board='.$brdarr['NAME']); ?>
<?php
	if( defined("HAVE_BRDENV") ){
		if( bbs_board_have_envelop($brdarr["NAME"]) ){
?>
	    |
	    <a href="/bbsenv.php?board=<?php echo $brd_encode; ?>" class="b2"><font class="b2">版面导读</font></a>
<?php
		}
	}
?>
	    )
    </td>
  </tr>
  <tr> 
    <td colspan="2" align="center" class="b4"><?php echo $brdarr["NAME"]."(".$brdarr["DESC"].")"; ?> 版</td>
  </tr>
  <tr><td class="b1">
  <img src="images/bm.gif" alt="版主" align="absmiddle">版主 <?php echo $bm_url; ?>
  </td></tr>
  <tr> 
    <td class="b1">
    <img src="images/online.gif" alt="本版在线人数" align="absmiddle">在线 <font class="b3"><?php echo $brdarr["CURRENTUSERS"]+1; ?></font> 人
<?php
	if($articles)
	{
?>
    <img src="images/postno.gif" alt="本版文章数" align="absmiddle">文章 <font class="b3"><?php echo $articles; ?></font> 篇
<?php
	}
?>
    </td>
    <td align="right" class="b1">
	    <img src="images/gmode.gif" align="absmiddle" alt="文摘区"><a class="b1" href="bbsgdoc.php?board=<?php echo $brd_encode; ?>"><font class="b1">文摘区</font></a> 
<?php
    	if ($ann_path != FALSE)
	{
        	if (!strncmp($ann_path,"0Announce/",10))
			$ann_path=substr($ann_path,9);
?>
	    | 
  	    <img src="images/soul.gif" align="absmiddle" alt="精华区"><a class="b1" href="/bbs0an.php?path=<?php echo urlencode($ann_path); ?>"><font class="b1">精华区</font></a>
	    <?php
	}
?>
	    | 
  	    <img src="images/search.gif" align="absmiddle" alt="版内查询"><a class="b1" href="/bbsbfind.php?board=<?php echo $brd_encode; ?>"><font class="b1">版内查询</font></a>
<?php
	if (strcmp($currentuser["userid"], "guest") != 0)
	{
?>
	    | 
  	    <img src="images/vote.gif" align="absmiddle" alt="本版投票"><a class="b1" href="/bbsshowvote.php?board=<?php echo $brd_encode; ?>"><font class="b1">本版投票</font></a>
	    | 
  	    <img src="images/model.gif" align="absmiddle" alt="发文模板"><a class="b1" href="/bbsshowtmpl.php?board=<?php echo $brd_encode; ?>"><font class="b1">发文模板</font></a>
<?php
	}
?>
    	        </td>
  </tr>
  <tr> 
    <td colspan="2" height="9" background="images/dashed.gif"> </td>
  </tr></tbody>
</table>
<?php	
}

function bbs_board_foot($brdarr,$listmode='')
{
	global $currentuser;
	$brd_encode = urlencode($brdarr["NAME"]);
	$usernum = $currentuser["index"];
	$brdnum  = $brdarr["NUM"];
?>

<table width="100%" border="0" cellspacing="0" cellpadding="3">
<tbody>  <tr> 
    <td colspan="2" height="9" background="images/dashed.gif"> </td>
  </tr>
  <tr> 
    <td colspan="2" align="center" class="b1">
    	[<a href="#listtop">返回顶部</a>]
    	[<a href="javascript:location.reload()">刷新</a>]
<?php
	if ($listmode != "ORIGIN") {
?>
[<a href="bbsodoc.php?board=<?php echo $brd_encode; ?>">同主题模式</a>]
<?php		
    }
    if ($listmode != "NORMAL") {
?>
[<a href="bbsdoc.php?board=<?php echo $brd_encode; ?>">普通模式</a>]
<?php
	}
?>
[<a href="/bbsbfind.php?board=<?php echo $brd_encode; ?>">版内查询</a>]
<?php
if (bbs_is_bm($brdnum, $usernum))
{
?>
[<a href="/bbsdeny.php?board=<?php echo $brd_encode; ?>">封禁名单</a>] 
[<a href="/bbsmnote.php?board=<?php echo $brd_encode; ?>">进版画面</a>]
[<a href="/bbsmvote.php?board=<?php echo $brd_encode;?>">管理投票</a>]
<?php
    if ($listmode != 'MANAGE') {
?>
[<a href="/bbsmdoc.php?board=<?php echo $brd_encode; ?>">管理模式</a>]
<?php
    }
    else {
?>
[<a href="/bbsclear.php?board=<?php echo $brd_encode; ?>">清除未读</a>]
<?php        
    }
    
}
?>
    </td>
  </tr>
<?php
	$relatefile = $_SERVER["DOCUMENT_ROOT"]."/brelated/".$brdarr["NAME"].".html";
	if( file_exists( $relatefile ) )
	{
?>
<tr>
<td colspan="2" align="center" class="b1">
来这个版的人常去的其他版面：
<?php
	include($relatefile);
?>
</td>
</tr>
<?php
	}
?>
</tbody></table>
<?php	
}

function bbs_board_avtiveboards()
{
?>
<table width="100%" cellpadding="3" cellspacing="0" border="0" />
<tr>
	<td width="150" height="77"><img src="images/logo.gif"></td>
	<td><SPAN ID='aboards'>Active Boards</SPAN></td>
</tr>
</table>
<SCRIPT SRC='abs.js'></SCRIPT>
<script language='javascript'>
display_active_boards();
</script>
<?php	
}

function htmlformat($str,$multi=false) {
    $str = str_replace(' ','&nbsp;',htmlspecialchars($str));
    if ($multi)
        $str = nl2br($str);
    return $str;    
}

 
function bbs_ann_updirs($path,&$board,&$up_dirs) {
    $board = '';
    $path = ltrim(trim($path));
    if ($path[0]!='/') $path='/'.$path;
    if ($path[strlen($path)-1]=='/') $path = substr($path,0,strlen($path)-1);
    $up_dirs = array();
    $buf = '';
    $dirs = explode('/',$path);
    $j = 0;
    foreach($dirs as $dir) {
        if (($dir)&&($dir!='.')) {
            if (!strcmp('0Announce',$dir))
                continue;
            $buf .= '/'.$dir;
            $up_dirs[] = $buf;
            if ($j == 2) $board = $dir;    
            $j ++;
        }
    }
    return sizeof($up_dirs);
}

function bbs_ann_header($board='') {
?>
<body topmargin="0" leftmargin="0">
<a name="listtop"></a>
<table width="100%" border="0" cellspacing="0" cellpadding="3">
<tbody>  <tr> 
    <td colspan="2" class="b2">
	    <a href="<?php echo MAINPAGE_FILE; ?>" class="b2"><font class="b2"><?php echo BBS_FULL_NAME; ?></font></a>
	    -
	    精华公布栏
<?php
        if ($board)
            echo ' <a href="/bbsdoc.php?board='.$board.'">'.$board.'版</a>';
?>
    </td>
  </tr>
  <tr> 
    <td colspan="2" height="9" background="images/dashed.gif"> </td>
  </tr></tbody>
</table>
<?php
}

function bbs_ann_xsearch($board) {
?>
<table width="100%" border="0" cellspacing="0" cellpadding="9">
  <form action="/bbsxsearch.php" name="qf0">
<tr> 
    <td align="right" colspan="2">
	    <a href="/bbsxsearch.php">令狐冲精华区搜索引擎</a>
	     <input type="text" class="b1" name="q" value="" size="41" onmouseover="this.focus();" />
<?php
    if ($board) {
?>
搜索范围
<input class="b1" type="radio" name="b" value="" />全站
<input class="b1" type="radio" name="b" checked value="<?php echo urlencode($board); ?>" /><?php echo $board; ?>版
<?php        
    }
?>	     
	     <input type="submit" class="a" style="width: 80px" value="开始搜" class="a" />
    &nbsp;&nbsp;&nbsp;</td>
  </tr>
   </form>    
</table>
<?php
}


function bbs_ann_foot($parent) {
?>   
<p align="center">
[<?php bbs_add_super_fav ('精华区'); ?>]
[<a href="/<?php echo MAINPAGE_FILE; ?>">返回首页</a>]
<?php   
    if ($parent){
?>
[<a href="/bbs0an.php?path=<?php echo rawurlencode($parent); ?>">上级目录</a>]
<?php
    }
?>
[<a href="/bbs0an.php">根目录</a>]
[<a href="/bbsxsearch.php">令狐冲精华区搜索</a>]
[<a href="#listtop">返回顶部</a>]
[<a href="javascript:location.reload()">刷新</a>] 
[<a href="javascript:history.go(-1)">返回</a>] 

</p>
<?php    
}


?>
