<?php
/**
 * 版面封禁
 * windinsn.04.05.17
 */
$denyreasons = array(
        '灌水',
        '干扰版面讨论秩序',
        '言语粗俗',
        '人身攻击',
        '开启无关话题',
        '发表不恰当文章',
        'test(在征得对方同意后供测试封禁使用)'
                );
 
require("funcs.php");
require("board.inc.php");
login_init();
if ($loginok != 1)
	html_nologin();

html_init("gb2312","","",1);

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
            switch (bbs_denyadd($board,$userid,$exp,$denyday,1)) {
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

bbs_board_header($brdarr);
?>
<center><br/>
<p>封禁名单</p>
<table cellspacing="0" cellpadding="3" border="0" width="90%" class="t1">
    <tbody><tr>
        <td width="40" class="t2">序号</td>
        <td width="100" class="t2">用户名</td>
        <td class="t2">理由</td>
        <td width="120" class="t2">说明</td>
        <td width="40" class="t2">解封</td>
    </tr></tbody>
<?php
    $i = 1;
    foreach ($denyusers as $user) {
        echo '<tbody><tr><td class="t3">'.$i.'</td><td class="t4"><a href="/bbsqry.php?userid='.$user['ID'].'">'.$user['ID'].'</a></td>'.
             '<td class="t7">'.$user['EXP'].' </td>'.
             '<td class="t5">'.htmlspecialchars($user['COMMENT']).'</td>'.
             '<td class="t4"><a onclick="return confirm(\'确实解封吗?\')" href="'.$_SERVER['PHP_SELF'].'?board='.$brd_encode.'&act=del&userid='.$user['ID'].'">解封</a></td>'.
             '</tr></tbody>';
        $i ++ ;
    }
?>
</table><br />
<form action="<?php $_SERVER['PHP_SELF']; ?>?act=add&board=<?php echo $brd_encode; ?>" method="post">
<table cellspacing="0" cellpadding="3" border="0" width="60%" class="t1">
<tr>
    <td colspan="2" class="t2">添加封禁用户</td>
</tr>
<tr>
    <td width="100" class="t3">用户名</td>
    <td class="t7"><input type="text" name="userid" size="12" maxlength="12" /></td>
</tr>
<tr>
    <td width="100" class="t3">封禁时间</td>
    <td class="t7">
    <select name="denyday">
<?php
    $i = 1;
    while ($i <= $maxdenydays) {
        echo '<option value="'.$i.'">'.$i.'</option>';
        $i += ($i >= 14)?7:1;
    }    
?>    
    </select>
    天
    </td>
</tr>
<tr>
    <td width="100" class="t3">封禁原因</td>
    <td class="t7">
    <select name="exp">
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
    </td>
</tr>
<tr>
    <td colspan="2" class="t3">
    <input type="submit" value="添加封禁" />
    </td>
</tr>
</table></form>
</center>
<?php
bbs_board_foot($brdarr,'DENYLIST');
html_normal_quit();
?>