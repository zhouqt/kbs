<?php
require("funcs.php");
login_init();
if ($loginok != 1) {
    html_nologin();
    exit;
}
html_init("gb2312","","",1);
?>
<body topmargin="0">
<?php

if (! bbs_can_send_mail() )
    html_error_quit("您不能发送信件");

$incept = trim(ltrim($_POST['userid']));
if (!$incept)
    html_error_quit("请输入收件人ID");
$lookupuser = array();
if (!bbs_getuser($incept,$lookupuser))
    html_error_quit("错误的收件人ID");
$incept = $lookupuser['userid'];

$title = preg_replace("/\\\(['|\"|\\\])/","$1",trim($_POST["title"]));
if (!$title) $title = '无主题';


$sig = intval($_POST['signature']); //签名档
$backup = isset($_POST['backup'])?strlen($_POST['backup']):0; //备份
$content = preg_replace("/\\\(['|\"|\\\])/","$1",$_POST["text"]); 

$ret = bbs_postmail($incept,$title,$content,$sig,$backup);

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
            html_error_quit($incept." 的信箱已满");
            break;
        case -6:
            html_error_quit("添加邮件列表出错");
            break;
        case -7:
?>
<script language="javascript">
<!--
alert('邮件发送成功，但未能保存到发件箱');
-->
</script>
<?php
            break;
        default:
            html_error_quit("系统错误，请联系管理员");
    }
}
?>
<br /><br /><br /><center>
信件发送成功
<br /><br /><br />
[<a href="/bbsmail.php">返回我的邮箱</a>]
[<a href="javascript:history.go(-2)">快速返回</a>]
</center>
<?php   
html_normal_quit();
?>