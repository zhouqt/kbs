<?php
/* 修改个人参数，从 wForum 偷来的。atppp 20040524 */
require("www2-funcs.php");
login_init();
toolbox_header("个人参数修改");
assert_login();

define('SHOWTELNETPARAM', 0); //是否显示 telnet 选项

/* 格式：每项五个参数，第一个 0 表示 telnet 下专用参数；第二个是参数名称；第三个是参数具体解释；第四五是参数 ON 和 OFF 所代表的具体含义 */
$user_define=array(array(0,"显示活动看版", "是否显示活动看板","显示","不显示"), /* DEF_ACBOARD */
	array(0,"使用彩色", "是否使用彩色显示", "使用", "不使用"),                /* DEF_COLOR */
	array(0, "编辑时显示状态栏","编辑文章时是否显示状态栏", "显示","不显示"),         /* DEF_EDITMSG */
	array(0,"分类讨论区以 New 显示", "是否以未读方式阅读分类讨论区", "是", "否"),    /* DEF_NEWPOST */
	array(0,"选单的讯息栏","是否显示选单讯息栏","显示","不显示"),             /* DEF_ENDLINE */
	array(0,"上站时显示好友名单","上站时是否显示好友在线名单","显示","不显示"),       /* DEF_LOGFRIEND */
	array(0,"让好友呼叫","当呼叫器关闭时是否允许好友呼叫","是", "否"),               /* DEF_FRIENDCALL */
	array(0, "使用自己的离站画面", "是否使用自己的离站画面","是", "否"),      /* DEF_LOGOUT */
	array(0, "进站时显示备忘录", "进站时是否显示备忘录", "是", "否"),        /* DEF_INNOTE */
	array(0, "离站时显示备忘录", "离站时是否显示备忘录", "是", "否"),        /* DEF_OUTNOTE */
	array(0, "讯息栏模式", "讯息栏的显示内容",  "呼叫器状态", "在线人数"), /* DEF_NOTMSGFRIEND */
	array(0, "菜单模式选择", "telnet的菜单模式", "缺省模式", "精简模式"), /* DEF_NORMALSCR */
	array(0, "阅读文章是否使用绕卷选择", "阅读文章是否绕卷选择", "是","否"),/* DEF_CIRCLE */
	array(0, "阅读文章游标停於第一篇未读","文章列表时光标自动定位的位置", "第一篇未读文章", "最新一篇文章"),       /* DEF_FIRSTNEW */
	array(0, "屏幕标题色彩", "屏幕标题色彩显示模式", "标准", "自动变换"), /* DEF_TITLECOLOR */
	array(1, "接受所有人的讯息", "是否允许所有人给您发短消息","是","否"),         /* DEF_ALLMSG */
	array(1, "接受好友的讯息", "是否允许好友给您发短消息", "是", "否"),          /* DEF_FRIENDMSG */
	array(0, "收到讯息发出声音","收到短信后是否以声音提醒您","是","否"),         /* DEF_SOUNDMSG */
	array(0, "离站後寄回所有讯息","退出登录时是否把短信息寄回信箱", "是", "否"),       /* DEF_MAILMSG */
	array(0, "发文章时实时显示讯息","编辑文章时是否实时显示短消息","是", "否"),     /*"所有好友上站均通知",    DEF_LOGININFORM */
	array(0,"菜单上显示帮助信息","是否在菜单上显示帮助信息", "是", "否"),       /* DEF_SHOWSCREEN */
	array(0, "进站时显示十大新闻","进站时是否显示十大热门话题", "显示", "不显示"),       /* DEF_SHOWHOT */
	array(0, "进站时观看留言版","进站时是否显示留言板","显示","不显示"),         /* DEF_NOTEPAD */
	array(0, "忽略讯息功能键", "用哪个键忽略短信", "Enter键","Esc键"),       /* DEF_IGNOREMSG */
	array(0, "使用高亮界面","是否使用高亮界面", "使用", "不使用"),         /* DEF_HIGHCOLOR */
	array(0, "进站时观看上站人数统计图", "进站时是否显示上站人数统计图", "显示", "不显示"), /* DEF_SHOWSTATISTIC Haohmaru 98.09.24 */
	array(0, "未读标记字符","用哪个字符作为未读标记", "*","N"),           /* DEF_UNREADMARK Luzi 99.01.12 */
	array(0, "使用GB码阅读","用GB码阅读", "是", "否"),             /* DEF_USEGB KCN 99.09.03 */
	array(0, "对汉字进行整字处理", "是否对汉字进行整字处理","是", "否"),  /* DEF_CHCHAR 2002.9.1 */
	array(0, "显示详细用户信息(wForum)", "是否允许他人看到您的用户资料", "允许", "不允许"),  /*DEF_SHOWDETAILUSERDATA 2003.7.31 */
	array(0, "显示真实用户信息(wForum)",  "是否允许他人看到您的真实资料", "允许", "不允许") /*DEF_REALDETAILUSERDATA 2003.7.31 */
);

