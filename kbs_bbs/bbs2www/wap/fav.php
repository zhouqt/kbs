<?php
require("wapfuncs.php");
if(loginok())
{
  if ($userid == "guest")
    {
      authpanic("guest没有在线好友");
    }
  else
    {
      $argv = decarg("favboard", $_GET['x']);
      if(!isset($argv["select"])) $select = -1;
      else $select = intval($argv["select"]);
      if($select > 818) $select = 0;
      if(!isset($argv["up"])) $father = -1;
      else $father = intval($argv["up"]);
      if($father > 818) $father = 0;

      $page = intval($argv["page"]);
      $pagesize = 12; //assume 50bytes/entry,so 50*12=600bytes~1kbytes
      $startnum = $page * $pagesize;
      waphead(0);
      checkmm();
      echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
      echo "<p align=\"center\">";
      if (($select < -1) || (bbs_load_favboard($select)==-1))
	{
	  echo "错误的参数";
	}
      else
	{
	  $boards = bbs_fav_boards($select, 1);
	  if ($boards == FALSE) echo "读取版列表失败";
	  else {
	    $brd_name = $boards["NAME"]; // 英文名
	    $brd_desc = $boards["DESC"]; // 中文描述
	    $brd_class = $boards["CLASS"]; // 版分类名
	    $brd_bm = $boards["BM"]; // 版主
	    $brd_artcnt = $boards["ARTCNT"]; // 文章数
	    $brd_unread = $boards["UNREAD"]; // 未读标记
	    $brd_zapped = $boards["ZAPPED"]; // 是否被 z 掉
	    $brd_position= $boards["POSITION"];//位置
	    $brd_flag= $boards["FLAG"];//目录标识
	    $brd_bid= $boards["BID"];//目录标识
	    $num = count($brd_name);
	    $endnum = min($num,$startnum + $pagesize);
	    for($i = $startnum; $i < $endnum; $i++)
	      {
		if( $brd_unread[$i] ==-1 && $brd_artcnt[$i] ==-1)
		  continue;
		echo ($i+1).".";
		if ($brd_flag[$i] ==-1 )
		  {
		    echo "+";
		    showlink(urlstr("favboard",array(),encarg("favboard",array("select" => transelect($brd_bid[$i]), "up" => transelect($select)))),$brd_desc[$i]);
		    echo "<br/>";
		    continue;
		  }
		$brdarr = array();
                $brdnum = bbs_getboard($brd_name[$i], $brdarr);
		showlink(urlstr("showboard",array(),encarg("showboard",array("board" => $brdnum))),$brd_name[$i]);
		echo "<br/>";
	      }
	    if ($select != -1) {
	      showlink(urlstr("favboard",array(),encarg("favboard",array("select" => transelect($father)))),"上一级");
	      echo "<br/>";
	    }
	    if ($page >0)
	    showlink(urlstr("favboard",array(),encarg("favboard",array("select" => transelect($select),"page" => ($page - 1)))),"上一页");
	    if ($num > $endnum) {
	      showlink(urlstr("favboard",array(),encarg("favboard",array("select" => transelect($select),"page" => ($page + 1)))),"下一页");
	      echo "<br/>";
	    }
	  }
	}
      showlink(urlstr("menu"),主菜单);
      echo "</p>";      
      bbs_release_favboard();
    }
}
?>
</card>
</wml>

<?php
function transelect($num)
{
  if($num == 0) return 904;
  if($num == -1) return 0;
  return $num;
}
?>