<?php
    /**
     * This file handles user login.
     * $Id$
     */
require("funcs.php");

set_fromhost();

$data = array ();
@$id = $_POST["id"];
@$passwd = $_POST["passwd"];
@$kick_multi = $_POST["kick_multi"];
@$mainurl = $_GET["mainurl"];
$error=-1;
if ($mainurl!="") $mainurl=urlencode($mainurl);
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
      if ($error==7)
        $loginok=7;
      }
      else {
        $loginok=0;
        $num=bbs_getcurrentuinfo($data);
        
        setcookie("UTMPKEY",$data["utmpkey"],0,"");
        setcookie("UTMPNUM",$num,0,"");
        setcookie("UTMPUSERID",$data["userid"],0,"");
        setcookie("LOGINTIME",$data["logintime"],0,"");
        
        if ($data["userid"]=="guest") {
            setcookie("WWWPARAMS",WWW_DEFAULT_PARAMS,0,"");
        }
        else  {
            $wwwparameters = "";
            bbs_getwwwparameters($wwwparameters);
            setcookie("WWWPARAMS",$wwwparameters,0,"");     
            $currentuser_num=bbs_getcurrentuser($currentuser);
            
            if (!($currentuser["userlevel"] & BBS_PERM_LOGINOK ))
            {
                $mainurl = "/bbsnew.php";
            }
        }
        if ($mainurl!="")
            header("Location: /wframes.php?mainurl=" . $mainurl);
        else
            header("Location: /wframes.php");
        return;
      }
    }
} else {
?>
<?xml version="1.0" encoding="gb2312"?>
<!DOCTYPE html
     PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=gb2312" />
    <title><?php echo BBS_FULL_NAME; ?></title>
  </head>
<body >
<SCRIPT language="javascript">
    alert("用户名不能为空!");
    window.location = "/index.html";
</SCRIPT>
</body>
</html>
<?php
    return;
}
?>
<html>

<?php
if ($loginok != 1) {
  if ($loginok==6) {
?>
<body >
<SCRIPT language="javascript">
    alert("用户密码错误，请重新登陆！"+
<?php
 echo "\"$error\"";
?>
);
    window.location = "/index.html";
</SCRIPT>
</body>
</html>
<?php
    return;
  }
  elseif ($loginok==7) {
?>
<body >
<SCRIPT language="javascript">
    alert("对不起,当前位置不允许登录该ID"+
<?php
 echo "\"$error\"";
?>
);
    window.location = "/index.html";
</SCRIPT>
</body>
</html>
<?php
    return;
  }
  else {
?>
<body >
<?php
  if ($mainurl!="") {
?>
<form name="infoform" action="bbslogin.php?mainurl=<?php echo $mainurl; ?>" method="post">
<?php
  } else {
?>
<form name="infoform" action="bbslogin.php" method="post">
<?php
  } //mainurl
?>
<input class="default" type="hidden" name="id" maxlength="12" size="8" value=<?php
echo "\"$id\"";
?>
><br>
<input class="default" type="hidden" name="passwd" maxlength="39" size="8" value=<?php
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
<?php
    return;
  }
} 
?>
