<?php


$needlogin=1;

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");

global $boardArr;
global $boardID;
global $boardName;

preprocess();

setStat("发表文章");

show_nav();

if (isErrFounded()) {
	html_error_quit() ;
} else {
	?>
	<br>
	<TABLE cellSpacing=0 cellPadding=0 width=97% border=0 align=center>
	<?php

	if ($loginok==1) {
		showUserMailbox();
?>
</table>
<?php
	}

	board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);

	showPostArticles($boardID,$boardName,$boardArr);
}

//showBoardSampleIcons();
show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $loginok;
	if ($loginok!=1) {
		foundErr("游客不能发表文章。");
		return false;
	}
	if (!isset($_GET['board'])) {
		foundErr("未指定版面。");
		return false;
	}
	$boardName=$_GET['board'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName,$brdArr);
	$boardArr=$brdArr;
	if ($boardID==0) {
		foundErr("指定的版面不存在");
		return false;
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版");
		return false;
	}
	if (bbs_is_readonly_board($boardArr)) {
			foundErr("本版为只读讨论区！");
			return false;
	}
	if (bbs_checkpostperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版");
		return false;
	}
	return true;
}

function showPostArticles($boardID,$boardName,$boardArr){
	global $currentuser;
	global $_COOKIE;
?>
<script src="inc/ubbcode.js"></script>
<form action="dopostarticle.php" method=POST onSubmit=submitonce(this) name=frmAnnounce>
<input type="hidden" value="<?php echo $boardName; ?>" name="board">
<table cellpadding=3 cellspacing=1 class=tableborder1 align=center>
    <tr>
      <th width=100% height=25 colspan=2 align=left>&nbsp;&nbsp;发表新帖子</th>
    </tr>
<!--
        <tr>
          <td width=20% class=tablebody2><b>用户名</b></td>
          <td width=80% class=tablebody2><input name=username value="<?php   echo $currentuser['userid']; ?>" class=FormClass>&nbsp;&nbsp;<font color=#ff0000><b>*</b></font><a href=register.php>您没有注册？</a> 
          </td>
        </tr>
        <tr>
          <td width=20% class=tablebody1><b>密&nbsp;&nbsp;码</b></td>
          <td width=80% class=tablebody1><input name=passwd type=password value=<?php   echo $_COOKIE['PASSWORD']; ?> class=FormClass><font color=#ff0000>&nbsp;&nbsp;<b>*</b></font><a href=lostpass.php>忘记密码？</a></td>
        </tr>
-->
        <tr>
          <td width=20% class=tablebody2><b>主题标题</b>
              <SELECT name=font onchange=DoTitle(this.options[this.selectedIndex].value)>
              <OPTION selected value="">选择话题</OPTION> <OPTION value=[原创]>[原创]</OPTION> 
              <OPTION value=[转帖]>[转帖]</OPTION> <OPTION value=[灌水]>[灌水]</OPTION> 
              <OPTION value=[讨论]>[讨论]</OPTION> <OPTION value=[求助]>[求助]</OPTION> 
              <OPTION value=[推荐]>[推荐]</OPTION> <OPTION value=[公告]>[公告]</OPTION> 
              <OPTION value=[注意]>[注意]</OPTION> <OPTION value=[贴图]>[贴图]</OPTION>
              <OPTION value=[建议]>[建议]</OPTION> <OPTION value=[下载]>[下载]</OPTION>
              <OPTION value=[分享]>[分享]</OPTION></SELECT>
          </td>
          <td width=80% class=tablebody2><input name=subject size=70 maxlength=80>&nbsp;&nbsp;<font color=#ff0000><strong>*</strong></font>不得超过 25 个汉字或50个英文字符
	 </td>
        </tr>
        <tr>
          <td width=20% valign=top class=tablebody1><b>当前心情</b><br><li>将放在帖子的前面</td>
          <td width=80% class=tablebody1>
<?php   for ($i=0; $i<=$Forum_PostFaceNum-1; $i=$i+1)
  {
?>
	<input type="radio" value="<?php     echo $Forum_PostFace[$i]; ?>" name="Expression" <?php     if ($i==0)
    {
      print "checked";    } ?>><img src="<?php     echo $Forum_info[8].$Forum_PostFace[$i]; ?>" >&nbsp;&nbsp;
<?php     if ($i>0 && (($i+1)% 9==0))
    {
      print "<br>";    } ?>
<?php 
  } ?>
 </td>
        </tr>
<?php   
	if (bbs_is_attach_board($boardArr)) {
?>
<tr>
<td width=20% valign=middle class=tablebody2>
<b>文件上传</b>
<!--
<select size=1>
<option value="">允许类型</option>
<?php 
/*
    $iupload=explode('|', $Board_Setting[19]);
	$len=count($iupload);
    for ($i=0; $i<$len; $i=$i+1)
    {

      print "<option value=".$iupload[$i].">".$iupload[$i]."</option>";

    } 
*/
?>
</select>
-->
</td><td width=80% class=tablebody2>
<iframe name="ad" frameborder=0 width=100% height=24 scrolling=no src=postupload.php></iframe>
</td></tr>
<?php   } ?>
        <tr>
          <td width=20% valign=top class=tablebody1>
<b>内容</b><br>
<li>HTML标签： <?php   echo $Board_Setting[5]?"可用":"不可用"; ?>
<li>UBB标签： <?php   echo $Board_Setting[6]?"可用":"不可用"; ?>
<li>贴图标签： <?php   echo $Board_Setting[7]?"可用":"不可用"; ?>
<li>多媒体标签：<?php   echo $Board_Setting[9]?"可用":"不可用"; ?>
<li>表情字符转换：<?php   echo $Board_Setting[8]?"可用":"不可用"; ?>
<li>上传图片：<?php   echo $Forum_Setting[3]?"可用":"不可用"; ?>
<li>最多<?php   echo intval($Board_Setting[16]/1024); ?>KB<BR><BR>
<B>特殊内容</B><BR>
<li><?php   echo $Board_Setting[10]?"<a href=\"javascript:money()\" title=\"使用语法：[money=可浏览该部分内容需要金钱数]内容[/money]\">金钱帖</a>":"金钱帖：不可用"; ?>
<li><?php   echo $Board_Setting[11]?"<a href=\"javascript:point()\" title=\"使用语法：[point=可浏览该部分内容需要积分数]内容[/point]\">积分帖</a>":"积分帖：不可用"; ?>
<li><?php   echo $Board_Setting[12]?"<a href=\"javascript:usercp()\" title=\"使用语法：[usercp=可浏览该部分内容需要魅力数]内容[/usercp]\">魅力帖</a>":"魅力帖：不可用"; ?>
<li><?php   echo $Board_Setting[13]?"<a href=\"javascript:power()\" title=\"使用语法：[power=可浏览该部分内容需要威望数]内容[/power]\">威望帖</a>":"威望帖：不可用"; ?>
<li><?php   echo $Board_Setting[14]?"<a href=\"javascript:article()\" title=\"使用语法：[post=可浏览该部分内容需要文章数]内容[/post]\">文章帖</a>":"文章帖：不可用"; ?>
<li><?php   echo $Board_Setting[15]?"<a href=\"javascript:replyview()\" title=\"使用语法：[replyview]该部分内容回复后可见[/replyview]\">回复帖</a>":"回复帖：不可用"; ?>
<li><?php   echo $Board_Setting[23]?"<a href=\"javascript:usemoney()\" title=\"使用语法：[usemoney=浏览该部分内容需要消耗的金钱数]内容[/usemoney]\">出售帖</a>":"出售帖：不可用"; ?>
	  </td>
          <td width=80% class=tablebody1>
		 
<br>
字体：
<SELECT onchange="if(this.options[this.selectedIndex].value!=''){showfont(this.options[this.selectedIndex].value);this.options[0].selected=true;}else {this.selectedIndex=0;}" >
<option value="宋体" selected>宋体</option>
<option value="楷体_GB2312">楷体</option>
<option value="新宋体">新宋体</option>
<option value="黑体">黑体</option>
<option value="隶书">隶书</option>
<OPTION value="Andale Mono">Andale Mono</OPTION> 
<OPTION value=Arial>Arial</OPTION> 
<OPTION value="Arial Black">Arial Black</OPTION> 
<OPTION value="Book Antiqua">Book Antiqua</OPTION>
<OPTION value="Century Gothic">Century Gothic</OPTION> 
<OPTION value="Comic Sans MS">Comic Sans MS</OPTION>
<OPTION value="Courier New">Courier New</OPTION>
<OPTION value=Georgia>Georgia</OPTION>
<OPTION value=Impact>Impact</OPTION>
<OPTION value=Tahoma>Tahoma</OPTION>
<OPTION value="Times New Roman" >Times New Roman</OPTION>
<OPTION value="Trebuchet MS">Trebuchet MS</OPTION>
<OPTION value="Script MT Bold">Script MT Bold</OPTION>
<OPTION value=Stencil>Stencil</OPTION>
<OPTION value=Verdana>Verdana</OPTION>
<OPTION value="Lucida Console">Lucida Console</OPTION>
</SELECT>&nbsp;&nbsp;&nbsp;&nbsp;
字体大小：<select  onChange="if(this.options[this.selectedIndex].value!=''){showsize(this.options[this.selectedIndex].value);this.options[0].selected=true;}else {this.selectedIndex=0;}">
	<option value="1">1</option>
	<option value="2">2</option>
	<option value="3" selected>3</option>
	<option value="4">4</option>
</select>
&nbsp;&nbsp;&nbsp;<font face="宋体" color=#333333>颜色：</font>
<SELECT onchange="if(this.options[this.selectedIndex].value!=''){showcolor(this.options[this.selectedIndex].value);this.options[0].selected=true;}else {this.selectedIndex=0;}" > 
<option style="background-color:#F0F8FF;color: #F0F8FF" value="#F0F8FF">#F0F8FF</option>
<option style="background-color:#FAEBD7;color: #FAEBD7" value="#FAEBD7">#FAEBD7</option>
<option style="background-color:#00FFFF;color: #00FFFF" value="#00FFFF">#00FFFF</option>
<option style="background-color:#7FFFD4;color: #7FFFD4" value="#7FFFD4">#7FFFD4</option>
<option style="background-color:#F0FFFF;color: #F0FFFF" value="#F0FFFF">#F0FFFF</option>
<option style="background-color:#F5F5DC;color: #F5F5DC" value="#F5F5DC">#F5F5DC</option>
<option style="background-color:#FFE4C4;color: #FFE4C4" value="#FFE4C4">#FFE4C4</option>
<option style="background-color:#000000;color: #000000" value="#000000">#000000</option>
<option style="background-color:#FFEBCD;color: #FFEBCD" value="#FFEBCD">#FFEBCD</option>
<option style="background-color:#0000FF;color: #0000FF" value="#0000FF">#0000FF</option>
<option style="background-color:#8A2BE2;color: #8A2BE2" value="#8A2BE2">#8A2BE2</option>
<option style="background-color:#A52A2A;color: #A52A2A" value="#A52A2A">#A52A2A</option>
<option style="background-color:#DEB887;color: #DEB887" value="#DEB887">#DEB887</option>
<option style="background-color:#5F9EA0;color: #5F9EA0" value="#5F9EA0">#5F9EA0</option>
<option style="background-color:#7FFF00;color: #7FFF00" value="#7FFF00">#7FFF00</option>
<option style="background-color:#D2691E;color: #D2691E" value="#D2691E">#D2691E</option>
<option style="background-color:#FF7F50;color: #FF7F50" value="#FF7F50">#FF7F50</option>
<option style="background-color:#6495ED;color: #6495ED" value="#6495ED" selected>#6495ED</option>
<option style="background-color:#FFF8DC;color: #FFF8DC" value="#FFF8DC">#FFF8DC</option>
<option style="background-color:#DC143C;color: #DC143C" value="#DC143C">#DC143C</option>
<option style="background-color:#00FFFF;color: #00FFFF" value="#00FFFF">#00FFFF</option>
<option style="background-color:#00008B;color: #00008B" value="#00008B">#00008B</option>
<option style="background-color:#008B8B;color: #008B8B" value="#008B8B">#008B8B</option>
<option style="background-color:#B8860B;color: #B8860B" value="#B8860B">#B8860B</option>
<option style="background-color:#A9A9A9;color: #A9A9A9" value="#A9A9A9">#A9A9A9</option>
<option style="background-color:#006400;color: #006400" value="#006400">#006400</option>
<option style="background-color:#BDB76B;color: #BDB76B" value="#BDB76B">#BDB76B</option>
<option style="background-color:#8B008B;color: #8B008B" value="#8B008B">#8B008B</option>
<option style="background-color:#556B2F;color: #556B2F" value="#556B2F">#556B2F</option>
<option style="background-color:#FF8C00;color: #FF8C00" value="#FF8C00">#FF8C00</option>
<option style="background-color:#9932CC;color: #9932CC" value="#9932CC">#9932CC</option>
<option style="background-color:#8B0000;color: #8B0000" value="#8B0000">#8B0000</option>
<option style="background-color:#E9967A;color: #E9967A" value="#E9967A">#E9967A</option>
<option style="background-color:#8FBC8F;color: #8FBC8F" value="#8FBC8F">#8FBC8F</option>
<option style="background-color:#483D8B;color: #483D8B" value="#483D8B">#483D8B</option>
<option style="background-color:#2F4F4F;color: #2F4F4F" value="#2F4F4F">#2F4F4F</option>
<option style="background-color:#00CED1;color: #00CED1" value="#00CED1">#00CED1</option>
<option style="background-color:#9400D3;color: #9400D3" value="#9400D3">#9400D3</option>
<option style="background-color:#FF1493;color: #FF1493" value="#FF1493">#FF1493</option>
<option style="background-color:#00BFFF;color: #00BFFF" value="#00BFFF">#00BFFF</option>
<option style="background-color:#696969;color: #696969" value="#696969">#696969</option>
<option style="background-color:#1E90FF;color: #1E90FF" value="#1E90FF">#1E90FF</option>
<option style="background-color:#B22222;color: #B22222" value="#B22222">#B22222</option>
<option style="background-color:#FFFAF0;color: #FFFAF0" value="#FFFAF0">#FFFAF0</option>
<option style="background-color:#228B22;color: #228B22" value="#228B22">#228B22</option>
<option style="background-color:#FF00FF;color: #FF00FF" value="#FF00FF">#FF00FF</option>
<option style="background-color:#DCDCDC;color: #DCDCDC" value="#DCDCDC">#DCDCDC</option>
<option style="background-color:#F8F8FF;color: #F8F8FF" value="#F8F8FF">#F8F8FF</option>
<option style="background-color:#FFD700;color: #FFD700" value="#FFD700">#FFD700</option>
<option style="background-color:#DAA520;color: #DAA520" value="#DAA520">#DAA520</option>
<option style="background-color:#808080;color: #808080" value="#808080">#808080</option>
<option style="background-color:#008000;color: #008000" value="#008000">#008000</option>
<option style="background-color:#ADFF2F;color: #ADFF2F" value="#ADFF2F">#ADFF2F</option>
<option style="background-color:#F0FFF0;color: #F0FFF0" value="#F0FFF0">#F0FFF0</option>
<option style="background-color:#FF69B4;color: #FF69B4" value="#FF69B4">#FF69B4</option>
<option style="background-color:#CD5C5C;color: #CD5C5C" value="#CD5C5C">#CD5C5C</option>
<option style="background-color:#4B0082;color: #4B0082" value="#4B0082">#4B0082</option>
<option style="background-color:#FFFFF0;color: #FFFFF0" value="#FFFFF0">#FFFFF0</option>
<option style="background-color:#F0E68C;color: #F0E68C" value="#F0E68C">#F0E68C</option>
<option style="background-color:#E6E6FA;color: #E6E6FA" value="#E6E6FA">#E6E6FA</option>
<option style="background-color:#FFF0F5;color: #FFF0F5" value="#FFF0F5">#FFF0F5</option>
<option style="background-color:#7CFC00;color: #7CFC00" value="#7CFC00">#7CFC00</option>
<option style="background-color:#FFFACD;color: #FFFACD" value="#FFFACD">#FFFACD</option>
<option style="background-color:#ADD8E6;color: #ADD8E6" value="#ADD8E6">#ADD8E6</option>
<option style="background-color:#F08080;color: #F08080" value="#F08080">#F08080</option>
<option style="background-color:#E0FFFF;color: #E0FFFF" value="#E0FFFF">#E0FFFF</option>
<option style="background-color:#FAFAD2;color: #FAFAD2" value="#FAFAD2">#FAFAD2</option>
<option style="background-color:#90EE90;color: #90EE90" value="#90EE90">#90EE90</option>
<option style="background-color:#D3D3D3;color: #D3D3D3" value="#D3D3D3">#D3D3D3</option>
<option style="background-color:#FFB6C1;color: #FFB6C1" value="#FFB6C1">#FFB6C1</option>
<option style="background-color:#FFA07A;color: #FFA07A" value="#FFA07A">#FFA07A</option>
<option style="background-color:#20B2AA;color: #20B2AA" value="#20B2AA">#20B2AA</option>
<option style="background-color:#87CEFA;color: #87CEFA" value="#87CEFA">#87CEFA</option>
<option style="background-color:#778899;color: #778899" value="#778899">#778899</option>
<option style="background-color:#B0C4DE;color: #B0C4DE" value="#B0C4DE">#B0C4DE</option>
<option style="background-color:#FFFFE0;color: #FFFFE0" value="#FFFFE0">#FFFFE0</option>
<option style="background-color:#00FF00;color: #00FF00" value="#00FF00">#00FF00</option>
<option style="background-color:#32CD32;color: #32CD32" value="#32CD32">#32CD32</option>
<option style="background-color:#FAF0E6;color: #FAF0E6" value="#FAF0E6">#FAF0E6</option>
<option style="background-color:#FF00FF;color: #FF00FF" value="#FF00FF">#FF00FF</option>
<option style="background-color:#800000;color: #800000" value="#800000">#800000</option>
<option style="background-color:#66CDAA;color: #66CDAA" value="#66CDAA">#66CDAA</option>
<option style="background-color:#0000CD;color: #0000CD" value="#0000CD">#0000CD</option>
<option style="background-color:#BA55D3;color: #BA55D3" value="#BA55D3">#BA55D3</option>
<option style="background-color:#9370DB;color: #9370DB" value="#9370DB">#9370DB</option>
<option style="background-color:#3CB371;color: #3CB371" value="#3CB371">#3CB371</option>
<option style="background-color:#7B68EE;color: #7B68EE" value="#7B68EE">#7B68EE</option>
<option style="background-color:#00FA9A;color: #00FA9A" value="#00FA9A">#00FA9A</option>
<option style="background-color:#48D1CC;color: #48D1CC" value="#48D1CC">#48D1CC</option>
<option style="background-color:#C71585;color: #C71585" value="#C71585">#C71585</option>
<option style="background-color:#191970;color: #191970" value="#191970">#191970</option>
<option style="background-color:#F5FFFA;color: #F5FFFA" value="#F5FFFA">#F5FFFA</option>
<option style="background-color:#FFE4E1;color: #FFE4E1" value="#FFE4E1">#FFE4E1</option>
<option style="background-color:#FFE4B5;color: #FFE4B5" value="#FFE4B5">#FFE4B5</option>
<option style="background-color:#FFDEAD;color: #FFDEAD" value="#FFDEAD">#FFDEAD</option>
<option style="background-color:#000080;color: #000080" value="#000080">#000080</option>
<option style="background-color:#FDF5E6;color: #FDF5E6" value="#FDF5E6">#FDF5E6</option>
<option style="background-color:#808000;color: #808000" value="#808000">#808000</option>
<option style="background-color:#6B8E23;color: #6B8E23" value="#6B8E23">#6B8E23</option>
<option style="background-color:#FFA500;color: #FFA500" value="#FFA500">#FFA500</option>
<option style="background-color:#FF4500;color: #FF4500" value="#FF4500">#FF4500</option>
<option style="background-color:#DA70D6;color: #DA70D6" value="#DA70D6">#DA70D6</option>
<option style="background-color:#EEE8AA;color: #EEE8AA" value="#EEE8AA">#EEE8AA</option>
<option style="background-color:#98FB98;color: #98FB98" value="#98FB98">#98FB98</option>
<option style="background-color:#AFEEEE;color: #AFEEEE" value="#AFEEEE">#AFEEEE</option>
<option style="background-color:#DB7093;color: #DB7093" value="#DB7093">#DB7093</option>
<option style="background-color:#FFEFD5;color: #FFEFD5" value="#FFEFD5">#FFEFD5</option>
<option style="background-color:#FFDAB9;color: #FFDAB9" value="#FFDAB9">#FFDAB9</option>
<option style="background-color:#CD853F;color: #CD853F" value="#CD853F">#CD853F</option>
<option style="background-color:#FFC0CB;color: #FFC0CB" value="#FFC0CB">#FFC0CB</option>
<option style="background-color:#DDA0DD;color: #DDA0DD" value="#DDA0DD">#DDA0DD</option>
<option style="background-color:#B0E0E6;color: #B0E0E6" value="#B0E0E6">#B0E0E6</option>
<option style="background-color:#800080;color: #800080" value="#800080">#800080</option>
<option style="background-color:#FF0000;color: #FF0000" value="#FF0000">#FF0000</option>
<option style="background-color:#BC8F8F;color: #BC8F8F" value="#BC8F8F">#BC8F8F</option>
<option style="background-color:#4169E1;color: #4169E1" value="#4169E1">#4169E1</option>
<option style="background-color:#8B4513;color: #8B4513" value="#8B4513">#8B4513</option>
<option style="background-color:#FA8072;color: #FA8072" value="#FA8072">#FA8072</option>
<option style="background-color:#F4A460;color: #F4A460" value="#F4A460">#F4A460</option>
<option style="background-color:#2E8B57;color: #2E8B57" value="#2E8B57">#2E8B57</option>
<option style="background-color:#FFF5EE;color: #FFF5EE" value="#FFF5EE">#FFF5EE</option>
<option style="background-color:#A0522D;color: #A0522D" value="#A0522D">#A0522D</option>
<option style="background-color:#C0C0C0;color: #C0C0C0" value="#C0C0C0">#C0C0C0</option>
<option style="background-color:#87CEEB;color: #87CEEB" value="#87CEEB">#87CEEB</option>
<option style="background-color:#6A5ACD;color: #6A5ACD" value="#6A5ACD">#6A5ACD</option>
<option style="background-color:#708090;color: #708090" value="#708090">#708090</option>
<option style="background-color:#FFFAFA;color: #FFFAFA" value="#FFFAFA">#FFFAFA</option>
<option style="background-color:#00FF7F;color: #00FF7F" value="#00FF7F">#00FF7F</option>
<option style="background-color:#4682B4;color: #4682B4" value="#4682B4">#4682B4</option>
<option style="background-color:#D2B48C;color: #D2B48C" value="#D2B48C">#D2B48C</option>
<option style="background-color:#008080;color: #008080" value="#008080">#008080</option>
<option style="background-color:#D8BFD8;color: #D8BFD8" value="#D8BFD8">#D8BFD8</option>
<option style="background-color:#FF6347;color: #FF6347" value="#FF6347">#FF6347</option>
<option style="background-color:#40E0D0;color: #40E0D0" value="#40E0D0">#40E0D0</option>
<option style="background-color:#EE82EE;color: #EE82EE" value="#EE82EE">#EE82EE</option>
<option style="background-color:#F5DEB3;color: #F5DEB3" value="#F5DEB3">#F5DEB3</option>
<option style="background-color:#FFFFFF;color: #FFFFFF" value="#FFFFFF">#FFFFFF</option>
<option style="background-color:#F5F5F5;color: #F5F5F5" value="#F5F5F5">#F5F5F5</option>
<option style="background-color:#FFFF00;color: #FFFF00" value="#FFFF00">#FFFF00</option>
<option style="background-color:#9ACD32;color: #9ACD32" value="#9ACD32">#9ACD32</option>
</SELECT>
<br>
<img onclick=Cbold() src="pic/bold.gif" width="22" height="22" alt="粗体" border="0">
<img onclick=Citalic() src="pic/italicize.gif" width="23" height="22" alt="斜体" border="0">
<img onclick=Cunder() src="pic/underline.gif" width="23" height="22" alt="下划线" border="0">
<img onclick=Ccenter() src="pic/center.gif" width="22" height="22" alt="居中" border="0">
<img onclick=Curl() src="pic/url1.gif" width="22" height="22" alt="超级连接" border="0">
<img onclick=Cemail() src="pic/email1.gif" width="23" height="22" alt="Email连接" border="0">
<img onclick=Cimage() src="pic/image.gif" width="23" height="22" alt="图片" border="0">
<img onclick=Cswf() src="pic/swf.gif" width="23" height="22" alt="Flash图片" border="0">
<img onclick=Cdir() src="pic/Shockwave.gif" width="23" height="22" alt="Shockwave文件" border="0">
<img onclick=Crm() src="pic/rm.gif" width="23" height="22" alt="realplay视频文件" border="0">
<img onclick=Cwmv() src="pic/mp.gif" width="23" height="22" alt="Media Player视频文件" border="0">
<img onclick=Cmov() src="pic/qt.gif" width="23" height="22" alt="QuickTime视频文件" border="0">

<img onclick=Cquote() src="pic/quote1.gif" width="23" height="22" alt="引用" border="0">
<IMG onclick=Cfly() height=22 alt=飞行字 src="pic/fly.gif" width=23 border=0>
<IMG onclick=Cmarquee() height=22 alt=移动字 src="pic/move.gif" width=23 border=0>
<IMG onclick=Cguang() height=22 alt=发光字 src="pic/glow.gif" width=23 border=0>
<IMG onclick=Cying() height=22 alt=阴影字 src="pic/shadow.gif" width=23 border=0>
<IMG onclick=openScript('smiles.php',500,600) src="pic/ubb_smilie.gif" border=0 alt=查看更多的心情图标>
<img onclick=Csound() src="pic/csound.gif" width="23" height="22" alt="背景音乐" border="0">

<textarea class=smallarea cols=95 name=Content rows=12 wrap=VIRTUAL title="可以使用Ctrl+Enter直接提交贴子" class=FormClass onkeydown=ctlent()></textarea>
          </td>
        </tr>

		<tr>
                <td class=tablebody1 valign=top colspan=2 style="table-layout:fixed; word-break:break-all"><b>点击表情图即可在帖子中加入相应的表情</B><br>&nbsp;
<?php 
  for ($i=0; $i<=12; $i=$i+1) {
    if (strlen($i)==1)   {
	    $ii="0".$i;
    } else  {
		$ii=$i;
	} 
	print "<img src=\"".$Forum_info[10].$Forum_emot[$i]."\" border=0 onclick=\"insertsmilie('[em".$ii."]')\" style=\"CURSOR: hand\">&nbsp;";

} 

?>
    		</td>
                </tr>
                <tr>
                <td valign=top class=tablebody1><b>选项</b></td>
                <td valign=middle class=tablebody1>&nbsp;<select name="signature">
<?php
		if ($currentuser["signature"] == 0)
		{
?>
<option value="0" selected="selected">不使用签名档</option>
<?php
		}
		else
		{
?>
<option value="0">不使用签名档</option>
<?php
			for ($i = 1; $i < 6; $i++)
			{
				if ($currentuser["signature"] == $i)
				{
?>
<option value="<?php echo $i; ?>" selected="selected">第 <?php echo $i; ?> 个</option>
<?php
				}
				else
				{
?>
<option value="<?php echo $i; ?>">第 <?php echo $i; ?> 个</option>
<?php
				}
			}
		}
?>
</select>
 [<a target="_balnk" href="bbssig.php">查看签名档</a>]<br>
 <?php
	if (bbs_is_outgo_board($boardArr) ) {
		echo "<input type=\"checkbox\" name=\"outgo\" value=\"1\" />转信<br />";
	}
		if (bbs_is_anony_board($boardArr) ) {
		echo "<input type=\"checkbox\" name=\"annonymous\" value=\"1\" />匿名<br />";
	}
?>
                <input type=checkbox name=emailflag value=yes>有回复时使用邮件通知您？</font>
<BR><BR></td>
                </tr><tr>
                <td valign=middle colspan=2 align=center class=tablebody2>
                <input type=Submit value='发 表' name=Submit> &nbsp; <input type=button value='预 览' name=Button onclick=gopreview()>&nbsp;
<input type=reset name=Submit2 value='清 除'>
                </td></form></tr>
      </table>
</form>
<form name=preview action=preview.php?boardid=<?php echo $Boardid; ?> method=post target=preview_page>
<input type=hidden name=title value=><input type=hidden name=body value=>
</form>
<?php
}
?>