<?php
require("wapfuncs.php");
if(loginok())
{
  $argv = decarg("readpost",$_GET['x']);
  $brdnum = intval($argv["board"]);
  $postid = intval($argv["id"]);
  $way = intval($argv["way"]);
  $page = intval($argv["page"]);
  waphead(0);
  echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
  echo "<p>";
  $brdarr = array();
  if( $brdnum != 0 ){
    $brdname = bbs_getbname($brdnum);
    if ($brdname){
      bbs_getboard($brdname, $brdarr);
      $isnormalboard=bbs_normalboard($brdname);
      if ($isnormalboard || bbs_checkreadperm($currentuser_num, $brdnum)!=0)
	{
	  //the lowest 2 bits are for direction of next article
	  //0 for current one
	  //1 for previous one
	  //2 for next one
	  //3 for current in getfrom_gid threads.
	  //$nextdir is defined in $dir_modes array 
	  $nextdir = ($way % 4);
	  $dirmode = ($way >> 2);
	  $posts = array ();
	  $haveprev = 0;
	  if( $nextdir == 3) {
	    $num = bbs_get_threads_from_gid($brdnum, $postid, $postid, $posts, $haveprev);
	  }
	  else {
	    $nextdir = 2 * $nextdir -3;
	    if ($nextdir != (-3)){
	      $articles = bbs_get_threads_from_id($brdnum, $postid, $dirmode, $nextdir); 
	      if ($articles != FALSE) {
		$postid = $articles[0]["ID"];
	      }
	    }
	    $num = bbs_get_records_from_id($brdarr["NAME"], $postid, $dirmode, $posts);
	  }

	  if ($num == 0) {
	    echo "错误的文章号";
	  }
	  else {
	    $filename=bbs_get_board_filename($brdarr["NAME"], $posts[($nextdir==3)?0:1]["FILENAME"]);
	    wapshowpost($filename, $page, $brdnum, $postid, $way);
	  }
	}
      else{echo "错误的讨论区<br/>";}
    }
    else{echo "错误的讨论区<br/>";}
  }
  else{echo "错误的讨论区<br/>";}
  showlink(urlstr("menu"),"主菜单");
  echo "</p>";  
}
?>
</card>
</wml>

<?php
function wapshowpost($fname, $page, $brdnum, $postid, $way)
{  
  $pagesize = 800;
  $fhandle = @fopen($fname, "rb");
  $fdata = ""; 
  $fbuf = "";
  while (!feof($fhandle)) $fdata .= fread($fhandle, 1024);
  fclose($fhandle);

  $fbuf1 = preg_split('/\x0/', $fdata, -1, PREG_SPLIT_NO_EMPTY);
  $fbuf2 = stripansi($fbuf1[0]);
  while(preg_match('/(.*)※(.*):·(.*)·\[FROM:(.*)\]/s', $fbuf2, $farray)){
    if($farray[1]) $fbuf2 = $farray[1];
  }
  $fbuf1 = preg_split('/\n/', $fbuf2, 4, PREG_SPLIT_NO_EMPTY);
  $fbody = "";
  if(preg_match('/发信人: (.*), 信区: (.*)/', $fbuf1[0], $farray)){
    $fwho = $farray[1];
    $fwhere = $farray[2];
  }
  else {
    $fwho = "";
    $fwhere = "";
    $fbody = $fbuf1[0];
  }

  if(preg_match('/标(\s)*题: (.*)/', $fbuf1[1], $farray)){
    $ftitle = $farray[2];
  }
  else {
    $ftitle = "";
    $fbody = $fbody.$fbuf1[1];
  }

  if(preg_match('/发信站: (.*)\((.*)\)(.*)/', $fbuf1[2], $farray)){
    $ftime = $farray[2];
    
  }
  else {
    $fsite = "";
    $fbody = $fbody.$fbuf1[2];
  }
  if(strncmp($ftitle, "Re: ", 4) != 0)
    $ftitle = "●".$ftitle;
  $ftime = preg_replace("/[\t\ ]+/", '.', $ftime);
  $fbody = $fwho."[".$ftime."]\n".$ftitle."\n".$fbody."\n".$fbuf1[3];
  $startnum = $page * $pagesize;
  $endnum = min($startnum+$pagesize,strlen($fbody));
  if($startnum < strlen($fbody)){
    $fbuf1 = ch_substr($fbody, $startnum, $pagesize);
    $fbuf2 = preg_replace("/[\t\ ]*/m", "", $fbuf1);
    $fbuf = preg_replace("/(\n)+/s","\n",$fbuf2);
    echo preg_replace("/\n/","<br/>",htmlspecialchars($fbuf));
    echo "<br/>";
    if($page > 0)
      showlink(urlstr("readpost",array(),encarg("readpost",array("board"=>$brdnum, "id"=>$postid, "way"=>$way, "page"=>($page -1)))),"上一页");
    if($endnum < strlen($fbody)) {
      showlink(urlstr("readpost",array(),encarg("readpost",array("board"=>$brdnum, "id"=>$postid, "way"=>$way, "page"=>($page+1)))),"下一页");
    }
    else {
      showlink(urlstr("writepost",array(),encarg("writepost",array("board"=>$brdnum, "reid"=>$postid))),"回复");
    }
    echo "<br/>";
  }
  else echo "错误的页号<br/>";
}
?>