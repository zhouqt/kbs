<?php
require("wapfuncs.php");
if(loginok())
{
  $id = $_GET["n"];
  $lookupuser = array ();
  waphead(0);
  echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
  
  if ($id=="" || (bbs_getuser($id, $lookupuser) == 0))
    {
      echo "<p align=\"center\">";
      echo "该用户不存在<br/>";
      showlink(urlstr("menu"),主菜单);
      echo "</p>";
    }
  else
    {
      $usermodestr = bbs_getusermode($id);
      echo "<p>";
      echo $lookupuser["userid"].'('
	.htmlspecialchars(stripansi($lookupuser["username"])).')';
      echo "上站".$lookupuser["numlogins"]."次，发表"
	.$lookupuser["numposts"]."篇文章。<br/>";
      echo "上次在".date("D M j H:i:s Y",$lookupuser["lastlogin"]);
      echo "<br/>从[".substr($lookupuser["lasthost"], 0, 
			strrpos($lookupuser["lasthost"],"."))
	."***]上站。<br/>";
      echo "离线时间";
      if( $usermodestr!="" && $usermodestr{0}=="1" )
	{
	  echo date("[D M j H:i:s Y]", $lookupuser["lastlogin"]+60+( $lookupuser["numlogins"]+$lookupuser["numposts"] )%100 );
	} else if($lookupuser["exittime"] < $lookupuser["lastlogin"])
	  echo "[不详]";
      else
	echo date("[D M j H:i:s Y]", $lookupuser["exittime"]);
      echo "<br/>";
      if( bbs_checknewmail($lookupuser["userid"]) ) echo "有新信<br/>";
      echo '生命力['.bbs_compute_user_value($lookupuser["userid"]).'],<br/>';
      echo '身份['.bbs_user_level_char($lookupuser["userid"]).']<br/>';
      if ($usermodestr!="" && $usermodestr{1} != "")
	echo ereg_replace("\n","<br/>",stripansi(strip_tags(substr($usermodestr, 1))));
      echo "<br/>";
      showlink(urlstr("menu"),主菜单);
      echo "</p>";
    }
}
?>
</card>
</wml>
