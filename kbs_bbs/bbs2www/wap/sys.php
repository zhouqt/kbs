<?php
require("wapfuncs.php");
if(loginok())
{
  setlocale(LC_ALL, "zh_CN");
  $time = strftime("<br/> %T <br/> %A %D <br/>");
  $uolnum = bbs_getonlinenumber();
  $webnum = bbs_getwwwguestnumber();
  waphead(0);
  checkmm();
  echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
  echo "<p>";
  echo "系统标准时间$time";
  echo "目前在线 $uolnum 人<br/>($webnum WWW GUEST)<br/>";
  echo "</p>";
}
?>
</card>
</wml>
