<?
require("funcs.jsp");
$data = array ();
$id = $_POST["id"];
$passwd = $_POST["passwd"];
$kick_multi = $_POST["kick_multi"];
$error=-1;
if ($id!="") {
    if (($id!="guest")&&bbs_checkpasswd($id,$passwd)!=0)
      $loginok=6;
    else {
      $kick=0;
      if ($kick_multi!="")
	  	$kick=1;
      $error=bbs_wwwlogin($kick);
      if (($error!=0)&&($error!=2)) {
        $loginok=6;
	  if ($error==-1) 
		$loginok=4;
      }
      else {
        $loginok=0;
        $num=bbs_getcurrentuinfo($data);
        setcookie("UTMPKEY",$data["utmpkey"],time()+360000,"/");
        setcookie("UTMPNUM",$num,time()+360000,"/");
        setcookie("UTMPUSERID",$data["userid"],time()+360000,"/");
        setcookie("LOGINTIME",$data["logintime"],time()+360000,"/");
	    header("Location: /frames.html");
	    return;
      }
    }
}
?>
<html>

<?
if ($loginok != 1) {
  if ($loginok==6) {
?>
<body >
<SCRIPT language="javascript">
	alert("用户密码错误，请重新登陆！"+
<?
 echo "\"$error\"";
?>
);
	window.location = "/index.html";
</SCRIPT>
</body>
</html>
<?
    return;
  } else {
?>
<body >
<form name="infoform" action="bbslogin.jsp" method="post">
<input class="default" type="hidden" name="id" maxlength="12" size="8" value=<?
echo "\"$id\"";
?>
><br>
<input class="default" type="hidden" name="passwd" maxlength="39" size="8" value=<?
echo "\"$passwd\"";
?>
><br>
<input class="default" type="hidden" name="kick_multi" value="1" maxlength="39" size="8"><br>
</form> 
<SCRIPT language="javascript">
	if (confirm("你登陆的窗口过多，是否踢出多余的窗口？"))
		document.infoform.submit();
	else
		window.location = "/index.html";
</SCRIPT>
</body>
</html>
<?
	return;
  }
} 
?>
