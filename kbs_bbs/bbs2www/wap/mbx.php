<?php
require("wapfuncs.php");
if(loginok())
{
  if ($userid == "guest")
    {
      authpanic("guest没有邮箱");
    }
  else
    {
      $mailboxes = loadmboxes($userid);
      waphead(0);
      echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
      echo "<p>";
      $mailboxnum = count($mailboxes["path"]);
      bbs_getmailnum($userid, $total, $unread, 0, 0);
      for( $i = 0; $i < $mailboxnum; $i++)
	{
	  showlink(urlstr("listmail",array(),encarg("listmail",array("mbox"=>$i))),$mailboxes["title"][$i]);
	  echo "(".((($i==0)&&($unread!=0))?$unread.'/':'').
	    $mailboxes["nums"][$i].")";
	  echo "<br/>";
	}
      showlink(urlstr("writemail"),"发邮件");
      echo "<br/>";
      showlink(urlstr("menu"),"主菜单");
      echo "</p>";      
    }
}
?>
</card>
</wml>

<?php

?>