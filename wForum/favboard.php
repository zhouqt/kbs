<?php
require("inc/funcs.php");
require("inc/usermanage.inc.php");
require("inc/user.inc.php");

setStat("用户收藏版面");

show_nav();	

if ($loginok==1) {
	showUserMailbox();
	head_var($userid."的控制面板","usermanagemenu.php",0);
	main();	
}else {	
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");	
}

if (isErrFounded())	{
	html_error_quit();
}

show_footer();

function main()	{
	global $currentuser;
	
	if (isset($_GET["select"]))
		$select	= $_GET["select"];
	else
		$select	= 0;
	settype($select, "integer");

	if ($select	< 0) {
		foundErr("错误的参数");
		return false;
	}
	if (bbs_load_favboard($select)==-1) {
		foundErr("错误的参数");
		return false;
	}

    if (isset($_GET["delete"]))
    {
            $delete_s=$_GET["delete"];
            settype($delete_s,"integer");
            bbs_del_favboard($select,$delete_s);
    }
    if (isset($_GET["deldir"]))
    {
            $delete_s=$_GET["deldir"];
            settype($delete_s,"integer");
            bbs_del_favboarddir($select,$delete_s);
    }
    if (isset($_GET["dname"]))
    {
            $add_dname=trim($_GET["dname"]);
            if ($add_dname)
                bbs_add_favboarddir($add_dname);
    }
    if (isset($_GET["bname"]))
    {
            $add_bname=trim($_GET["bname"]);
            if ($add_bname)
                $sssss=bbs_add_favboard($add_bname);
    }

	showSecs($select, 0, true, 1); //第三个参数是 isFold，暂时设定为永远展开。如果要改那 showSecs() 函数也要改。- atppp
?>
<center>
<form action=favboard.php>增加目录: 输入目录名称&nbsp;<input name=dname size=24 maxlength=20 type=text value="">&nbsp;<input type=submit value=确定><input type=hidden name=select value=<?php echo $select;?>></form>
<form action=favboard.php>增加版面: 输入英文版名&nbsp;<input name=bname size=24 maxlength=20 type=text value="">&nbsp;<input type=submit value=确定><input type=hidden name=select value=<?php echo $select;?>></form>
<!-- 这个...wForum似乎把英文版名只用作内部数据交换，用户不直接接触英文版名，这个地方还要考虑考虑 -->
</center>
<?php
}
?>
