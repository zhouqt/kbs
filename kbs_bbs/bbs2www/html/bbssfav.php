<?php
require_once ('funcs.php');
require_once ('favorite.inc.php');
login_init();

if ($loginok !=1 ) {
    html_nologin();
    exit ();
}

if (!strcmp ($currentuser['userid'],'guest')) {
    html_init('gb2312','','',1);
    html_error_quit('请先注册帐号');
}
if (strcmp ($currentuser['userid'], $_GET['userid'])) {
    html_init('gb2312','','',1);
    html_error_quit('参数错误');
}

if (!($fav = new Favorite ($currentuser['userid']))) {
    html_init('gb2312','','',1);
    html_error_quit ($fav);
}

if (cache_header("public, must-revalidate",filemtime($fav->file),10))
    return;

html_init('gb2312','','',1);
if (isset ($_GET['pid']))
    $pid = intval ($_GET['pid']);
else
    $pid = 0;

if (!$fav->load ($pid)) {
    html_error_quit ($fav->err);
}

$form = 0;
if (isset ($_GET['act'])) {
    switch ($_GET['act']) {
        case 'add':
            if (!$fav->add ($fav->currentPid, $_GET['type'], $_GET['order'], $_GET['title'], $_GET['url']))
                html_error_quit ($fav->err);
            break;
        case 'del':
            if (!$fav->del ($_GET['id']))
                html_error_quit ($fav->err);
            break;
        case 'edit':
            if (!$fav->id_exist ($_GET['id']))
                html_error_quit ('参数错误');
            $editArr = $fav->allFav[$_GET['id']];
            $form = 1;
            break;
        case 'edit2':
            if (!$fav->move ($_GET['id'], $_GET['order']))
                html_error_quit ($fav->err);
            break;
        case 'choose':
            $fav->load_all_dirs ();
            $form = 2;
            break;
        default:
    }
    $fav->load ($pid);
}

?>
<body topmargin="0">
<table width="100%" border="0" cellspacing="0" cellpadding="3" >
  <tr> 
    <td colspan="2" class="kb2">
	    <a class="kts1"  href="<?php echo MAINPAGE_FILE; ?>"><?php echo BBS_FULL_NAME; ?></a>  - <?php echo FAVORITE_NAME; ?></td>
  </tr>
   <tr valign=bottom align=center> 
    <td align="left" class="kb4">&nbsp;&nbsp;&nbsp;&nbsp; <?php echo FAVORITE_NAME; ?></td>
    </tr>
       <tr> 
    <td height="9" background="images/dashed.gif"> </td>
  </tr>
     <tr><td align="center">
<?php
display_super_fav ($fav);
switch ($form) {
    case 1:
        display_edit_form ($fav, $editArr);
        break;
    case 2:
        display_choose_form ($fav);
        break;    
    default:
        display_add_form ($fav);
}
unset ($fav);
?>
   <tr> 
    <td height="9" background="images/dashed.gif"> </td>
  </tr>
  </table>
