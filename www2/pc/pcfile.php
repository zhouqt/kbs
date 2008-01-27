<?php
require("userfile.php");

function pc_file_navigationbar($pc)
{
?>	
<center>
[<a href="index.php?id=<?php echo $pc["USER"]; ?>"><?php echo $pc["NAME"]; ?></a>]
[<a href="pcdoc.php?userid=<?php echo $pc["USER"]; ?>&tag=7">参数设定</a>]
[<a href="/<?php echo MAINPAGE_FILE; ?>"><?php echo BBS_FULL_NAME; ?>首页</a>]
[<a href="javascript:history.go(-1);">快速返回</a>]
</center>
<?php	
}

function pc_file_infor($pc,$used,$total)
{
?>
<br />
<center><table cellspacing="0" cellpadding="5" border="0" class="t1" >
<tr><td class="t3">用户</td><td class="t3">总空间</td><td class="t3">已用空间</td><td class="t3">剩余空间</td><td class="t3">容量</td><td class="t3">文件数</td></tr>
<tr>
	<td class="t4"><?php echo $pc["USER"]; ?></td>
	<td class="t4"><?php echo sizestring($pc["FILELIMIT"]); ?></td>
	<td class="t4"><?php echo sizestring($used); ?></td>
	<td class="t4"><?php echo sizestring($pc["FILELIMIT"] - $used); ?></td>
	<td class="t4"><?php echo sizestring($pc["FILENUMLIMIT"]); ?></td>
	<td class="t4"><?php echo $total; ?></td>
</tr>
</table></center><br />
<?php
}

