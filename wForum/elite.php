<?php

$setboard=1;

require("inc/funcs.php");
require("inc/board.inc.php");
require("inc/user.inc.php");
require("inc/attachment.inc.php");
require("inc/ubbcode.php");

global $boardName;
global $boardArr;
global $boardID;
global $parent;
global $file;
global $cacheit;

setStat("精华区");

preprocess();

if ($file !== false) {
	@$attachpos=$_GET["ap"];
	if ($attachpos!=0) {
		if ($cacheit) {
			if (cache_header('public',filemtime($file),300))
				return;
		}
		output_attachment($file, $attachpos);
		exit;
	}
}

setStat("精华区文章".($file===false?"列表":"阅读"));

show_nav($boardName);

showUserMailBoxOrBR();
if ($boardID) {
	board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);
	if ($file === false) {
		require("inc/conn.php"); //只有这个时候才需要
?>
<table cellSpacing=0 cellPadding=0 width=97% border=0 align=center>
<?php
		showAnnounce(); 
?>
</table>
<?php
		showBoardStaticsTop($boardArr, true);
?>
<table cellPadding=1 cellSpacing=1 class=TableBorder1 align=center>
<?php
		showBroadcast($boardID,$boardName);
?>
</table>
<?php
	}
} else {
	head_var("精华区", "elite.php");
}
if ($file !== false) {
	ann_display_file($file, $parent);
} else {
	ann_display_folder($articles, $parent);
}

show_footer();

function preprocess(){
	global $articles;
	global $boardName;
	global $boardArr;
	global $boardID;
	global $parent;
	global $currentuser;
	global $file;
	global $cacheit;
	
	$file = false;
	
	if (isset($_GET['path']))
		$path = trim($_GET['path']);
	else 
		$path = "";
	
	if (isset($_GET['file'])) {
		$path = trim($_GET['file']);
		if($path[0]=='/')
			$path = "0Announce".$path;
		else
			$path = "0Announce/".$path;
		$file = $path;
	}
	
	if (strstr($path, '.Names') || strstr($path, '..') || strstr($path, 'SYSHome')) {
	    foundErr('不存在该目录');
	}
	
	$boardName = '';
	$articles = array();
	$path_tmp = '';
	if ($file === false) {
		$ret = bbs_read_ann_dir($path,$boardName,$path_tmp,$articles);
		switch ($ret) {
		    case -1:
		        foundErr('精华区目录不存在');
		    case -2:
		        foundErr('无法加载目录文件');
		    case -3:
		        /* 其实所有的信息都有返回，只不过 $articles 没东西而已，可以继续处理. See Also bbs_read_ann_dir() in phpbbslib.c.
		        foundErr('该目录尚无文章');
		        */
		        break;
		    case -9:
		        foundErr('系统错误');
		    default;
		}
		$path = $path_tmp;
	} else {
		if( bbs_ann_traverse_check($path, $currentuser["userid"]) < 0 ) {
			foundErr("错误的目录");
		}
	}
	$parent = '';
	
	$up_dirs = array();
	$up_cnt = bbs_ann_updirs($path,$boardName,$up_dirs);
	$cacheit = true;
	if ($up_cnt >= 2)
    	$parent = $up_dirs[$up_cnt - 2];
	if ($boardName) {
	    $brdArr = array();
	    $boardID = bbs_getboard($boardName,$brdArr);
	    $boardArr = $brdArr;
	    if ($boardID) {
	        $boardName = $boardArr['NAME'];
	        $usernum = $currentuser['index'];
	        if (bbs_checkreadperm($usernum, $boardID) == 0) {
	    		foundErr('不存在该目录');
	    	}
	        bbs_set_onboard($boardID,1);
	        if (!bbs_normalboard($board)) $cacheit = false;
	    }
	    else {
	        $boardName = '';
	    }
	}
	else {
	    $boardID = 0;
	}
	return true;
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
        if ($dir) {
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

function ann_display_folder($articles, $parent) {
?>
<table cellPadding=1 cellSpacing=1 class=TableBorder1 align=center>
<tr>
	<th height=25 width=50>序号</th>
	<th width=30>类型</th>
	<th width=*>标 题</th>
	<th width=80>整理</th>
	<th width=80>编辑日期</th>
</tr>
<?php
	if ($parent != "") {
?>
<tr>
	<td height="27" align="center" class="TableBody1">0</td>
	<td align="center" class="TableBody2"><img src="pic/istop.gif" alt="目录" border="0" /></td>
	<td class="TableBody1">&nbsp;<a href="elite.php?path=<?php echo urlencode($parent); ?>">上级目录</a></td>
	<td align="center" class="TableBody2">&nbsp;</td>
	<td align="center" class="TableBody1">&nbsp;</td>
</tr>
<?php
	}
	if (count($articles) == 0) {
		echo '<tr><td height="27" align="center" colspan="5" class="TableBody1">该目录尚无文章</td></tr>';
	} else {
	    $i = 1;
	    foreach ($articles as $article) {
	        switch($article['FLAG']) {
	            case 0:
	                continue 2;
	            case 1:
	                $img = 'pic/ifolder.gif';
	                $alt = '目录';
	                $url = 'elite.php?path='.urlencode($article['PATH']);
	                break;
	            case 2:
	            case 3:
	            default:
	                $img = 'pic/folder.gif';
	                $alt = '文件';
	                $url = 'elite.php?file='.urlencode($article['PATH']);
	        }
	        echo '<tr><td height="27" align="center" class="TableBody1">'.$i.'</td><td align="center" class="TableBody2">';
	        echo '<img src="'.$img.'" alt="'.$alt.'" border="0" />';
	        echo '</td><td class="TableBody1">';
	        if ($article['FLAG']==3)
	            echo '<font color="red">@</font>';
	        else
	            echo '&nbsp;';
	        $title = htmlformat(trim($article['TITLE']))." ";
	        echo '<a href="'.$url.'">'.$title.'</a>';
	        $bm = explode(' ',$article['BM']);
	        $bm = $bm[0];
	        echo '</td><td align="center" class="TableBody2">'.($bm?'<a target="_blank" href="dispuser.php?id='.$bm.'">'.$bm.'</a>':'&nbsp;').'</td><td align="center" class="TableBody1">'.date('Y-m-d',$article['TIME']).'</td></tr>';
	        $i ++;
	    }
    }
?>
</table>
<?php    
}

function ann_display_file($filename, $parent) {
?>
<table cellPadding=1 cellSpacing=1 align=center class=TableBorder1 style=" table-layout:fixed;word-break:break-all">
<tr><th height="25" width="100%" class=TableBody2>精华区文章阅读</th></tr>
<tr><td width="100%" style="font-size:9pt;line-height:12pt;padding:10px" class=TableBody1>
<?php
		echo dvbcode(bbs_printansifile($filename,1,'elite.php?file='.urlencode($_GET['file'])),0,"TableBody1");
?>
</td></tr>
<tr><td height="20" align="center" class=TableBody2>[<a href="elite.php?path=<?php echo urlencode($parent); ?>">返回精华区目录</a>]</td></tr></table>
<?php
}
?>
