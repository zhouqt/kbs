<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("察看邮件");

show_nav();

echo "<br><br>";

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
	if (!isset($_GET['num'])) {
		foundErr("您所指定的信件不存在!");
		return false;
	}
	$num=intval($_GET['num']);
	if ($boxName=='') {
		$boxName='inbox';
	}
	if ($boxName=='inbox') {
		showmail('inbox','.DIR','收件箱', $num);
		return true;
	}
	if ($boxName=='sendbox') {
		showmail('sendbox','.SENT','发件箱',$num );
		return true;
	}
	if ($boxName=='deleted') {
		showmail('deleted','.DELETED','垃圾箱',$num);
		return true;
	}
	foundErr("您指定了错误的邮箱名称！");
	return false;
}

function showmail($boxName, $boxPath, $boxDesc, $num){
	global $currentuser;
?>
<table cellpadding=3 cellspacing=1 align=center class=tableborder1>
            <tr>
                <th colspan=3>欢迎使用邮件功能，<?php echo $currentuser['userid'] ; ?></th>
            </tr>

<?php
	$dir = bbs_setmailfile($currentuser["userid"],$boxPath);

	$total = filesize( $dir ) / 256 ;
	if( $total <= 0 ){
?>
<tr><td>您所指定的信件不存在。
</td></tr>
</table>
<?php
		return false;
	}
	$articles = array ();
	if( bbs_get_records_from_num($dir, $num, $articles) ) {
		$file = $articles[0]["FILENAME"];
	}else{
?>
<tr><td>您所指定的信件不存在。
</td></tr>
</table>
<?php
		return false;
	}

	$filename = bbs_setmailfile($currentuser["userid"],$file) ;

	if(! file_exists($filename)){
?>
<tr><td>您所指定的信件不存在。
</td></tr>
</table>
<?php
		return false;
	}
?>
    <tr>
	    <td class=tablebody1 valign=middle align=center colspan=3><a href="deleteusermail.php?file=<?php echo $file; ?>&boxname=<?php echo $boxName; ?>"><img src="pic/m_delete.gif" border=0 alt="删除邮件"></a> &nbsp; <a href="sendmail.php"><img src="pic/m_write.gif" border=0 alt="发送消息"></a> &nbsp;<a href="sendmail.php?num=<?php echo $num ;?>&boxname=<?php echo $boxName; ?>"><img src="pic/m_reply.gif" border=0 alt="回复消息"></a>&nbsp;<a href="forwardusermail.php?num=<?php echo $num ;?>"><img src=pic/m_fw.gif border=0 alt=转发消息></a></td>
    </tr>
    <tr><td class=tablebody2 height=25>
		    <b><?php echo $articles[0]['OWNER'] ;?></b> 在 <b><?php echo strftime("%Y-%m-%d %H:%M:%S", $articles[0]['POSTTIME']); ?></b> 给您发送的信件：<b>[<?php echo htmlspecialchars($articles[0]['TITLE'],ENT_QUOTES) ;?>]</b>
</td>
                </tr>
                <tr>
                    <td  class=tablebody1 valign=top align=left>
					<b>
<?php
					echo bbs_printansifile($filename);
?>
	</b>
	&nbsp;
                    </td>
                </tr>
				<tr align=center><td width="100%" class=tablebody2>
				<a href="<?php   echo 'usermailbox.php?boxname='.$boxName; ?>"> << <?php echo '返回'.$boxDesc; ?></a>
				</td></tr>
                </table>
<?php
		bbs_setmailreaded($dir,$num);
}

?>