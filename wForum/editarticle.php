<?php


$needlogin=1;

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");

global $boardArr;
global $boardID;
global $boardName;
global $reID;
global $reArticles;

preprocess();

setStat("编辑文章");

show_nav();

if (isErrFounded()) {
	html_error_quit() ;
} else {
	?>
	<br>
	<TABLE cellSpacing=0 cellPadding=0 width=97% border=0 align=center>
	<?php

	if ($loginok==1) {
		showUserMailbox();
?>
</table>
<?php
	}

	board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);

	showPostArticles($boardID,$boardName,$boardArr,$reID,$reArticles);
}

//showBoardSampleIcons();
show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $loginok;
	global $reID;
	global $reArticles;
	if ($loginok!=1) {
		foundErr("游客不能发表文章。");
		return false;
	}
	if (!isset($_GET['board'])) {
		foundErr("未指定版面。");
		return false;
	}
	$boardName=$_GET['board'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName,$brdArr);
	$boardArr=$brdArr;
	if ($boardID==0) {
		foundErr("指定的版面不存在。");
		return false;
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版！");
		return false;
	}
	if (bbs_is_readonly_board($boardArr)) {
			foundErr("本版为只读讨论区！");
			return false;
	}
	if (bbs_checkpostperm($usernum, $boardID) == 0) {
		foundErr("您无权在本版发表文章！");
		return false;
	}
	if (isset($_GET["reID"])) {
		$reID = $_GET["reID"];
	}else {
		foundErr("未指定编辑的文章.");
		return false;
	}
	settype($reID, "integer");
	$articles = array();
	if ($reID > 0)	{
	$num = bbs_get_records_from_id($boardName, $reID,$dir_modes["NORMAL"],$articles);
		if ($num == 0)	{
			foundErr("错误的文章文编号");
			return false;
		}
	}
	$ret=bbs_caneditfile($boardName,$articles[1]['FILENAME']);
	switch ($ret) {
	case -1:
		foundErr("讨论区名称错误");
		return false;
	case -2:
		foundErr("本版不能修改文章");
		return false;
	case -3:
		foundErr("本版已被设置只读");
		return false;
	case -4:
		foundErr("无法取得文件记录");
		return false;
	case -5:
		foundErr("不能修改他人文章!");
		return false;
	case -6:
		foundErr("同名ID不能修改老ID的文章");
		return false;
	case -7:
		foundErr("您的POST权被封");
		return false;
	}
	$reArticles=$articles;
	return true;
}

