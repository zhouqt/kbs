<?php
require("wapfuncs.php");
if(loginok())
{
  if ($userid == "guest") {
    authpanic("guest没有在线好友");
  }
  else {
    $page = decodesession($_GET['x']);
    $pagesize = 12; //assume 50bytes/entry,so 50*12=600bytes~1kbytes
    $friends = bbs_getonlinefriends();
    waphead(0);
    checkmm();
    echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
    echo "<p align=\"center\">";
    showonlinefriend($friends, $page, $pagesize);
    showlink(urlstr("menu"),主菜单);
    echo "</p>";      
  }
}
?>
</card>
</wml>

<?php
function showonlinefriend($friends, $page, $pagesize)
{
  $startnum = $page * $pagesize;
  if ($friends == 0)
    $num = 0;
  else
    $num = count($friends);
  echo ($num==0?"没有好友在线":"在线好友列表");
  echo "<br/>";
  $endnum = min($num,$startnum + $pagesize);
  for($i = $startnum; $i < $endnum; $i++)
    {
      //echo ($i+1).".";
      showlink(urlstr("showuser",array('n'=>$friends[$i]["userid"])),
	       $friends[$i]["userid"]);
      echo "<br/>";
    }
  if ($page >0)showlink(urlstr("onlinefriend",array(),encodesession($page-1)),"上一页");
  if ($num > $endnum){
    showlink(urlstr("onlinefriend",array(),encodesession($page+1)),"下一页");
    echo "<br/>";
  }
}
?>