<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("删除邮件");

requireLoginok();

show_nav();

echo "<br>";
$boxDesc=getMailBoxName($_GET['boxname']);
head_var($userid."的".$boxDesc,"usermailbox.php?boxname=".$_GET['boxname'],0);
main();

show_footer();

function main(){
	global $_GET;
	global $boxDesc;
	$boxName=$_GET['boxname'];
	if (!isset($_GET['file'])) {
		foundErr("您所指定的信件不存在!");
	}
	$file=$_GET['file'];
	if ($boxName=='') {
		$boxName='inbox';
	}
	if (getMailBoxPathDesc($boxName, $path, $desc)) {
		deletemail($boxName, $path, $desc, $file);
	} else {
		foundErr("您指定了错误的邮箱名称！");
	}
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
		break;
	case -2:
        foundErr("错误的参数!");
        break;
	}
	return html_success_quit('返回'.$boxDesc, 'usermailbox.php?boxname='.$boxName);
}

?>