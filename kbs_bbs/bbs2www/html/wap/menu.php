<?php
require("wapfuncs.php");
$backmenu=0;
//************************

if(loginok())
{
  waphead(0);
  if ($userid != 'guest') checkmm();
  echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
  echo "<p align=\"center\">主菜单</p>";
  echo "<p>";
  echo "<a href=\"".urlstr('top10')."\">十大话题</a><br/>";
  //  echo "<a href=\"".urlstr('
  echo ($userid != 'guest')?"<a href=\"".urlstr('readmail')."\">读邮件</a><br/>":'';
  echo ($userid != 'guest')?"<a href=\"".urlstr('sendmsg')."\">发消息</a><br/>":'';
  echo "<a href=\"".urlstr('logout')."\">退出</a><br/>";
  echo "</p>";
  echo "</card>";
  echo "</wml>";
}
?>
