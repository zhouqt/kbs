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
      $argv = decarg("writepost",$_GET['x']);
      $brdnum = intval($argv["board"]);
      $reid = intval($argv["reid"]);
      @$origmsg = $_GET['m'];
      $msg = smarticonv($origmsg);
      @$origtitle = $_GET['t'];
      $title = smarticonv($origtitle);
      waphead(0);
      echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
      echo "<p>";
      
      if( $brdnum != 0 ){
	$brdname = bbs_getbname($brdnum);
	if ($brdname){
	  $brdarr = array();
	  bbs_getboard($brdname, $brdarr);
	  if (bbs_checkreadperm($currentuser_num, $brdnum) &&(!bbs_is_readonly_board($brdarr)) && bbs_checkpostperm($currentuser_num, $brdnum)){
	    $articles = array();
	    if(empty($msg)){//no content,show out post forms
	      if ($reid > 0) {//reply
		$num = bbs_get_records_from_id($brdname, $reid, $dir_modes["NORMAL"], $articles);
		if ($num == 0) {
		  echo "回复的原文章号错误<br/>";
		}
		else {
		  if(strncmp($articles[1]["TITLE"],"Re: ", 4)!=0)
		    $title = "Re: ".$articles[1]["TITLE"];
		  showpostform($brdnum, $title, $reid);
		}
	      }
	      else showpostform($brdnum, "", 0);
	    }
	    else{ // have content
	      if ($reid > 0) {//reply
		$num = bbs_get_records_from_id($brdname, $reid, $dir_modes["NORMAL"], $articles);
		if ($num == 0) {
		  echo "回复的原文章号错误<br/>";
		}
		else {
		  if ($articles[1]["FLAGS"][2] == 'y') {
		    echo "该文不可回复<br/>";
		  }
		  else {
		    if(empty($title)) {
		      if(strncmp($articles[1]["TITLE"],"Re: ", 4)!=0)
			$title = "Re: ".$articles[1]["TITLE"];
		      else $title = $articles[1]["TITLE"];
		    }
		    bbswappost($brdname, $title, $msg, $reid);
		  }
		}
	      }
	      else {
		if(empty($title)) {
		  echo "错误的发文请求<br/>";
		}
		else {
		  bbswappost($brdname, $title, $msg, 0);
		}
	      }
	    }
	  }
	  else {
	    echo "没有在本版发文章的权限<br/>";
	  }
	}
	else {
	  echo "错误的版面号<br/>";
	}
      }
      else{
	echo "不存在此版面号<br/>";
      }
      echo "</p>";
    }
}
?>
</card>
</wml>

<?php
function bbswappost($board, $title, $content, $reid)
{
  $retn = bbs_postarticle($board, preg_replace("/\\\(['|\"|\\\])/","$1",$title), preg_replace("/\\\(['|\"|\\\])/","$1",$content), 0, $reid, 0, 0);
  switch ($retn) {
  case -1:
    echo "错误的讨论区名称!<br/>";
    break;
  case -2:
    echo "本版为二级目录版!<br/>";
    break;
  case -3:
    echo "标题为空!<br/>";
    break;
  case -4:
    echo "此讨论区是唯读的, 或是您尚无权限在此发表文章!<br/>";
    break;
  case -5:
    echo "很抱歉, 你被版务人员停止了本版的post权利!<br/>";
    break;
  case -6:
    echo "两次发文间隔过密,请休息几秒再试!<br/>";
    break;
  case -7:
    echo "无法读取索引文件! 请通知站务人员, 谢谢! <br/>";
    break;
  case -8:
    echo "本文不可回复!<br/>";
    break;
  case -9:
    echo "系统内部错误, 请迅速通知站务人员, 谢谢!<br/>";
    break;
  case -21:
    echo "您的积分不符合当前讨论区的设定, 暂时无法在当前讨论区发表文章!<br/>";
    break;
  default:
    echo "发文成功<br/>";
  }
  echo "<do type=\"prev\" label=\"返回\"><prev/></do>";
  return;
}

function showpostform($boardnum,$title,$reid)
{
  echo "标题<input emptyok=\"true\" name=\"t\" size=\"20\""
    .((empty($title))?"":" value=\"".$title."\"")."/><br/>";
  echo "正文<input emptyok=\"true\" name=\"m\" size=\"80\"/><br/>";
  showlink(urlstr('writepost',array('m'=>'$(m)','t'=>'$(t)'), encarg('writepost',array("board"=>$boardnum,"reid"=>$reid))),"发表");
  echo "<br/>";
  showlink(urlstr("menu"),"主菜单");

}
?>
