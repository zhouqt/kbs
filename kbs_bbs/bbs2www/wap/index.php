<?php
require("wapfuncs.php");
$havetoget = 0; 

//***********************************
//begin dealing cookies

@$cuid = $_COOKIE['U'];
@$cupwd = $_COOKIE['P'];
@$clear = $_GET['c'];

if($clear == 1)
{
  //  setcookie("PWD",'',time()-360,urlstr(index));
  //  setcookie("UID",'',time()-360,urlstr(index));
  setcookie("P",'',time()-360,"/");
  setcookie("U",'',time()-360,"/");

}
waphead(0);

echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
echo "<p align=\"center\">";
echo BBS_FULL_NAME;
echo "</p>";
echo "<p align=\"left\">";

if($cuid != '' && empty($clear))
{
  echo "´úºÅ:$cuid<br/>";
  echo "ÃÜÂë:";
  $len = strlen($cupwd);
  for($i = 0; $i < $len; ++$i)
    echo "*";
  echo "</p>";
  echo "<p align=\"center\">";
  echo "<do type=\"accept\" label=\"ËÉ°ó\">";
  echo "<go href=\"".urlstr('index',array('c'=>'1'))."\"></go>";
  echo "</do>";
  echo "<do type=\"button\" label=\"µÇÂ½\">";
  echo "<go href=\"".urlstr('login',array('u'=>$cuid,'p'=>$cupwd))."\"></go>";
  echo "</do>";
  echo "</p>";
}
else
{
  echo "´úºÅ:<input name=\"u\" size=\"7\" /><br/>";
  echo "ÃÜÂë:<input emptyok=\"true\" type=\"password\" name=\"p\" size=\"7\" />";
  echo "</p>";
  echo "<p align=\"center\">";
  echo "<do type=\"accept\" label=\"ËÉ°ó\">";
  echo "<go href=\"".urlstr('index',array('c'=>'1'))."\"></go>";
  echo "</do>";
  echo "<do type=\"button\" label=\"µÇÂ½\">";
  echo "<go href=\"".urlstr('login',array('u'=>'$(u)','p'=>'$(p)'))."\"></go>";
  echo "</do>";
  echo "</p>";
}
?>
</card>
</wml>