function showPostArticles($boardID,$boardName,$boardArr,$reID,$reArticles){
	global $currentuser;
	global $_COOKIE;
?>
<script src="inc/ubbcode.js"></script>
<form action="doeditarticle.php" method=POST onSubmit=submitonce(this) name=frmAnnounce>
<input type="hidden" value="<?php echo $boardName; ?>" name="board">
<input type="hidden" value="<?php echo $reID; ?>" name="reID">
<table cellpadding=3 cellspacing=1 class=tableborder1 align=center>
    <tr>
      <th width=100% height=25 colspan=2 align=left>&nbsp;&nbsp;编辑帖子</th>
    </tr>
<!--
        <tr>
          <td width=20% class=tablebody2><b>用户名</b></td>
          <td width=80% class=tablebody2><input name=username value="<?php   echo $currentuser['userid']; ?>" class=FormClass>&nbsp;&nbsp;<font color=#ff0000><b>*</b></font><a href=register.php>您没有注册？</a> 
          </td>
        </tr>
        <tr>
          <td width=20% class=tablebody1><b>密&nbsp;&nbsp;码</b></td>
          <td width=80% class=tablebody1><input name=passwd type=password value=<?php   echo $_COOKIE['PASSWORD']; ?> class=FormClass><font color=#ff0000>&nbsp;&nbsp;<b>*</b></font><a href=lostpass.php>忘记密码？</a></td>
        </tr>
-->
        <tr>
          <td width=20% class=tablebody2><b>主题标题</b>
          </td>
          <td width=80% class=tablebody2>&nbsp;<?php echo htmlspecialchars($reArticles[1]["TITLE"],ENT_QUOTES); ?>&nbsp;&nbsp;
	 </td>
        </tr>
        <tr>
          <td width=20% valign=top class=tablebody1><b>当前心情</b><br><li>将放在帖子的前面</td>
          <td width=80% class=tablebody1>
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
        <tr>
          <td width=20% valign=top class=tablebody1>
<b>内容</b><br>
<li>HTML标签： <?php   echo $Board_Setting[5]?"可用":"不可用"; ?>
<li>UBB标签： <?php   echo $Board_Setting[6]?"可用":"不可用"; ?>
<li>贴图标签： <?php   echo $Board_Setting[7]?"可用":"不可用"; ?>
<li>多媒体标签：<?php   echo $Board_Setting[9]?"可用":"不可用"; ?>
<li>表情字符转换：<?php   echo $Board_Setting[8]?"可用":"不可用"; ?>
<li>上传图片：<?php   echo $Forum_Setting[3]?"可用":"不可用"; ?>
<li>最多<?php   echo intval($Board_Setting[16]/1024); ?>KB<BR><BR>
<B>特殊内容</B><BR>
<li><?php   echo $Board_Setting[10]?"<a href=\"javascript:money()\" title=\"使用语法：[money=可浏览该部分内容需要金钱数]内容[/money]\">金钱帖</a>":"金钱帖：不可用"; ?>
<li><?php   echo $Board_Setting[11]?"<a href=\"javascript:point()\" title=\"使用语法：[point=可浏览该部分内容需要积分数]内容[/point]\">积分帖</a>":"积分帖：不可用"; ?>
<li><?php   echo $Board_Setting[12]?"<a href=\"javascript:usercp()\" title=\"使用语法：[usercp=可浏览该部分内容需要魅力数]内容[/usercp]\">魅力帖</a>":"魅力帖：不可用"; ?>
<li><?php   echo $Board_Setting[13]?"<a href=\"javascript:power()\" title=\"使用语法：[power=可浏览该部分内容需要威望数]内容[/power]\">威望帖</a>":"威望帖：不可用"; ?>
<li><?php   echo $Board_Setting[14]?"<a href=\"javascript:article()\" title=\"使用语法：[post=可浏览该部分内容需要文章数]内容[/post]\">文章帖</a>":"文章帖：不可用"; ?>
<li><?php   echo $Board_Setting[15]?"<a href=\"javascript:replyview()\" title=\"使用语法：[replyview]该部分内容回复后可见[/replyview]\">回复帖</a>":"回复帖：不可用"; ?>
<li><?php   echo $Board_Setting[23]?"<a href=\"javascript:usemoney()\" title=\"使用语法：[usemoney=浏览该部分内容需要消耗的金钱数]内容[/usemoney]\">出售帖</a>":"出售帖：不可用"; ?>
	  </td>
          <td width=80% class=tablebody1>
<?php require_once("inc/ubbmenu.php"); ?>
<textarea class=smallarea cols=95 name=Content rows=12 wrap=VIRTUAL title="可以使用Ctrl+Enter直接提交贴子" class=FormClass onkeydown=ctlent()>
<?php
	bbs_printoriginfile($boardName,$reArticles[1]['FILENAME']);
?>
</textarea>
          </td>
        </tr>
		<tr>
                <td class=tablebody1 valign=top colspan=2 style="table-layout:fixed; word-break:break-all"><b>点击表情图即可在帖子中加入相应的表情</B><br>
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
<tr>
	<td valign=middle colspan=2 align=center class=tablebody2>
	<input type=Submit value='发 表' name=Submit> &nbsp; <input type=button value='预 览' name=Button onclick=gopreview() disabled>&nbsp;
<input type=reset name=Submit2 value='清 除'>
                </td></form></tr>
      </table>
</form>
<form name=preview action=preview.php?boardid=<?php echo $Boardid; ?> method=post target=preview_page>
<input type=hidden name=title value=><input type=hidden name=body value=>
</form>
<?php
}
?>
