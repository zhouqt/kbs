<?php
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
		$filename=bbs_sethomefile($currentuser["userid"],"msgfile");
?>
<body>
<pre>
<?php
    $fp = fopen ($filename, "r");
    if ($fp!=false) {
        while (!feof ($fp)) {
            $buffer = fgets($fp, 300);
            echo ansi_convert($buffer,"#000000","#F0F0FF");
        }
        fclose ($fp);
    } else {
?>
没有任何讯息
<?php
}
?>
<a onclick="return confirm('你真的要清除所有讯息吗?')" href="/cgi-bin/bbs/bbsdelmsg">清除所有讯息</a> <a href="/cgi-bin/bbs/bbsmailmsg">寄回所有信息</a></pre>
</pre>
</body>
</html>
<?php
	}
?>
