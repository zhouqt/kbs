<?php
require("wapfuncs.php");
if(loginok())
{
  if ($userid == "guest")
    {
      authpanic("过客无法发表文章");
    }
  else
    {
      $target = $_GET['to'];
      @$origmsg = $_GET['m'];
      $msg = smarticonv($origmsg);
      @$origtitle = $_GET['t'];
      $title = smarticonv($origtitle);
      waphead(0);
      echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
      echo "<p>";
      if(empty($msg) || empty($title) || empty($target)){
	showmailform($target);
	}
      else {
	bbswapmail($target, $title, $msg);
      }
      echo "</p>";
    }
}
?>
</card>
</wml>

<?php
function bbswapmail($target, $title, $content)
{
  $retn = bbs_postmail($target, preg_replace("/\\\(['|\"|\\\])/","$1",$title), preg_replace("/\\\(['|\"|\\\])/","$1",$content), 0, 0);
  switch ($retn) {
  case -1:
    echo "发信失败:无法创建文件<br/>";
    break;
  case -2:
    echo "发信失败:对方拒收你的邮件<br/>";
    break;
  case -3:
    echo "发信失败:对方信箱满<br/>";
    break;
  default:
    echo "成功寄出<br/>";
  }
  echo "<do type=\"prev\" label=\"返回\"><prev/></do>";
  return;
}

function showmailform($target)
{
   echo "收信人<input emptyok=\"true\" name=\"to\" size=\"20\""
     .((empty($target))?"":" value=\"".$target."\"")."/><br/>";
  echo "标题<input emptyok=\"true\" name=\"t\" size=\"20\"/><br/>";
  echo "正文<input emptyok=\"true\" name=\"m\" size=\"80\"/><br/>";
  showlink(urlstr('writemail',array('m'=>'$(m)','t'=>'$(t)','to'=>'$(to)')),"发送");
  echo "<br/>";
  showlink(urlstr("menu"),"主菜单");
}
?>