function pc_file_showfiles($pc,$c_dir,$root_pid)
{
    global $file_access;
    $c_files = array();
    if(!$c_dir->uf_readdir($c_files))
        exit($c_dir->err);
    $c_files_num = sizeof($c_files);
?>
<hr width="100%">
<p class="f2">&nbsp;&nbsp;&nbsp;&nbsp;
[当前目录：<?php echo ($c_dir->fid==$root_pid)?'根目录':$c_dir->filename; ?>]</p>
<center>
<table cellspacing="0" cellpadding="5" width="95%" class="t1" border="0">
<tr>
	<td class="t2" width="40">&nbsp;</td>
	<td class="t2">文件名</td>
	<td class="t2" width="60">权限</td>
	<td class="t2" width="80">大小</td>
	<td class="t2" width="160">类型</td>
	<td class="t2" width="120">修改时间</td>
	<td class="t2" width="20">改</td>
	<td class="t2" width="20">删</td>
	<td class="t2" width="20">复</td>
	<td class="t2" width="20">剪</td>
	
</tr>
<?php
    if ($root_pid != $c_dir->fid)
        echo '<tr><td colspan="10" class="t8">[<a href="'.$_SERVER["PHP_SELF"].'?userid='.$pc["USER"].'&pid='.$root_pid.'">返回根目录</a>]</td></tr>'.
             '<tr><td colspan="10" class="t8">[<a href="'.$_SERVER["PHP_SELF"].'?userid='.$pc["USER"].'&pid='.$c_dir->pid.'">返回上层目录</a>]</td></tr>';
        
    if ($c_files_num) {
	$start = 1;
	for($i = 0;$i < $c_files_num;$i ++) {
	        if (!($file = new UserFile($c_files[$i],$c_dir->uid)))
	            continue;
	        if (!$file->fid)
	            continue;
	        if ($file->type==1) {
	            echo '<tr><td class="t3">目录</td>';
	            $link_url = '<a href="'.$_SERVER["PHP_SELF"].'?userid='.$pc["USER"].'&pid='.$file->fid.'" title="'.htmlspecialchars($file->remark).'">';
	        }
	        else {
	            echo '<tr><td class="t3">'.$start.'</td>';
	            $start ++;
	            $link_url = '<a href="';
	            $link_url .= 'pcdownload.php?fid='.$file->fid.'" title="'.htmlspecialchars($file->remark).'" target="_blank">';
	        }
	        
    		echo '<td class="t5">'.$link_url.html_format($file->filename).'</a></td>'.
    		     '<td class="t3">'.$file_access[$file->access].'</td>'.
    		     '<td class="t4">'.sizestring($file->filesize).'</td>'.
    		     '<td class="t3">'.html_format($file->filetype).'</td>'.
    		     '<td class="t4">'.time_format($file->filetime).'</td>'.
    		     '<td class="t3"><a href="'.$_SERVER['PHP_SELF'].'?userid='.$pc['USER'].'&act=edit&fid='.$file->fid.'&pid='.$c_dir->fid.'">改</a></td>'.
    		     '<td class="t3"><a href="#" onclick="bbsconfirm(\''.$_SERVER['PHP_SELF'].'?userid='.$pc['USER'].'&act=rm&fid='.$file->fid.'&pid='.$c_dir->fid.'\',\'确实要删除'.addslashes(htmlspecialchars($file->filename)).'吗?\')" >删</a></td>';
    		if ($_COOKIE['PCFILEFID'])
    		{
    		    if ($file->type==1)
    		        echo '<td colspan="2" class="t3"><a href="'.$_SERVER['PHP_SELF'].'?userid='.$pc['USER'].'&act=pt&fid='.intval($_COOKIE['PCFILEFID']).'&pid='.$c_dir->fid.'&tid='.$file->fid.'">粘</a></td>';
    		    else
    		        echo '<td colspan="2" class="t3">-</td>';
    		}
    		else
    		    echo '<td class="t3"><a href="'.$_SERVER['PHP_SELF'].'?userid='.$pc['USER'].'&act=cp&fid='.$file->fid.'&pid='.$c_dir->fid.'">复</a></td>'.
    		         '<td class="t3"><a href="'.$_SERVER['PHP_SELF'].'?userid='.$pc['USER'].'&act=mv&fid='.$file->fid.'&pid='.$c_dir->fid.'">剪</a></td>';
    		     
    	    unset($file);
    	}
	}
	
?>
</table>
<?php
    if ($_COOKIE['PCFILEFID'])
        echo '<br />[<a href="'.$_SERVER['PHP_SELF'].'?userid='.$pc['USER'].'&act=pt&fid='.intval($_COOKIE['PCFILEFID']).'&pid='.$root_pid.'&tid='.$root_pid.'">粘贴到根目录</a>]&nbsp;'.
             '[<a href="'.$_SERVER['PHP_SELF'].'?userid='.$pc['USER'].'&act=cl&pid='.$c_dir->fid.'">清空剪贴板</a>]<br/>';
?>
</center>
<hr width="100%">
<?php
    
}

