<?php
require("wapfuncs.php");
if(loginok())
{
  if ($userid == "guest")
    {
      authpanic("过客无法发送讯息");
    }
  else
    {
      @$origmsg = $_GET['m'];
      $msg = smarticonv($origmsg);
      @$destid = $_GET['t'];
      @$destutmpstr = $_GET['i'];

      if (!empty($destid))
	{
	  if (empty($destumpstr))
	    {
	      $destutmp=0;
	    }
	  else
	    {
	      $destutmp = decodesession($destutmpstr);
	    }
	  bbs_sendwebmsg($destid, $msg, $destutmp, $errmsg);
	  waphead(0);
	  echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
	  echo "<p>$errmsg</p>";
	  echo "<do type=\"prev\" label=\"返回\"><prev/></do>";
	}
      else
	{
	  waphead(0);
	  checkmm();
          echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
	  echo "<p>";
	  echo "对<input emptyok=\"true\" name=\"t\" size=\"7\"/><br/>";
	  echo "说<input emptyok=\"true\" name=\"m\" size=\"20\"/><br/>";
          echo "<a href=\"".urlstr('sendmsg',array('m'=>'$(m)','t'=>'$(t)'))."\">发送</a>";
	  echo "</p>";
	  echo "<p>";
	  showlink(urlstr('menu'),"主菜单");
	  echo "</p>";
	}
    }
}
?>
</card>
</wml>
