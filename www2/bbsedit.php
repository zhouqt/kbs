<?php
	require("www2-funcs.php");
	require("www2-board.php");
	login_init();
	assert_login();

	if (isset($_GET["board"]))
		$board = $_GET["board"];
	else
		html_error_quit("�����������");
	// ����û��ܷ��Ķ��ð�
	$brdarr = array();
	$brdnum = bbs_getboard($board, $brdarr);
	if ($brdnum == 0)
		html_error_quit("�����������");
	bbs_set_onboard($brdnum,1);
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $brdnum) == 0)
		html_error_quit("�����������");
	$board = $brdarr['NAME'];
	if(bbs_checkpostperm($usernum, $brdnum) == 0) {
		html_error_quit("�������������������Ȩ�ڴ���������������");
	}
	if (bbs_is_readonly_board($brdarr))
		html_error_quit("������ֻ����������������");

	bbs_board_nav_header($brdarr, "�޸�����");
	
	if (isset($_GET['id']))
		$id = intval($_GET['id']);
	else
		html_error_quit("������ı��");
	$articles = array();
	$num = bbs_get_records_from_id($brdarr["NAME"], $id,$dir_modes["NORMAL"],$articles);
	if ($num == 0)
		html_error_quit("������ı��");
	$ret = bbs_caneditfile($board,$articles[1]['FILENAME']);
	switch ($ret) {
		case -1:
			html_error_quit("���������ƴ���");
			break;
		case -2:
			html_error_quit("���治���޸�����");
			break;
		case -3:
			html_error_quit("�����ѱ�����ֻ��");
			break;
		case -4:
			html_error_quit("�޷�ȡ���ļ���¼");
			break;
		case -5:
			html_error_quit("�����޸���������!");
			break;
		case -6:
			html_error_quit("ͬ��ID�����޸���ID������");
			break;
		case -7:
			html_error_quit("����POSTȨ����");
			break;
		default:
	}

	$brd_encode = urlencode($brdarr["NAME"]);
	
	if (isset($_GET['do'])) {
		$ret=bbs_updatearticle($board,$articles[1]['FILENAME'],$_POST['text']);
		switch ($ret) {
			case -1:
				html_error_quit("�޸�����ʧ�ܣ����¿��ܺ��в�ǡ������");
				break;
			case -10:
				html_error_quit("�Ҳ����ļ�!");
				break;
			case 0:
				html_success_quit("�����޸ĳɹ�",
				array("<a href='bbsdoc.php?board=" . $brd_encode . "'>���� " . $brdarr['DESC'] . "</a>",
				"<a href='bbscon.php?board=" . $brd_encode . "&id=" . $id . "'>���ء�" . htmlspecialchars($articles[1]['TITLE']) . "��</a>"));
				break;
			default:
		}
		html_error_quit("ϵͳ����");
	}
?>
<link rel="stylesheet" type="text/css" href="ansi.css"/>
<form name="postform" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>?board=<?php echo $brd_encode; ?>&id=<?php echo $id; ?>&do" class="large">
<div class="article">
<?php
	$notes_file = bbs_get_vote_filename($brdarr["NAME"], "notes");
	$fp = FALSE;
	if(file_exists($notes_file))
	{
	    $fp = fopen($notes_file, "r");
	    if ($fp == FALSE)
	    {
	    	$notes_file = "vote/notes";
            if(file_exists($notes_file))
    		    $fp = fopen($notes_file, "r");
		}
	}
	if ($fp == FALSE)
	{
?>
<div class="green">����ע������: <br/>
����ʱӦ���ؿ������������Ƿ��ʺϹ������Ϸ��������������ˮ��лл���ĺ�����</div>
<?php
	}
    else
	{
	    fclose($fp);
		echo bbs_printansifile($notes_file);
	}
?>
</div>
<fieldset><legend>�޸�����</legend>
������: <?php echo $articles[1]['OWNER']; ?>, ����: <?php echo $brd_encode; ?> [<a href="bbsdoc.php?board=<?php echo $brd_encode; ?>">��������</a>]<br/>
��&nbsp;&nbsp;��: <input readonly type="text" name="title" size="40" maxlength="100" value="<?php echo $articles[1]['TITLE']; ?>" />
[<a href="bbsedittitle.php?board=<?php echo $brd_encode; ?>&id=<?php echo $id; ?>">�޸ı���</a>]
<textarea name="text" onkeydown='return textarea_okd(dosubmit, event);' wrap="physical" id="sfocus">
<?php
	bbs_printoriginfile($board,$articles[1]['FILENAME']);
?>
</textarea>
<div class="oper">
<input type="button" onclick="dosubmit();" name="post" value="�޸�" />
<input type="reset" value="��ԭ" />
<input type="button" value="����" onclick="history.go(-1)" />
</div>
</form>
<?php
page_footer();
?>