function pc_file_uploadform($pc,$pid,$c_dir)
{
    global $pcconfig;
?>
<center>
<br />
<table cellpadding="10"><tr><td>
<form action="<?php echo $_SERVER['PHP_SELF']; ?>?userid=<?php echo $pc["USER"]; ?>&pid=<?php echo $pid; ?>&act=add" method="post" ENCTYPE="multipart/form-data">
<table cellpadding="5" cellspacing="0" border="0" class="t1">
<tr><td colspan="2" class="t2">上传新文件</td></tr>
<tr><td class="t3">文件</td><td class="t5">
<input type="file" name="pcfile" size="20" class="f1">
</td></tr>
<?php
    if ($pcconfig["USERFILEPERM"]) {
?>
<tr><td class="t3">权限</td><td class="t5">
<input type="radio" name="fileaccess" class="f1" value="0" <?php if($c_dir->access==0) echo 'checked'; ?> />公开
<input type="radio" name="fileaccess" class="f1" value="1" <?php if($c_dir->access==1) echo 'checked'; ?>/>好友
<input type="radio" name="fileaccess" class="f1" value="2" <?php if($c_dir->access==2) echo 'checked'; ?>/>私有
</td></tr>
<?php   
    }
    else {
?>
<input type="hidden" name="fileaccess" value="0" />
<?php
    }
?>
<tr><td class="t3">备注</td><td class="t5">
<input type="text" name="fileremark" class="f1" />
</td></tr>
<tr><td colspan="2" align="center" class="t3">
<input type="submit" value="上传" class="f1" />
<input type="reset" value="重置" class="f1" />
</td></tr>
</table>
</form>
</td><td width="20"> </td><td>
<form action="<?php echo $_SERVER['PHP_SELF']; ?>?userid=<?php echo $pc["USER"]; ?>&pid=<?php echo $pid; ?>&act=mkdir" method="post" >
<table cellpadding="5" cellspacing="0" border="0" class="t1">
<tr><td colspan="2" class="t2">新建文件夹</td></tr>
<tr><td class="t3">名称</td><td class="t5">
<input type="text" name="dirname" size="20" class="f1">
</td></tr>
<?php
    if ($pcconfig["USERFILEPERM"]) {
?>
<tr><td class="t3">权限</td><td class="t5">
<input type="radio" name="diraccess" class="f1" value="0" <?php if($c_dir->access==0) echo 'checked'; ?> />公开
<input type="radio" name="diraccess" class="f1" value="1" <?php if($c_dir->access==1) echo 'checked'; ?>/>好友
<input type="radio" name="diraccess" class="f1" value="2" <?php if($c_dir->access==2) echo 'checked'; ?>/>私有
</td></tr>
<?php   
    }
    else {
?>
<input type="hidden" name="diraccess" value="0" />
<?php
    }
?>
<tr><td class="t3">备注</td><td class="t5">
<input type="text" name="dirremark" class="f1" />
</td></tr>
<tr><td colspan="2" align="center" class="t3">
<input type="submit" value="新建" class="f1" />
<input type="reset" value="重置" class="f1" />
</td></tr>
</table>
</form>
</td></tr></table>
</center>
<?php	
}

function pc_file_editform($pc,$pid,$f) {
    global $pcconfig;
?>
<center>
<br /><br /><br /><br />
<form action="<?php echo $_SERVER['PHP_SELF']; ?>?userid=<?php echo $pc["USER"]; ?>&pid=<?php echo $pid; ?>&act=edit2&fid=<?php echo $f->fid; ?>" method="post">
<table cellpadding="5" cellspacing="0" border="0" class="t1">
<tr><td colspan="2" class="t2">修改文件/目录</td></tr>
<tr><td class="t3">文件/目录名</td><td class="t5">
<input type="text" name="newname" size="20" class="f1" value="<?php echo htmlspecialchars($f->filename); ?>">
</td></tr>
<?php
    if ($pcconfig["USERFILEPERM"]) {
?>
<tr><td class="t3">权限</td><td class="t5">
<input type="radio" name="newaccess" class="f1" value="0" <?php if($f->access==0) echo 'checked'; ?>/>公开
<input type="radio" name="newaccess" class="f1" value="1" <?php if($f->access==1) echo 'checked'; ?>/>好友
<input type="radio" name="newaccess" class="f1" value="2" <?php if($f->access==2) echo 'checked'; ?>/>私有
</td></tr>
<?php   
    }
    else {
?>
<input type="hidden" name="newaccess" value="0" />
<?php
    }
?>
<tr><td class="t3">备注</td><td class="t5">
<input type="text" name="newremark" class="f1" value="<?php echo $f->remark; ?>" />
</td></tr>
<tr><td colspan="2" align="center" class="t3">
<input type="submit" value="修改" class="f1" />
<input type="reset" value="重置" class="f1" />
<input type="button" value="返回" class="f1" onclick="history.go(-1)" />
</td></tr>
</table>
</form>
<?php
}    

function pc_file_checkfree($pc,$used,$total,$filesize)
{
    if ($filesize+$used > $pc["FILELIMIT"])
	    return false;
	if ($total + 1 > $pc["FILENUMLIMIT"])
	    return false;
	return true;
}

if ($loginok != 1)
	html_nologin();
