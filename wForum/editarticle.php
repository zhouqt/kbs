<?php

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");

global $boardArr;
global $boardID;
global $boardName;
global $reID;
global $reArticles;

setStat("编辑文章");

requireLoginok("游客不能编辑文章。");

preprocess();

show_nav();

showUserMailBoxOrBR();
board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);
showPostArticles($boardID,$boardName,$boardArr,$reID,$reArticles);

show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $loginok;
	global $reID;
	global $reArticles;
	global $dir_modes;
	if (!isset($_GET['board'])) {
		foundErr("未指定版面。");
	}
	$boardName=$_GET['board'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName,$brdArr);
	$boardArr=$brdArr;
	$boardName=$brdArr['NAME'];
	if ($boardID==0) {
		foundErr("指定的版面不存在。");
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版！");
	}
	if (bbs_is_readonly_board($boardArr)) {
		foundErr("本版为只读讨论区！");
	}
	if (bbs_checkpostperm($usernum, $boardID) == 0) {
		foundErr("您无权在本版发表文章！");
	}
	if (isset($_GET["reID"])) {
		$reID = $_GET["reID"];
	}else {
		foundErr("未指定编辑的文章.");
	}
	settype($reID, "integer");
	$articles = array();
	if ($reID > 0)	{
	$num = bbs_get_records_from_id($boardName, $reID,$dir_modes["NORMAL"],$articles);
		if ($num == 0)	{
			foundErr("错误的文章文编号");
		}
	}
	$ret=bbs_caneditfile($boardName,$articles[1]['FILENAME']);
	switch ($ret) {
	case -1:
		foundErr("讨论区名称错误");
	case -2:
		foundErr("本版不能修改文章");
	case -3:
		foundErr("本版已被设置只读");
	case -4:
		foundErr("无法取得文件记录");
	case -5:
		foundErr("不能修改他人文章!");
	case -6:
		foundErr("同名ID不能修改老ID的文章");
	case -7:
		foundErr("您的POST权被封");
	}
	$reArticles=$articles;
	return true;
}

function showPostArticles($boardID,$boardName,$boardArr,$reID,$reArticles){
	global $currentuser;
?>
<script src="inc/ubbcode.js"></script>
<form action="doeditarticle.php" method=POST onSubmit=submitonce(this) name=frmAnnounce>
<input type="hidden" value="<?php echo $boardName; ?>" name="board">
<input type="hidden" value="<?php echo $reID; ?>" name="reID">
<table cellpadding=3 cellspacing=1 class=TableBorder1 align=center>
    <tr>
      <th width=100% height=25 colspan=2 align=left>&nbsp;&nbsp;编辑帖子</th>
    </tr>
<!--
        <tr>
          <td width=20% class=TableBody2><b>用户名</b></td>
          <td width=80% class=TableBody2><input name=username value="<?php   echo $currentuser['userid']; ?>" class=FormClass>&nbsp;&nbsp;<font color=#ff0000><b>*</b></font><a href=register.php>您没有注册？</a> 
          </td>
        </tr>
        <tr>
          <td width=20% class=TableBody1><b>密&nbsp;&nbsp;码</b></td>
          <td width=80% class=TableBody1><input name=passwd type=password value=<?php   echo $_COOKIE['PASSWORD']; ?> class=FormClass><font color=#ff0000>&nbsp;&nbsp;<b>*</b></font><a href=lostpass.php>忘记密码？</a></td>
        </tr>
-->
        <tr>
          <td width=20% class=TableBody2><b>主题标题</b>
          </td>
          <td width=80% class=TableBody2>&nbsp;<?php echo htmlspecialchars($reArticles[1]["TITLE"],ENT_QUOTES); ?>&nbsp;&nbsp;
          <input name=subject type=hidden value="<?php echo htmlspecialchars($reArticles[1]["TITLE"],ENT_QUOTES); ?>">
	 </td>
        </tr>
<?php
		/* disabled by atppp
?>
        <tr>
          <td width=20% valign=top class=TableBody1><b>当前心情</b><br><li>将放在帖子的前面</td>
          <td width=80% class=TableBody1>
<?php
	for ($i=0; $i<=18; $i++) {
?>
	<input type="radio" value="<?php     echo $i ?>" name="Expression" 
<?php    
		if ($i==0) {
			echo "checked";    
		} 
?>><img src="face/face<?php echo  $i; ?>.gif" >&nbsp;&nbsp;
<?php 
		if ($i>0 && (($i+1)% 9==0)) {
			echo "<br>";    
		} 
	} 
?>
 </td>
        </tr>
<?php
     (心情符号，disabled) */
?>
        <tr>
          <td width=20% valign=top class=TableBody1>
<b>内容</b>
	  </td>
          <td width=80% class=TableBody1>
<?php if (ENABLE_UBB) require_once("inc/ubbmenu.php"); ?>
<textarea class=smallarea cols=95 name=Content rows=12 wrap=VIRTUAL title="可以使用Ctrl+Enter直接提交贴子" class=FormClass onkeydown=ctlent()>
<?php
	bbs_printoriginfile($boardName,$reArticles[1]['FILENAME']); //ToDo: 这个地方过滤 </textarea> 且没有 html 化，待改
?>
</textarea>
          </td>
        </tr>
<?php
        if (ENABLE_UBB) {
?>
		<tr>
                <td class=TableBody1 valign=top colspan=2 style="table-layout:fixed; word-break:break-all"><b>点击表情图即可在帖子中加入相应的表情</B><br>
<?php 
	for ($i=1; $i<=69; $i++) {
		if (strlen($i)==1)   {
			$ii="0".$i;
		} else  {
			$ii=$i;
		} 
		if ($i!=1 && (($i-1)%20)==0) {
			echo "<br>\n";
		}
		echo "<img src=\"emot/em".$ii.".gif\" border=0 onclick=\"insertsmilie('[em".$ii."]')\" style=\"CURSOR: hand\">&nbsp;";
	} 
?>
    		</td>
                </tr>
<?php
        }
?>
<tr>
	<td valign=middle colspan=2 align=center class=TableBody2>
	<input type=Submit value='发 表' name=Submit> &nbsp;&nbsp;&nbsp; <input type=button value='预 览 (附件无效)' name=preview onclick=gopreview()>
                </td></form></tr>
      </table>
</form>
<form name=frmPreview action=preview.php?boardid=<?php echo $Boardid; ?> method=post target=preview_page>
<input type=hidden name=title value=><input type=hidden name=body value=><input type=hidden name=texflag value=<?php echo (SUPPORT_TEX && $reArticles[1]["IS_TEX"])?1:0; ?>>
</form>
<?php
}
?>
