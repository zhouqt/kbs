<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("删除邮件");

show_nav();

echo "<br>";

$boxDesc=getMailBoxName($_POST['boxname']);

if (!isErrFounded()) {
	head_var($userid."的".$boxDesc,"usermailbox.php?boxname=".$_POST['boxname'],0);
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
	global $_POST;
	global $boxDesc;
	$boxName=$_POST['boxname'];
	if (!isset($_POST['nums'])) {
		foundErr("您所指定的信件不存在!");
		return false;
	}
	$action=$_POST['action'];
	if ($action=='deleteAll') {
		return deleteAll($boxName,$boxDesc);
	}
	if ($action!='delete') {
		foundErr("参数错误！");
		return false;
	}
	$nums=split(',',$_POST['nums']);
	if ($boxName=='') {
		$boxName='inbox';
	}
	if ($boxName=='inbox') {

		deleteMails('inbox','.DIR','收件箱', $nums);
		return true;
	}
	if ($boxName=='sendbox') {

		deleteMails('sendbox','.SENT','发件箱',$nums );
		return true;
	}
	if ($boxName=='deleted') {
		deleteMails('deleted','.DELETED','垃圾箱',$nums);
		return true;
	}
	foundErr("您指定了错误的邮箱名称！");
	return false;
}

function deleteMails($boxName, $boxPath, $boxDesc, $nums){
	global $currentuser;
	$dir = bbs_setmailfile($currentuser["userid"],$boxPath);

	$total = filesize( $dir ) / 256 ;
	if( $total <= 0 ){
		foundErr("您所指定的信件不存在。");
		return false;
	}
	$articles = array ();
	$mailnum=count($nums);
	$filenames=array();
	for ($i=0;$i<$mailnum;$i++) {
		if( bbs_get_records_from_num($dir, intval($nums[$i]), $articles) ) {
			$filenames[] = $articles[0]["FILENAME"];
		}else{
			foundErr("您所指定的信件不存在。");
			return false;
		}
	}
	for ($i=0;$i<$mailnum;$i++){
		$ret = bbs_delmail($boxPath,$filenames[$i]);
		switch($ret){
		case 0:
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
	}
	setSucMsg("邮件已成功删除！");
	return html_success_quit('返回'.$boxDesc, 'usermailbox.php?boxname='.$boxName);
}
?>