<?php
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
		$filename=bbs_sethomefile($currentuser["userid"],"signatures");
		if ($_GET["type"]=="1") {
			if(!strcmp($currentuser["userid"],"guest"))
        		html_error_quit("不能给guest设置签名档!");
		    $fp=@fopen($filename,"w+");
                    if ($fp!=false) {
		    fwrite($fp,str_replace("\r\n", "\n", $_POST["text"]));
		    fclose($fp);
                    }
		}
?>
<body>
<center><?php echo BBS_FULL_NAME; ?> -- 设置签名档 [使用者: <?php echo $currentuser["userid"]; ?>]</center><hr />
<form name=form1 method="post" action="bbssig.php?type=1">
签名档每6行为一个单位, 可设置多个签名档.<table width="610" border="1"><tr><td><textarea name="text"  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.form1.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.form1.submit()' rows="20" cols="100" wrap="physical">
<?php
if ($_GET["type"]!="1") {
    $fp = @fopen ($filename, "r");
    if ($fp!=false) {
    while (!feof ($fp)) {
        $buffer = fgets($fp, 300);
        echo $buffer;
    }
    fclose ($fp);
    }
}
else {
    echo $_POST["text"];
}
?>
</textarea></table>
<input type="submit" value="存盘" /> <input type="reset" value="复原" />
</form><hr />
</body>
</html>
<?php
	}
?>
