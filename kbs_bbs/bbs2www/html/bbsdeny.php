<?php
$denyreasons = array(
	'灌水',
	'干扰版面讨论秩序',
	'言语粗俗',
	'人身攻击',
	'开启无关话题',
	'发表不恰当文章',
	'test(在征得对方同意后供测试封禁使用)'
);
 
require("www2-funcs.php");
require("www2-board.php");
login_init();

if (isset($_GET["board"]))
	$board = $_GET["board"];
else
	html_error_quit("错误的讨论区");

$brdarr = array();
$bid = bbs_getboard($board, $brdarr);
if ($bid == 0)
	html_error_quit("错误的讨论区");
$usernum = $currentuser["index"];
if (!bbs_is_bm($bid, $usernum))
	html_error_quit("你不是版主");
$board = $brdarr['NAME'];
$brd_encode = urlencode($board);

bbs_board_nav_header($brdarr, "封禁名单");

if (isset($_GET['act'])) {
	switch ($_GET['act']) {
		case 'del':
			$userid = ltrim(trim($_GET['userid']));
			if (!$userid)
				html_error_quit("请输入解封用户的ID");
			switch (bbs_denydel($board,$userid)) {
				case -1:
				case -2:
					html_error_quit("讨论区错误");
					break;
				case -3:
					html_error_quit($userid." 不在封禁列表中");
					break;
				default:
			}
			break;
		case 'add':
			$userid = ltrim(trim($_POST['userid']));
			$denyday = intval($_POST['denyday']);
			$exp = (trim($_POST['exp2']))?trim($_POST['exp2']):$denyreasons[intval($_POST['exp'])];
			if (!$userid || !$denyday || !$exp)
				break;
			if (!strcasecmp($userid,'guest') || !strcasecmp($userid,'SYSOP'))
				html_error_quit("不能封禁 ".$userid);
			switch (bbs_denyadd($board,$userid,$exp,$denyday,0)) {
				case -1:
				case -2:
					html_error_quit("讨论区错误");
					break;
				case -3:
					html_error_quit("不正确的使用者ID");
					break;
				case -4:
					html_error_quit("用户 ".$userid." 已在封禁列表中");
					break;
				case -5:
					html_error_quit("封禁时间错误");
					break;
				case -6:
					html_error_quit("请输入封禁理由");
					break;
				default:
			}
			break;
		default:
	}
}

$denyusers = array();
$ret = bbs_denyusers($board,$denyusers);
switch ($ret) {
	case -1:
		html_error_quit("系统错误，请联系管理员");
		break;
	case -2:
		html_error_quit("错误的讨论区");
		break;
	case -3:
		html_error_quit("您尚无权限");
		break;
	default:    
}

$maxdenydays = ($currentuser["userlevel"]&BBS_PERM_SYSOP)?70:14;
?>
<table class="main wide adj">
<caption>封禁名单</caption>
<col class="center" width="60"/><col class="center" width="100"/><col width="*"/><col class="center" width="150"/><col class="center" width="60"/>
<tbody><tr><th>序号</th><th>用户名</th><th>理由</th><th>说明</th><th>解封</th></tr>
<?php
	$i = 1;
	foreach ($denyusers as $user) {
		echo '<tr><td>'.$i.'</td><td><a href="bbsqry.php?userid='.$user['ID'].'">'.$user['ID'].'</a></td>'.
			 '<td>'.htmlspecialchars($user['EXP']).' </td>'.
			 '<td>'.htmlspecialchars($user['COMMENT']).'</td>'.
			 '<td><a onclick="return confirm(\'确实解封吗?\')" href="'.$_SERVER['PHP_SELF'].'?board='.$brd_encode.'&act=del&userid='.$user['ID'].'">解封</a></td>'.
			 '</tr>';
		$i ++ ;
	}
?>
</tbody></table>
<form action="<?php $_SERVER['PHP_SELF']; ?>?act=add&board=<?php echo $brd_encode; ?>" method="post" class="medium">
	<fieldset><legend>添加封禁用户</legend>
		<div class="inputs">
			<label>用户名</label><input type="text" name="userid" size="12" maxlength="12" /><br/>
			<label>封禁时间</label><select name="denyday">
<?php
	$i = 1;
	while ($i <= $maxdenydays) {
		echo '<option value="'.$i.'">'.$i.'</option>';
		$i += ($i >= 14)?7:1;
	}    
?>    
			</select>天<br/>
			<label>封禁原因</label><select name="exp">
<?php
	$i = 0;
	foreach ($denyreasons as $reason) {
		echo '<option value="'.$i.'">'.htmlspecialchars($reason).'</option>';
		$i ++;
	}
?>    
			</select><br />
			或手动输入封禁理由：
			<input type="text" name="exp2" size="20" maxlength="28" />
		</div>
		<div class="oper"><input type="submit" value="添加封禁" /></div>
	</fieldset>
</form>
<?php
page_footer(FALSE);
?>