<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("删除邮件");

show_nav();

echo "<br>";

$boxDesc=getMailBoxName($_GET['boxname']);

if (!isErrFounded()) {
	head_var($userid."的".$boxDesc,"usermailbox.php?boxname=".$_GET['boxname'],0);
}

if ($loginok==1) {
	main();
}else {
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
}


if (isErrFounded()) {
		html_error_quit();
} 
show_footer();

function main(){
	global $_GET;
	global $boxDesc;
	$boxName=$_GET['boxname'];
	if (!isset($_GET['file'])) {
		foundErr("您所指定的信件不存在!");
		return false;
	}
	$file=$_GET['file'];
	if ($boxName=='') {
		$boxName='inbox';
	}
	if ($boxName=='inbox') {

		deletemail('inbox','.DIR','收件箱', $file);
		return true;
	}
	if ($boxName=='sendbox') {

		deletemail('sendbox','.SENT','发件箱',$file );
		return true;
	}
	if ($boxName=='deleted') {
		deletemail('deleted','.DELETED','垃圾箱',$file);
		return true;
	}
	foundErr("您指定了错误的邮箱名称！");
	return false;
}

function deletemail($boxName, $boxPath, $boxDesc, $filename){
	global $currentuser;
	$ret = bbs_delmail($boxPath,$filename);
	switch($ret){
	case 0:
		setSucMsg("邮件已成功删除！");
		break;
	case -1:
	    foundErr("您所指定信件不存在, 无法删除！");
		return false;
		break;
	case -2:
        foundErr("错误的参数!");
		return false;
        break;
	}
	return html_success_quit('返回'.$boxDesc, 'usermailbox.php?boxname='.$boxName);
}

?>