<?php
require("wapfuncs.php");

//************************

if(loginok())
{
  waphead(0);
  if ($userid != 'guest') checkmm();
  echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
  echo "<p align=\"center\">主菜单</p>";
  echo "<p>";
  showlink(urlstr('top10'),"十大话题");echo "<br/>";
  if($userid != 'guest')
    {
      showlink(urlstr('favboard'),"个人定制");echo "<br/>";
      showlink(urlstr('onlinefriend'),"在线好友");echo "<br/>";
      showlink(urlstr('mailbox'),"处理邮件");echo "<br/>";
      showlink(urlstr('sendmsg'),"发送讯息");echo "<br/>";
    }
  echo "<input emptyok=\"true\" type=\"text\" name=\"a\" size=\"10\"/><br/>";
  showlink(urlstr('showuser',array('n'=>'$(a)')),"查询网友");echo "<br/>";
  showlink(urlstr('searchboard',array('n'=>'$(a)')),"查询版面");echo "<br/>";
  showlink(urlstr('sysinfo'), "系统信息"); echo "<br/>";
  showlink(urlstr('logout'),"退出");echo "<br/>";
  echo "</p>";
}
?>
</card>
</wml>
