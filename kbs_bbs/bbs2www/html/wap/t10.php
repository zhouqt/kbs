<?php
require("wapfuncs.php");
if(loginok())
{
  waphead(0);
  echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
  echo "<p>menus...</p>";
  echo "</card>";
  echo "</wml>";
}
?>
