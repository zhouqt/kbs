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

setStat("发表文章");

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
		$reID = 0;
	}
	settype($reID, "integer");
	$articles = array();
	if ($reID > 0)	{
	$num = bbs_get_records_from_id($boardName, $reID,$dir_modes["NORMAL"],$articles);
		if ($num == 0)	{
				foundErr("错误的 Re 文编号");
				return false;
		}
		if ($articles[1]["FLAGS"][2] == 'y') {
			foundErr("该文不可回复!");
			return false;
		}
	}
	$reArticles=$articles;

	return true;
}

function showPostArticles($boardID,$boardName,$boardArr,$reID,$reArticles){
	global $currentuser;
	global $_COOKIE;
?>
<script src="inc/ubbcode.js"></script>
<form action="dopostarticle.php" method=POST onSubmit=submitonce(this) name=frmAnnounce>
<input type="hidden" value="<?php echo $boardName; ?>" name="board">
<input type="hidden" value="<?php echo $reID; ?>" name="reID">
<table cellpadding=3 cellspacing=1 class=tableborder1 align=center>
    <tr>
      <th width=100% height=25 colspan=2 align=left>&nbsp;&nbsp;发表新帖子</th>
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
              <SELECT name=font onchange=DoTitle(this.options[this.selectedIndex].value)>
              <OPTION selected value="">选择话题</OPTION> <OPTION value=[原创]>[原创]</OPTION> 
              <OPTION value=[转帖]>[转帖]</OPTION> <OPTION value=[灌水]>[灌水]</OPTION> 
              <OPTION value=[讨论]>[讨论]</OPTION> <OPTION value=[求助]>[求助]</OPTION> 
              <OPTION value=[推荐]>[推荐]</OPTION> <OPTION value=[公告]>[公告]</OPTION> 
              <OPTION value=[注意]>[注意]</OPTION> <OPTION value=[贴图]>[贴图]</OPTION>
              <OPTION value=[建议]>[建议]</OPTION> <OPTION value=[下载]>[下载]</OPTION>
              <OPTION value=[分享]>[分享]</OPTION></SELECT>
          </td>
          <td width=80% class=tablebody2>
<?php
		if ($reID>0)	{
	        if(!strncmp($reArticles[1]["TITLE"],"Re: ",4)) $nowtitle = $reArticles[1]["TITLE"];
	        else
	            $nowtitle = "Re: " . $reArticles[1]["TITLE"];
		} else {
			$nowtitle='';
		}
?>
		  <input name=subject size=70 maxlength=80 value="<?php echo htmlspecialchars($nowtitle,ENT_QUOTES); ?>">&nbsp;&nbsp;<font color=#ff0000><strong>*</strong></font>不得超过 25 个汉字或50个英文字符
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
<?php   
	if (bbs_is_attach_board($boardArr)) {
?>
<tr>
<td width=20% valign=middle class=tablebody2>
<b>文件上传</b>
<!--
<select size=1>
<option value="">允许类型</option>
<?php 
/*
    $iupload=explode('|', $Board_Setting[19]);
	$len=count($iupload);
    for ($i=0; $i<$len; $i=$i+1)
    {

      print "<option value=".$iupload[$i].">".$iupload[$i]."</option>";

    } 
*/
?>
</select>
-->
</td><td width=80% class=tablebody2>
<iframe name="ad" frameborder=0 width=100% height=24 scrolling=no src="postupload.php?board=<?php echo $boardName; ?>"></iframe>
</td></tr>
<?php   } ?>
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
    if($reID > 0){
		$filename = $reArticles[1]["FILENAME"];
		$filename = "boards/" . $boardName. "/" . $filename;
		if(file_exists($filename))	{
			$fp = fopen($filename, "r");
			if ($fp) {
				$lines = 0;
				$buf = fgets($fp,256);       /* 取出第一行中 被引用文章的 作者信息 */
				$end = strrpos($buf,")");
				$start = strpos($buf,":");
				if($start != FALSE && $end != FALSE)
					$quser=substr($buf,$start+2,$end-$start-1);

				echo "\n【 在 " . $quser . " 的大作中提到: 】\n";
				for ($i = 0; $i < 3; $i++) {
					if (($buf = fgets($fp,500)) == FALSE)
						break;
				}
				while (1) {
					if (($buf = fgets($fp,500)) == FALSE)
						break;
					if (strncmp($buf, ": 【", 4) == 0)
						continue;
					if (strncmp($buf, ": : ", 4) == 0)
						continue;
					if (strncmp($buf, "--\n", 3) == 0)
						break;
					if (strncmp($buf,'\n',1) == 0)
						continue;
					if (++$lines > 10) {
						echo ": ...................\n";
						break;
					}
					/* */
					if (stristr($buf, "</textarea>") == FALSE)  //filter </textarea> tag in the text
						echo ": ". $buf;
				}
				fclose($fp);
			}
		}
	}
?></textarea>
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
		echo "<img src=\"emot/em".$ii."\" border=0 onclick=\"insertsmilie('[em".$ii."]')\" style=\"CURSOR: hand\">&nbsp;";
	} 
?>
    		</td>
                </tr>
                <tr>
                <td valign=top class=tablebody1><b>选项</b></td>
                <td valign=middle class=tablebody1>&nbsp;<select name="signature">
<?php
		if ($currentuser["signature"] == 0)	{
?>
<option value="0" selected="selected">不使用签名档</option>
<?php
		}else{
?>
<option value="0">不使用签名档</option>
<?php
		}
		for ($i = 1; $i <= bbs_getnumofsig(); $i++) {
			if ($currentuser["signature"] == $i) {
?>
<option value="<?php echo $i; ?>" selected="selected">第 <?php echo $i; ?> 个</option>
<?php
			}else{
?>
<option value="<?php echo $i; ?>">第 <?php echo $i; ?> 个</option>
<?php
			}
		}
?>
</select>
 [<a target="_balnk" href="bbssig.php">查看签名档</a>]<br>
 <?php
	if (bbs_is_outgo_board($boardArr) ) {
		echo "<input type=\"checkbox\" name=\"outgo\" value=\"1\" />转信<br />";
	}
		if (bbs_is_anony_board($boardArr) ) {
		echo "<input type=\"checkbox\" name=\"annonymous\" value=\"1\" />匿名<br />";
	}
?>
    <input type=checkbox name=emailflag value=yes disabled>有回复时使用邮件通知您？</font>
<BR><BR></td>
	</tr><tr>
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