<?php
html_normal_quit();
?>
<?php
function display_super_fav ($fav) {
    
?>
<center><table width="95%" border="0" cellspacing="0" cellpadding="3" class="kt1">	
<tbody><tr>
<td class="kt2" width="50">位置</td>
<td class="kt2" width="50">类型</td>
<td class="kt2" width="300">名称</td>
<td class="kt2">地址</td>
<td class="kt2" colspan="2">操作</td>
</tr></tbody>
<?php
    if ($fav->currentPid != 0) {
?>
<tbody><tr>
<td class="kt3 c2" height="25">&nbsp;</td>
<td class="kt3 c2"> <img src="images/groupgroup.gif" height="15" width="20" alt="up" title="回到根目录"></td>
<td class="kt3 c3" colspan="4" align="left"><a class="kts1"  href="<?php echo $_SERVER['PHP_SELF'].'?userid='.$fav->userId; ?>">回到根目录</a>
</td></tr></tbody>
<tbody><tr>
<td class="kt3 c2" height="25">&nbsp;</td>
<td class="kt3 c2"> <img src="images/groupgroup.gif" height="15" width="20" alt="up" title="回到上一级"></td>
<td class="kt3 c3" colspan="4" align="left"><a class="kts1"  href="<?php echo $_SERVER['PHP_SELF'].'?userid='.$fav->userId.'&pid='.$fav->info['PID']; ?>">回到上一级</a>
</td></tr></tbody>

<?php        
    }
    for ($i=0; $i < $fav->favCnt; $i ++) {
?>   
<tbody><tr>
<td class="kt3 c2" height="25"><?php echo $fav->fav[$i]['ORDER']; ?></td>
<td class="kt3 c2"> 
<?php
    if ($fav->is_dir ($fav->fav[$i]['ID'])) {
?>
<img src="images/groupgroup.gif" height="15" width="20"  alt="目录" title="目录">
<?php        
    }
    else {
?>
<img src="images/newgroup.gif"  height="15" width="20" alt="条目" title="条目">
<?php        
    }
?>
</td>
<?php
    if ($fav->is_dir ($fav->fav[$i]['ID'])) {
?>
<td class="kt3 c4" colspan="2" align="left"><a class="kts1" href="<?php echo $_SERVER['PHP_SELF'].'?userid='.$fav->userId.'&pid='.($fav->fav[$i]['ID']); ?>"><?php echo htmlspecialchars ($fav->fav[$i]['TITLE']); ?></a></td>
<?php
    }
    else {
?>
<td class="kt3 c4" align="left"><a class="kts1" href="<?php echo $fav->fav[$i]['URL']; ?>"><?php echo htmlspecialchars ($fav->fav[$i]['TITLE']); ?></a></td>
<td class="kt3 c4" align="left"><input type="text" value="<?php echo htmlspecialchars ($fav->fav[$i]['URL']); ?>" size="35" onmouseover="this.select()" /></td>
<?php
    }
?>
<td class="kt3 c2" width="50"><a href="<?php echo $_SERVER['PHP_SELF'].'?userid='.$fav->userId.'&pid='.$fav->currentPid.'&act=edit&id='.$fav->fav[$i]['ID']; ?>">修改</a></td>
<td class="kt3 c2" width="50"><a href="<?php echo $_SERVER['PHP_SELF'].'?userid='.$fav->userId.'&pid='.$fav->currentPid.'&act=del&id='.$fav->fav[$i]['ID']; ?>">删除</a></td>
</tr></tbody>
<?php        
    }
?>
</table></center>
<?php
    
}

function display_edit_form ($fav, $editArr) {
?>
<form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="get" >
<input type="hidden" name="userid" value="<?php echo $fav->userId; ?>" />
<input type="hidden" name="act" value="edit2" />
<input type="hidden" name="pid" value="<?php echo $fav->currentPid; ?>" />
<input type="hidden" name="id" value="<?php echo $editArr['ID']; ?>" />
<center><table width="60%" border="0" cellspacing="0" cellpadding="3" class="kt1">	
<tbody><tr><td class="kt2" colspan="2">编辑</td></tr></tbody>
<tbody><tr><td class="kt3 c2" width="120">名称</td><td class="kt3 c4"><?php echo htmlspecialchars ($editArr['TITLE']); ?></td></tr></tbody>
<tbody><tr><td class="kt3 c2" width="120">新位置</td><td class="kt3 c4">
<select name="order">
<?php
    for ($i=1; $i <= $fav->maxOrder; $i ++) {
        if ($editArr['ORDER']==$i)
            echo '<option value="'.$i.'" selected>'.$i.'</option>';
        else
            echo '<option value="'.$i.'">'.$i.'</option>';
    }
?>
</select>
</td></tr></tbody>
<tbody><tr><td class="kt3 c2" colspan="2"><input type="submit" value="修改" />&nbsp;<input type="button" onclick="window.location.href='<?php echo $_SERVER['PHP_SELF'].'?userid='.$fav->userId.'&pid='.$fav->currentPid; ?>';" value="添加" /></td></tr></tbody>
</table></center>
</form>
<?php
}

