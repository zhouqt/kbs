<?php
require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");
require("inc/ubbcode.php");

global $boardArr;
global $boardID;
global $boardName;
global $reID;
global $reArticles;

setStat("发表文章");

requireLoginok("游客不能发表文章。");

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
		$reID = 0;
	}
	settype($reID, "integer");
	$articles = array();
	if ($reID > 0)	{
	$num = bbs_get_records_from_id($boardName, $reID,$dir_modes["NORMAL"],$articles);
		if ($num == 0)	{
			foundErr("错误的 Re 文编号");
		}
		if ($articles[1]["FLAGS"][2] == 'y') {
			foundErr("该文不可回复!");
		}
		if ($articles[0]["FLAGS"][2] == 'y') {
			foundErr("该文不可回复!");
		}
	}
	$reArticles=$articles;

	return true;
}

function showPostArticles($boardID,$boardName,$boardArr,$reID,$reArticles){
	global $currentuser;
?>
<script src="inc/ubbcode.js"></script>
<form action="dopostarticle.php" method=POST onSubmit=submitonce(this) name=frmAnnounce>
<input type="hidden" value="<?php echo $boardName; ?>" name="board">
<input type="hidden" value="<?php echo $reID; ?>" name="reID">
<table cellpadding=3 cellspacing=1 class=TableBorder1 align=center>
    <tr>
      <th width=100% height=25 colspan=2 align=left>&nbsp;&nbsp;发表新帖子</th>
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
              <SELECT name=font onchange=DoTitle(this.options[this.selectedIndex].value)>
              <OPTION selected value="">选择话题</OPTION> <OPTION value=[原创]>[原创]</OPTION> 
              <OPTION value=[转帖]>[转帖]</OPTION> <OPTION value=[灌水]>[灌水]</OPTION> 
              <OPTION value=[讨论]>[讨论]</OPTION> <OPTION value=[求助]>[求助]</OPTION> 
              <OPTION value=[推荐]>[推荐]</OPTION> <OPTION value=[公告]>[公告]</OPTION> 
              <OPTION value=[注意]>[注意]</OPTION> <OPTION value=[贴图]>[贴图]</OPTION>
              <OPTION value=[建议]>[建议]</OPTION> <OPTION value=[下载]>[下载]</OPTION>
              <OPTION value=[分享]>[分享]</OPTION></SELECT>
          </td>
          <td width=80% class=TableBody2>
<?php
		if ($reID>0)	{
	        if(!strncmp($reArticles[1]["TITLE"],"Re: ",4)) $nowtitle = $reArticles[1]["TITLE"]." ";
	        else
	            $nowtitle = "Re: " . $reArticles[1]["TITLE"]." ";
		} else {
			$nowtitle='';
		}
?>
		  <input name=subject size=70 maxlength=80 value="<?php echo htmlspecialchars($nowtitle,ENT_QUOTES); ?>">&nbsp;&nbsp;<font color=#ff0000><strong>*</strong></font>不得超过 25 个汉字或50个英文字符
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
	if (bbs_is_attach_board($boardArr)) {
?>
<tr>
<td width=20% valign=middle class=TableBody2>
<b>文件上传</b>
</td><td width=80% class=TableBody2>
<iframe name="ad" frameborder=0 width=100% height=24 scrolling=no src="postupload.php?board=<?php echo $boardName; ?>"></iframe>
</td></tr>
<?php   } ?>
        <tr>
          <td width=20% valign=top class=TableBody1><b>内容</b></td>
          <td width=80% class=TableBody1>
<?php 
    if (ENABLE_UBB) require_once("inc/ubbmenu.php");
?>
<textarea class=smallarea cols=95 name=Content id="oArticleContent" rows=12 wrap=VIRTUAL title="可以使用Ctrl+Enter直接提交贴子" class=FormClass onkeydown=ctlent()>
<?php
    if (($reID > 0) && ($_GET['quote']==1 || OLD_REPLY_STYLE)){
    	$isquote = ($_GET['quote']==1) && ENABLE_UBB;
		$filename = $reArticles[1]["FILENAME"];
		$filename = "boards/" . $boardName. "/" . $filename;
		if(file_exists($filename))	{
			$fp = fopen($filename, "r");
			if ($fp) {
				if ($isquote) {
					echo "[quote][b]以下是引用[i]".$reArticles[1]['OWNER']."在".strftime("%Y-%m-%d %H:%M:%S",$reArticles[1]['POSTTIME'])."[/i]的发言：[/b]\n";
					$prefix = "";
				} else {
					echo "\n【 在 " . $reArticles[1]['OWNER'] . " 的大作中提到: 】\n";
					$prefix = ": ";
				}
				$buf2='';
				$buf = fgets($fp, 5000);
				if(strncmp($buf, "发信人", 6) == 0) {
					for ($i = 0; $i < 3; $i++) {
						if (($buf = fgets($fp,5000)) == FALSE)
							break;
					}
				}
				$q_prefix = str_repeat(": ", BBS_QUOTE_LEV);
				$q_len = 2 + 2 * BBS_QUOTE_LEV;
				while (1) {
					if (($buf = fgets($fp, 500)) == FALSE)
						break;
					if (strncmp($buf, $q_prefix . "【", $q_len) == 0)
						continue;
					if (strncmp($buf, $q_prefix . ": ", $q_len) == 0)
						continue;
					if (strpos($buf, "※ 来源") !== FALSE)
						break;
					if (strpos($buf, "※ 修改") !== FALSE)
						break;
					if (strncmp($buf, "--\n", 3) == 0)
						break;
					if (strncmp($buf, "\n", 1) == 0)
						continue;
					if (++$lines > BBS_QUOTED_LINES) {
						$buf2 .= ": ...................\n";
						break;
					}
					/* */
					//if (stristr($buf, "</textarea>") == FALSE)  //filter </textarea> tag in the text
						$buf2 .= $prefix . htmlspecialchars($buf);
				}
				echo reUBBCode($buf2);
				if ($isquote) echo "[/quote]\n";
				fclose($fp);
			}
		}
	}
?></textarea>
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
                <td valign=top class=TableBody1><b>选项</b></td>
                <td valign=middle class=TableBody1>&nbsp;<select name="signature">
<?php
		if ($currentuser["signature"] == 0)	{
?>
<option value="0" selected="selected">不使用签名档</option>
<?php
		}else{
?>
<option value="0">不使用签名档</option>
<?php
			for ($i = 1; $i <= $currentuser["signum"]; $i++) {
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
<option value="-1" <?php if ($currentuser["signature"] < 0) echo "selected "; ?>>随机签名档</option>
<?php
		}
?>
</select>
 [<a target="_balnk" href="bbssig.php">查看签名档</a>]<br>
 <?php
    if (bbs_is_outgo_board($boardArr)) {
        $local_save = 0;
        if ($reID > 0) $local_save = !strncmp($reArticles[1]["INNFLAG"], "LL", 2);
?>
<input type="checkbox" name="outgo" value="1"<?php if (!$local_save) echo " checked=\"checked\""; ?> />转信<br />
<?php
    }
		if (bbs_is_anony_board($boardArr) ) {
		echo "<input type=\"checkbox\" name=\"anonymous\" value=\"1\" />匿名<br />";
	}
	if ($reID == 0) {
?>
    <input type=checkbox name=emailflag value="1">有回复时使用邮件通知您？
<?php
	}
	if (SUPPORT_TEX) {
?>
    <input type=checkbox name=texflag value="1">使用 tex 发表<?php if (ENABLE_UBB) echo "（ubb 效果将被禁用）"; ?>
<?php
	}
?>
<BR><BR></td>
	</tr><tr>
	<td valign=middle colspan=2 align=center class=TableBody2>
	<input type=Submit value='发 表' name=Submit id="oSubmit"> &nbsp;&nbsp;&nbsp; <input type=button value='预 览' name=preview onclick=gopreview()>
                </td></form></tr>
      </table>
</form>
<form name=frmPreview action=preview.php?boardid=<?php echo $Boardid; ?> method=post target=preview_page>
<input type=hidden name=title value=><input type=hidden name=body value=><input type=hidden name=texflag value=>
</form>
<?php
}
?>