elseif(!strcmp($currentuser["userid"],"guest"))
{
	html_init("gb2312");
	html_error_quit("guest 没有Blog!");
}	
else
{
	$link = pc_db_connect();
	$pc = pc_load_infor($link,$_GET["userid"]);
	
	if(!$pc)
	{
		pc_db_close($link);
		html_error_quit("对不起，您要查看的Blog不存在");
	}
	if(!pc_is_admin($currentuser,$pc))
	{
		pc_db_close($link);
		html_error_quit("对不起，您要查看的Blog不存在");
	}
	
	$root_pid = bbs_userfile_getrootfid($pc["UID"]);
	$pid = intval($_GET['pid'])?$_GET['pid']:$root_pid;
	pc_get_userfiles($link,$pc,$used,$total);
	
	if (!($c_dir = new UserFile($pid,$pc["UID"])))
	{
		pc_db_close($link);
		html_error_quit("对不起，您要查看的目录不存在");
	}   
		
    if ($_GET['act']=='edit' || $_GET['act']=='edit2' || $_GET['act']=='rm' || $_GET['act']=='cp' || $_GET['act']=='mv' || $_GET['act']=='pt') {
        $fid = intval($_GET['fid']);
        if (!$fid)
	        html_error_quit("错误的参数");
	    if (!($f = new UserFile($fid,$pc['UID'])))
	        html_error_quit("文件/目录不存在");
	        
    }
    
	switch($_GET['act']) {
	    case 'add':
	        @$errno=$_FILES['pcfile']['error'];
	        if ($errno==UPLOAD_ERR_OK) {
	            if (!pc_file_checkfree($pc,$used,$total,$_FILES['pcfile']['size'])) {
	                unlink($_FILES['pcfile']['tmp_name']);
	                html_error_quit("对不起，您的文件夹空间不够");
	            }
	            if (!$c_dir->uf_add($_FILES['pcfile']['name'],$_FILES['pcfile']['tmp_name'],$_FILES['pcfile']['type'],$_POST['fileaccess'],$_POST['fileremark']))
	                html_error_quit($c_dir->err);
	        }
	        break;
	    case 'mkdir':
	        if (!pc_file_checkfree($pc,$used,$total,0))
	            html_error_quit("对不起，您的文件数目已达上限");
	        if (!$c_dir->uf_mkdir($_POST['dirname'], $_POST['diraccess'], $_POST['dirremark']))
	            html_error_quit($c_dir->err);
	        break;
	    case 'edit':
	        pc_file_editform($pc,$pid,$f);
	        pc_db_close($link);
	        html_normal_quit();
	        exit();
	        break;
	    case 'edit2':
	        if (!$f->uf_edit($_POST['newname'],$_POST['newaccess'],$_POST['newremark']))
	            html_error_quit($f->err);
	        break;
	    case 'rm' :
	        if (!$f->uf_rm())
	            html_error_quit($f->err);
	        break;
	    case 'cp':
	    case 'mv':
	        setcookie('PCFILEACT',$_GET['act']);
	        setcookie('PCFILEFID',$f->fid);
	        break;
	    case 'pt':
	        if ($_COOKIE['PCFILEACT']=='cp')
	            if (!($f->uf_cp($_GET['tid'])))
	                html_error_quit($f->err);
	        if ($_COOKIE['PCFILEACT']=='mv')
	            if (!($f->uf_mv($_GET['tid'])))
	                html_error_quit($f->err);
	        setcookie('PCFILEACT');
	        setcookie('PCFILEFID');
	        break;
	    case 'cl':
	        setcookie('PCFILEACT');
	        setcookie('PCFILEFID');
	        break;
	    default:
	}
	
	if ($_GET['act'] == 'add' || $_GET['act'] == 'mkdir')
	    pc_get_userfiles($link,$pc,$used,$total);
	pc_get_userfiles($link,$pc,$used,$total);
    pc_html_init("gb2312",stripslashes($pc["NAME"]));
?>
<br />
<p align="center"><b>
<?php echo BBS_FULL_NAME; ?>个人空间
</b></p>
<?php	
	pc_file_navigationbar($pc);
	pc_file_infor($pc,$used,$total);
	pc_file_showfiles($pc,$c_dir,$root_pid);
	pc_file_navigationbar($pc);
	pc_file_uploadform($pc,$pid,$c_dir);
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
}

?>
