<?
  require("funcs.jsp");
  if ($loginok!=1) {
?>
<html>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<link rel="stylesheet" type="text/css" href="/bbs.css">
没有登陆<br><br>
<a href="/index.html">快速返回</a>
</html>
<? } else {
	bbs_wwwlogoff();
    setcookie("UTMPKEY","");
    setcookie("UTMPNUM","");
    setcookie("UTMPUSERID","");
    setcookie("LOGINTIME","");

	header("Location: /index.html");
   }
?>
