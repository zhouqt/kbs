<?php

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");

global $boardArr;
global $boardID;
global $boardName;

setStat("管理封禁名单");

requireLoginok();

preprocess();

$processed = processAct();

show_nav($processed ? false : $boardName);

showUserMailBoxOrBR();
board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);

if ($processed) {
	html_success_quit("返回封禁名单", "bmdeny.php?board=" . $boardName);
} else {
	showDenys();
}

show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	if (!isset($_GET['board']))	{
		foundErr("未指定版面。");
	}
	$boardName=$_GET['board'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName,$brdArr);
	$boardArr=$brdArr;
	$boardName=$brdArr['NAME'];
	if ($boardID==0) {
		foundErr("指定的版面不存在");
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum,	$boardID) == 0)	{
		foundErr("您无权阅读本版");
	}
	if (!bbs_is_bm($boardID, $usernum))
		foundErr("你不是版主");
	return true;
}

/* return if we	did	any	process	*/
function processAct() {
	global $boardName;
	if (!isset($_GET['act'])) return false;
	switch ($_GET['act']) {
		case 'del':
			$userid = trim($_GET['userid']);
			if (!$userid)
				foundErr("请输入解封用户的ID");
			switch (bbs_denydel($boardName,$userid)) {
				case -1:
				case -2:
					foundErr("讨论区错误");
					break;
				case -3:
					foundErr($userid." 不在封禁列表中");
					break;
				default:
					setSucMsg("$userid 已被解封");
					return true;
			}
			break;
		case 'add':
			$userid = trim($_POST['userid']);
			$denyday = intval($_POST['denyday']);
			$exp = trim($_POST['exp']);
			if (!$userid)
				foundErr("请输入封禁用户的ID");
			if (!$denyday)
				foundErr("请输入封禁时间");
			if (!$exp)
				foundErr("请输入封禁理由");
			if (!strcasecmp($userid,'guest') || !strcasecmp($userid,'SYSOP'))
				foundErr("不能封禁 ".$userid);
			switch (bbs_denyadd($boardName,$userid,$exp,$denyday,0)) {
				case -1:
				case -2:
					foundErr("讨论区错误");
					break;
				case -3:
					foundErr("不正确的使用者ID");
					break;
				case -4:
					foundErr("用户 ".$userid." 已在封禁列表中");
					break;
				case -5:
					foundErr("封禁时间错误");
					break;
				case -6:
					foundErr("请输入封禁理由");
					break;
				default:
					setSucMsg("$userid 已被加入封禁名单");
					return true;
			}
			break;
		default:
	}
	return false;
}

function showDenys() {
	global $boardName;
	global $currentuser;
	$denyusers = array();
	$ret = bbs_denyusers($boardName,$denyusers);
	switch ($ret) {
	case -1:
		foundErr("系统错误，请联系管理员");
		break;
	case -2:
		foundErr("错误的讨论区");
		break;
	case -3:
		foundErr("您尚无权限");
		break;
	default:    
	}

	$maxdenydays = ($currentuser["userlevel"]&BBS_PERM_SYSOP)?70:14;
	$brd_encode = urlencode($boardName);
?>
<br>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<tr>
<th valign=middle width=30 height=25>序号</th>
<th valign=middle width=100>用户账号</th>
<th valign=middle width=*>理由</th>
<th valign=middle width=120>说明</th>
<th valign=middle width=40>解封</th>
</tr>
<?php
	$i = 1;
	foreach ($denyusers as $user) {
?>
<tr>
<td class="TableBody1" align="center"><?php echo $i++; ?></td>
<td class="TableBody1" align="center"><a href="dispuser.php?id=<?php echo $user['ID']; ?>"><?php echo $user['ID']; ?></a></td>
<td class="TableBody1"><?php echo htmlspecialchars($user['EXP']); ?></td>
<td class="TableBody1" align="center"><?php echo htmlspecialchars($user['COMMENT']); ?></td>
<td class="TableBody1" align="center"><a onclick="return confirm('确定解封<?php echo $user['ID']; ?>吗?')" href="bmdeny.php?board=<?php echo $brd_encode; ?>&act=del&userid=<?php echo $user['ID']; ?>">解封</a></td>
</tr>
<?php
	}
?>
<tr>
<td align=right valign=middle colspan=5 class=TableBody2>
本版共封禁了 <b><?php echo count($denyusers); ?></b> 位用户。
</td>
</tr>
</table><br />
<form action="bmdeny.php?act=add&board=<?php echo $brd_encode; ?>" method="post" name="adddeny">
<table align="center">
<tr>
	<td>添加封禁用户: 用户名<input type="text" name="userid" size="12" value="<?php
	    if (isset($_GET["userid"])) echo htmlspecialchars($_GET["userid"], ENT_QUOTES); ?>" maxlength="12" />，封禁时间
	<select name="denyday">
<?php
	$i = 1;
	while ($i <= $maxdenydays) {
		echo '<option value="'.$i.'">'.$i.'</option>';
		$i += ($i >= 14)?7:1;
	}    
?>    
	</select>天 <input type="submit" value="添加封禁" />
	</td>
</tr>
<tr>
	<td>封禁原因
<script language="JavaScript">
<!--
	function setReason(sReason) {
		if (sReason) {
			document.adddeny.exp.value = sReason;
			document.adddeny.exp.focus();
		}
	}
//-->
</script>
	<select name="explist" onchange="setReason(this.options[this.selectedIndex].value)">
<option value="" selected="selected">选择预定义封禁理由</option>
<?php
$denyreasons = array(
        '灌水',
        '干扰版面讨论秩序',
        '言语粗俗',
        '人身攻击',
        '开启无关话题',
        '发表不恰当文章',
        'test(供测试封禁使用)'
);
	foreach ($denyreasons as $reason) {
		echo '<option value="'.htmlspecialchars($reason).'">'.htmlspecialchars($reason).'</option>';
	}
?>    
	</select>
	<input type="text" name="exp" size="28" maxlength="28" />
	</td>
</tr>
</table></form>
<?php
}
?>