$user_define1=array(array(1,"隐藏 IP", "是否隐藏自己的 IP 信息","隐藏","不隐藏") /* DEF_HIDEIP */
);

$mailbox_prop=array(array(1,"发信时保存信件到发件箱", "是否发信时自动选择保存到发件箱","保存","不保存"),
array(1,"删除信件时不保存到垃圾箱", "是否删除信件时不保存到垃圾箱","不保存","保存"),
array(0,"快捷邮箱键", "版面按 'v' 时进入什么界面","信箱主界面","收件箱")
);



if (isset($_GET['do'])) {
	$userdefine0 = getOptions("user_define", $currentuser['userdefine0']);
	$userdefine1 = getOptions("user_define1", $currentuser['userdefine1']);
	$mailbox_prop = getOptions("mailbox_prop", $currentuinfo['mailbox_prop']);
	bbs_setuserparam($userdefine0, $userdefine1, $mailbox_prop);
	html_success_quit("参数修改成功");
	exit;
}
showOptionsForm();
page_footer();


function getOptions($var_name, $oldvalue) {
	global $$var_name;
	$userdefine = $$var_name;
	$ccc = count($userdefine);
	$flags = $oldvalue;
	for ($i = 0; $i < $ccc; $i++) {
		if (isset($_POST[$var_name."_".$i])) {
			if ($_POST[$var_name."_".$i] == 1) {
				$flags |= (1<<$i);
			} else {
				$flags &= ~(1<<$i);
			}
		}
	}
	return $flags;
}


function showOptions($var_name, $userparam, $isWWW) {
	global $$var_name;
	$userdefine = $$var_name;
	$ccc = count($userdefine);
	for ($i = 0; $i < $ccc; $i++) {
		if ($userdefine[$i][0]!=$isWWW) 
			continue;
		$flag=1<<$i;
?>
<label><?php echo $userdefine[$i][2]; ?></label>   
<input type="radio" name="<?php echo $var_name."_".$i; ?>" value="1" <?php if ($userparam & $flag) echo "checked"; ?> ><?php echo $userdefine[$i][3]; ?>
 
<input type="radio" name="<?php echo $var_name."_".$i; ?>" value="0" <?php if (!($userparam & $flag)) echo "checked"; ?> ><?php echo $userdefine[$i][4]; ?><br/>
<?php
	}	
}

function showDefines($isWWW) {
	global $currentuser;
	global $currentuinfo;
	showOptions("user_define", $currentuser['userdefine0'], $isWWW);
	showOptions("user_define1", $currentuser['userdefine1'], $isWWW);
	showOptions("mailbox_prop", $currentuinfo['mailbox_prop'], $isWWW);
}

function showOptionsForm(){
	global $currentuser;
	global $user_define,$user_define_num;
?>
<form action="bbsparm.php?do=1" method="post" class="large align" id="ft">
	<fieldset><legend>用户个人参数</legend><div class="inputs">
<?php
	showDefines(1);
?>
	</div></fieldset>
<?php
	if (SHOWTELNETPARAM == 1) {
?>
	<fieldset><legend>用户个人参数（telnet方式专用）</legend><div class="inputs">
<?php
		showDefines(0);
?>
	</div></fieldset>
<?php
	}
?>
<div class="oper"><input type="submit" value="确定修改"></div>
</form>
<?php
}
