<?php
	/*this file do some mail's action	$id:windinsn nov 8,2003	*/
	require("funcs.php");
	
	if ($loginok != 1)
		html_nologin();
	elseif(!strcmp($currentuser["userid"],"guest")){
		html_init("gb2312");
		html_error_quit("guest 没有自己的邮箱!");
		}
	else{
		$act = $_GET["act"];
		$dirname = $_GET["dir"];
		$title = $_GET["title"];
		$mail_num = 19;
		
		if (strstr($dirname,'..')){
			html_init("gb2312");
                        html_error_quit("读取邮件数据失败!");
                        }
		
		if($act == "del"){
			$filename = $_GET["file"];
			$ret = bbs_delmail($dirname,$filename);
			if($ret != 0){
				html_init("gb2312");
				html_error_quit("信件不存在或参数错误, 无法删除");
				die();
				}
			}
		elseif($act == "move"){
			for($i=0;$i < $mail_num;$i++)
			{
				if(!isset($_POST["file".$i])||$_POST["file".$i]=="")
					continue;
				
				$filename = $_POST["file".$i];
				$ret = bbs_delmail($dirname,$filename);
			}
			}	
		header("Location:bbsmailbox.php?path=".urlencode($dirname)."&title=".urlencode($title));
		}
?>