<?php
require("www2-funcs.php");
login_init();
assert_login();
mailbox_header("发送信件");

$mailfile = @$_POST["file"];
$dirfile = @$_POST["dir"];
if (strstr($dirfile,'..')) die;
$maildir = bbs_setmailfile($currentuser["userid"], $dirfile);
$num = @intval($_POST["num"]);

if ( !bbs_can_send_mail($mailfile ? 1 : 0) )
	html_error_quit("您不能发送信件");

if($mailfile == "")		// if not reply
{
	$incept = trim(@$_POST['userid']);
	if (!$incept)
		html_error_quit("请输入收件人ID");
	$lookupuser = array();
	if (!bbs_getuser($incept,$lookupuser))
		html_error_quit("错误的收件人ID");
	$incept = $lookupuser['userid'];
	
	if (!strcasecmp($incept,'guest'))
		html_error_quit("不能发信给guest");
}

$title = trim(@$_POST["title"]);
if (!$title) $title = '无主题';

$sig = intval(@$_POST['signature']); //签名档
$backup = 0;
if (isset($_POST['backup'])) $backup = intval($_POST['backup']);

if($mailfile == "")
{
	$ret = bbs_postmail($incept,$title,@$_POST["text"],$sig,$backup);
}
else
{
	$ret = bbs_postmail($maildir, $mailfile, $num, $title, @$_POST["text"], $sig, $backup);
}

if ($ret < 0)  {
	switch($ret) {
		case -1:
		case -2:
			html_error_quit("无法创建文件");
			break;
		case -3:
			html_error_quit($incept." 拒收您的邮件");
			break;
		case -4:
			html_error_quit($incept." 的信箱已满或者不能收信");
			break;
		case -5:
			html_error_quit("两次发文/信间隔过密,请休息几秒再试!");	
			break;
		case -6:
			html_error_quit("添加邮件列表出错");
			break;
		case -7:
			html_error_quit("邮件发送成功，但未能保存到发件箱");
			break;
		case -8:
			html_error_quit("找不到所回复的原信。");
			break;
		case -9:
			html_error_quit("您没有发信的权限。");
			break;		
		case -100:
			html_error_quit("错误的收件人ID");
			break;
		default:
			html_error_quit("系统错误，请联系管理员");
	}
}
html_success_quit("信件发送成功<br/>[<a href=\"bbsmail.php\">返回我的邮箱</a>]");
?>