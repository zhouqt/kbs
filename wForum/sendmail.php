<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

require("inc/board.inc.php");

$action=0;
$article;

setStat("撰写新邮件");

show_nav();

echo "<br><br>";

preprocess();

if (!isErrFounded()) {
	head_var($userid."的发件箱","usermailbox.php?boxname=sendbox",0);
}

if ($loginok==1) {
	showUserManageMenu();
	showmailBoxes();
	main();
}else {
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
}


if (isErrFounded()) {
		html_error_quit();
} 
show_footer();

function preprocess() {
	global $action;
    if (!bbs_can_send_mail()) {
		foundErr("您没有写信权力!");
		return false;
	}
	if (isset($_GET['boxname'])) {
		setstat("回复信件");
		$action=1;
		$num=intval($_GET['num']);
		if ($_GET['boxname']=='inbox') {
			return getmail('inbox','.DIR','收件箱', $num);
		}
		if ($_GET['boxname']=='sendbox') {
			return getmail('sendbox','.SENT','发件箱',$num );
		}
		if ($_GET['boxname']=='deleted') {
			return getmail('deleted','.DELETED','垃圾箱',$num);
		}
	}
	if (isset($_GET['board'])) {
		setstat("发信给作者");
		$action=2;
		$reID=intval($_GET['reID']);
		return getarticle($_GET['board'],$reID);
	}
}

function getarticle($boardName,$reID){
	global $article;
	global $currentuser;
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
	settype($reID, "integer");
	$articles = array();
	if ($reID > 0)	{
	$num = bbs_get_records_from_id($brdArr['NAME'], $reID,$dir_modes["NORMAL"],$articles);
		if ($num == 0)	{
			foundErr("错误的文章文编号");
			return false;
		}
	}
	$article=$articles[1];
	return true;
}


function getmail($boxName, $boxPath, $boxDesc, $num){
	global $article;
	global $currentuser;
	$dir = bbs_setmailfile($currentuser["userid"],$boxPath);

	$total = filesize( $dir ) / 256 ;
	if( $total <= 0 ){
		foundErr("您所指定的信件不存在");
		return false;
	}
	$articles = array ();
	if( bbs_get_records_from_num($dir, $num, $articles) ) {
		$file = $articles[0]["FILENAME"];
	}else{
		foundErr("您所指定的信件不存在");
		return false;
	}

	$filename = bbs_setmailfile($currentuser["userid"],$file) ;

	if(! file_exists($filename)){
		foundErr("您所指定的信件不存在");
		return false;
	}
	$article=$articles[0];

	return true;
}

function main() {
	global $currentuser;
	global $_GET;
	global $article;
	global $action;
?>
<br>
<form action="dosendmail.php" method=post name=messager onkeydown="if(event.keyCode==13 && event.ctrlKey)messager.submit()">
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
          <tr> 
            <th colspan=3><?php echo $action==0?"撰写新邮件":"回复邮件"; ?></td>
          </tr>
          <tr> 
            <td class=TableBody1 valign=middle><b>收件人:</b></td>
            <td class=TableBody1 valign=middle>
              <input name="destid" maxlength="12" value="<?php if ($action!=0) 
			echo $article['OWNER'].'" size="12" readonly />'; 
					else { ?>" size="12" />			 
              <SELECT name=font onchange=DoTitle(this.options[this.selectedIndex].value)>
              <OPTION selected value="">选择</OPTION>
			  </SELECT><?php } ?>
            </td>
          </tr>
           <tr> 
            <td class=TableBody1 valign=top width=15%><b>标题：</b></td>
            <td  class=TableBody1 valign=middle>
<?php
		if ($action!=0)	{
	        if(!strncmp($article["TITLE"],"Re: ",4)) $nowtitle = $article["TITLE"];
	        else
	            $nowtitle = "Re: " . $article["TITLE"];
		} else {
			$nowtitle='';
		}
?>
              <input name="title" maxlength="50" size="50" value="<?php echo htmlspecialchars($nowtitle,ENT_QUOTES); ?>"/>
            </td>
          </tr>
           <tr> 
            <td class=TableBody1 valign=top width=15%><b>内容：</b></td>
            <td  class=TableBody1 valign=middle>
              <textarea style="width:500;height:300" name="content"><?php
    if($action!=0){
		if ($action==2){
    		$filename = "boards/" . $_GET['board']. "/" . $article['FILENAME'];
            echo "\n【 在 " . $article['OWNER'] . " 的大作中提到: 】\n";
		}else{
			$filename = bbs_setmailfile($currentuser["userid"],$article['FILENAME']) ;
            echo "\n【 在 " . $article['OWNER'] . " 的来信中提到: 】\n";
		}
		if(file_exists($filename))
		{
		    $fp = fopen($filename, "r");
	        if ($fp) {
				$buf = fgets($fp,500);
				if(strncmp($buf, "发信人", 6) == 0) {
					for ($i = 0; $i < 4; $i++) {
						if (($buf = fgets($fp,500)) == FALSE)
							break;
					}
				}
				while (1) {
					if (strncmp($buf, ": 【", 4) == 0)
						continue;
					if (strncmp($buf, ": : ", 4) == 0)
						continue;
					if (strpos($buf, "※ 来源") !== FALSE)
						break;
					if (strpos($buf, "※ 修改") !== FALSE)
						break;
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
					if (($buf = fgets($fp,500)) == FALSE)
						break;
				}
				fclose($fp);
	        }
	    }
	}
?></textarea>
            </td>
          </tr>
		 <tr>
                <td valign=top class=TableBody1><b>选项：</b></td>
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
		}
		for ($i = 1; $i <= bbs_getnumofsig(); $i++) {
			if ($currentuser["signature"] == $i) {
?>
<option value="<?php echo $i; ?>" selected="selected">第 <?php echo $i; ?> 个</option>
<?php
			} else {
?>
<option value="<?php echo $i; ?>">第 <?php echo $i; ?> 个</option>
<?php
			}
		}
?>
</select>
 [<a target="_balnk" href="bbssig.php">查看签名档</a>]<br>
 <input type=checkbox name=backup>备份到发件箱中
			</td>
		  </tr>
          <tr> 
            <td  class=TableBody1 colspan=2>
<b>说明</b>：<br>
① 您可以使用<b>Ctrl+Enter</b>键快捷发送短信<br>
<!--
② 可以用英文状态下的逗号将用户名隔开实现群发，最多<b>5</b>个用户<br>

③ 标题最多<b>50</b>个字符，内容最多<b>300</b>个字符<br>
-->
            </td>
          </tr>

          <tr> 
            <td  class=TableBody2 valign=middle colspan=2 align=center> 
			  
			  &nbsp;
              <input type=Submit value="发送信件" name=Submit>
            </td>
          </tr>

        </table>
</form>
<?php

}
?>
