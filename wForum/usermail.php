<?php

require("inc/funcs.php");
require("inc/usermanage.inc.php");
require("inc/user.inc.php");
require("inc/ubbcode.php");

setStat("察看信件");

requireLoginok();

show_nav();

echo "<br>";

$boxDesc=getMailBoxName($_GET['boxname']);
head_var($userid."的".$boxDesc,"usermailbox.php?boxname=".$_GET['boxname'],0);
main();

show_footer();

function main(){
	global $boxDesc;
	$boxName=$_GET['boxname'];
	if (!isset($_GET['num'])) {
		foundErr("您所指定的信件不存在!");
	}
	$num=intval($_GET['num']);
	if ($boxName=='') {
		$boxName='inbox';
	}
	if (getMailBoxPathDesc($boxName, $path, $desc)) {
		showmail($boxName, $path, $desc, $num);
	} else {
		foundErr("您指定了错误的邮箱名称！");
	}
}

function showmail($boxName, $boxPath, $boxDesc, $num){
	global $currentuser;
?>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
            <tr>
                <th colspan=3>欢迎使用信件功能，<?php echo $currentuser['userid'] ; ?></th>
            </tr>

<?php
	$dir = bbs_setmailfile($currentuser["userid"],$boxPath);

	if( filesize( $dir ) <= 0 ){
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
    <tr><td class=TableBody1 valign=middle align=center colspan=3>
	    <a href="deleteusermail.php?file=<?php echo $file; ?>&boxname=<?php echo $boxName; ?>"><img src="pic/m_delete.gif" border=0 title="删除信件"></a>&nbsp;
	    <a href="sendmail.php"><img src="pic/m_write.gif" border=0 title="撰写信件"></a>&nbsp;
	    <a href="sendmail.php?num=<?php echo $num ;?>&boxname=<?php echo $boxName; ?>"><img src="pic/m_reply.gif" border=0 title="回复信件"></a>&nbsp;
	    <a href="sendmail.php?num=<?php echo $num ;?>&boxname=<?php echo $boxName; ?>&forward=1"><img src=pic/m_fw.gif border=0 title="转发信件"></a></td>
    </tr>
    <tr><td class=TableBody2 height=25>
		    <b><?php echo $articles[0]['OWNER'] ;?></b> 在 <b><?php echo strftime("%Y-%m-%d %H:%M:%S", $articles[0]['POSTTIME']); ?></b> 给您发送的信件：<b>[<?php echo htmlspecialchars($articles[0]['TITLE'],ENT_QUOTES) ;?>]</b>
</td>
                </tr>
                <tr>
                    <td  class=TableBody1 valign=top align=left>
					<b>
<?php
					echo dvbcode(bbs_printansifile($filename,1,"bbsmailcon.php?" . $_SERVER['QUERY_STRING']),0,'TableBody2');
?>
	</b>
	&nbsp;
                    </td>
                </tr>
				<tr align=center><td width="100%" class=TableBody2>
				<a href="<?php   echo 'usermailbox.php?boxname='.$boxName; ?>"> << <?php echo '返回'.$boxDesc; ?></a>
				</td></tr>
                </table>
<?php
		bbs_setmailreaded($dir,$num);
}

?>
