<?php
require("wapfuncs.php");
if(loginok())
{
  bbs_wwwlogoff();
  setcookie("SID",'',time()-360,"/");
  authpanic("ÍË³ö³É¹¦");
}

?>
</card>
</wml>