function display_add_form ($fav) {
?>
<form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="get" >
<input type="hidden" name="userid" value="<?php echo $fav->userId; ?>" />
<input type="hidden" name="act" value="add" />
<input type="hidden" name="pid" value="<?php echo $fav->currentPid; ?>" />
<center><table width="60%" border="0" cellspacing="0" cellpadding="3" class="kt1">	
<tbody><tr><td class="kt2" colspan="2">添加</td></tr></tbody>
<tbody><tr><td class="kt3 c2" width="120">名称</td><td class="kt3 c4"><input type="text" size="30" maxlength="100" name="title" onMouseOver="this.focus()" onFocus="this.select()" /></td></tr></tbody>
<tbody><tr><td class="kt3 c2" width="120">类型</td><td class="kt3 c4"><input type="radio" name="type" value="0" checked />条目&nbsp;&nbsp;<input type="radio" name="type" value="1" />目录</td></tr></tbody>
<tbody><tr><td class="kt3 c2" width="120">位置</td><td class="kt3 c4">
<select name="order">
<?php
    for ($i=1; $i <= $fav->maxOrder; $i ++) {
        echo '<option value="'.$i.'">'.$i.'</option>';
    }
    echo '<option value="'.($fav->maxOrder + 1).'" selected>'.($fav->maxOrder + 1).'</option>';
?>
</select>
</td></tr></tbody>
<tbody><tr><td class="kt3 c2" width="120">地址</td><td class="kt3 c4"><input type="text" size="30" maxlength="200" name="url" onMouseOver="this.focus()" onFocus="this.select()" /> (添加为条目时有效)</td></tr></tbody>
<tbody><tr><td class="kt3 c2" colspan="2"><input type="submit" value="添加" /></td></tr></tbody>
</table></center>
</form>
<?php    
}

function display_choose_form ($fav) {
?>
<form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="get" >
<input type="hidden" name="userid" value="<?php echo $fav->userId; ?>" />
<input type="hidden" name="act" value="add" />
<center><table width="60%" border="0" cellspacing="0" cellpadding="3" class="kt1">	
<tbody><tr><td class="kt2" colspan="2">添加</td></tr></tbody>
<tbody><tr><td class="kt3 c2" width="120">名称</td><td class="kt3 c4"><input type="text" size="30" maxlength="100" name="title" value="<?php echo $_GET['title']; ?>" onMouseOver="this.focus()" onFocus="this.select()" /></td></tr></tbody>
<tbody><tr><td class="kt3 c2" width="120">类型</td><td class="kt3 c4"><input type="radio" name="type" value="0" <?php if ($_GET['type']!=1) echo 'checked'; ?> />条目&nbsp;&nbsp;<input type="radio" name="type" value="1"  <?php if ($_GET['type']==1) echo 'checked'; ?> />目录</td></tr></tbody>
<tbody><tr><td class="kt3 c2" width="120">所在目录</td><td class="kt3 c4">
<select name="pid">
<option value="0" selected >根目录</option>
<?php
    reset ($fav->allDirs);
    while (list ($id, $val)=each ($fav->allDirs))
        echo '<option value="'.$id.'">'.htmlspecialchars($val['TITLE']).'</option>';
?>
</select>
</td></tr></tbody>
<tbody><tr><td class="kt3 c2" width="120">地址</td><td class="kt3 c4"><input type="text" size="30" maxlength="200" name="url" value="<?php echo $_GET['url']; ?>" onMouseOver="this.focus()" onFocus="this.select()" /> (添加为条目时有效)</td></tr></tbody>
<tbody><tr><td class="kt3 c2" colspan="2"><input type="submit" value="添加" /></td></tr></tbody>
</table></center>
</form>
<?php    
}
?>