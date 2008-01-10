<?php

	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_READING);
	assert_login();

		if(isset($_POST["board"]))
			$board = $_POST["board"];
		else if(isset($_GET["board"]))
			$board = $_GET["board"];
		else
			html_error_quit("参数错误1");

		if(isset($_POST["submit"])){
			if(isset($_POST["type"]))
				$type = $_POST["type"];
			else
				html_error_quit("参数错误2");

			settype($type,"integer");
			if($type < 1 || $type > 5)
				html_error_quit("参数错误3");

			if(isset($_POST["title"]))
				$title = $_POST["title"];
			else
				html_error_quit("参数错误5");

			if(isset($_POST["desp"]))
				$ball_desp = $_POST["desp"];
			else
				$ball_desp = "本投票暂时没有描述";

			/*if(isset($_POST["numlogin"])){
				$numlogin = $_POST["numlogin"];
				settype($numlogin,"integer");
				if($numlogin < 0)
					$numlogin = 0;
			}
			else*/
				$numlogin = 0;

			/*if(isset($_POST["numpost"])){
				$numpost = $_POST["numpost"];
				settype($numpost,"integer");
				if($numpost < 0)
					$numpost = 0;
			}
			else*/
				$numpost = 0;

			/*if(isset($_POST["numstay"])){
				$numstay = $_POST["numstay"];
				settype($numstay,"integer");
				if($numstay < 0)
					$numstay = 0;
			}
			else*/
				$numstay = 0;

			/*if(isset($_POST["numday"])){
				$numday = $_POST["numday"];
				settype($numday,"integer");
				if($numday < 0)
					$numday = 0;
			}
			else*/
				$numday = 0;

			if(isset($_POST["maxdays"])){
				$ball_maxdays = $_POST["maxdays"];
				settype($ball_maxdays,"integer");
				if($ball_maxdays <=0)
					$ball_maxdays = 1;
			}
			else
				$ball_maxdays = 1;

			if($title == "")
				html_error_quit("参数错误6");

			$items = array();
			for($i = 0; $i < 31; $i ++)
				$items[$i] = "";
			$ball_maxtkt = 1;
			$ball_totalitems = 3;

			if($type == 1){
				//是非
			}else if($type == 2 || $type == 3){
				//单选,复选

				if(isset($_POST["maxitems"]))
					$ball_totalitems = $_POST["maxitems"];
				else
					html_error_quit("参数错误5");

				settype($ball_totalitems,"integer");
				if( $ball_totalitems <= 0 || $ball_totalitems > 32)
					html_error_quit("参数错误5");

				if($type == 2)
					$ball_maxtkt = 1;
				else{
					if(isset($_POST["maxtkt"]))
						$ball_maxtkt = $_POST["maxtkt"];
					else
						html_error_quit("错误的参数4");

					settype($ball_maxtkt,"integer");
					if($ball_maxtkt <= 0)
						$ball_maxtkt = 1;
				}

				for($i = 0; $i < $ball_totalitems; $i ++){
					$itemstr = "ITEM".($i+1);
					if(isset($_POST[$itemstr])){
						$items[$i] = $_POST[$itemstr];
					}
				}
			}else if($type == 4){
				if(isset($_POST["maxnumin"]))
					$ball_maxtkt = $_POST["maxnumin"];
				else
					html_error_quit("错误的参数6");

				settype($ball_maxtkt,"integer");
				if($ball_maxtkt <= 0)
					$ball_maxtkt = 100;

			}

			$ret = bbs_start_vote($board, $type, $numlogin, $numpost, $numstay, $numday, $title, $ball_desp, $ball_maxdays, $ball_maxtkt, $ball_totalitems, $items[0], $items[1], $items[2], $items[3], $items[4], $items[5], $items[6], $items[7], $items[8], $items[9]);

			if($ret <= 0)
				html_error_quit("开投票错误.".$ret);
			else{
				html_success_quit("开投票成功<br/><a href='bbsdoc.php?board=" . $board . "'>返回本讨论区</a>");
			}
		}
		
	$usernum = $currentuser["index"];
	$brdarr = array();
	$brdnum = bbs_getboard($board, $brdarr);
	if ($brdnum == 0)
		html_error_quit("错误的讨论区");
	if (!bbs_is_bm($brdnum,$usernum))
		html_error_quit("你不是版主");
	bbs_board_nav_header($brdarr, "新开投票");
?>
<script type="text/javascript">
<!--//
var maxitemnum=10;
var defaultitem=3;
function doGenerate(){
	var oSelectType=document.getElementById("oType");
	var type=oSelectType.value;
	var targetDiv=document.getElementById("oDiv");
	var content="";
	var i;
	if  ( (type=="2")  || (type=="3") ){
		content+="选项个数:<select name=\"maxitems\" class=\"input\"  style=\"WIDTH: 60px\" id=\"oItemNum\" onChange=\"doGenerateItem();\">";
		for (i=1;i<=maxitemnum;i++){
			if (i==defaultitem){
				content+="<option value=\""+i+"\" selected>"+i+"</option>";
			}else {
				content+="<option value=\""+i+"\">"+i+"</option>";
			}
		}
		content+="</select><BR>";
		if (type=="3") {
			content+="用户最多可以选择个数:<input type=\"text\" name=\"maxtkt\" value=\"1\"><br>"
		}
		targetDiv.innerHTML=content;
		doGenerateItem();
	}else {
		if (type=="4") {
			content="最大数字限额:<input type=\"text\" name=\"maxnumin\" value=\"1\"><br>";
			
		} else {
			content="<BR>";
		}
		targetDiv.innerHTML=content;
		clearItem();
	}

	return;
}


function doGenerateItem(){
	var objItemNum=document.getElementById("oItemNum");
	var itemNum=parseInt(objItemNum.value);
	var targetDiv=document.getElementById("oDivItems");
	var content="";
	var i;
	for (i=1;i<=itemNum;i++){
			content+=i+":<input type=\"text\" name=\"ITEM"+i+"\" value=\"\"><br>";
	}
	targetDiv.innerHTML=content;
	return;
}

function clearItem(){
	var oTargetDiv=document.getElementById("oDivItems");
	oTargetDiv.innerHTML="<BR>";
}

//-->
</script>
<form action="bbsmvote.php" method="post" class="large">
<input type="hidden" name="board" value="<?php echo $board;?>">
选择投票种类:
<select name="type" class="input"  style="WIDTH: 60px" id="oType" onChange="doGenerate();">
<option value="1">是非</option>
<option value="2">单选</option>
<option value="3">复选</option>
<option value="4">数字</option>
<option value="5">问答</option>
</select>
<hr class="default">
投票标题:<input type="text" name="title" value=""><br>
投票描述:<textarea name="desp" rows=10 cols=80 wrap="physical"></textarea><br>
投票所须天数:<input type="text" name="maxdays" value="1"><br>
<hr class="default">
<div id="oDiv">
</div>
<br>
<div id="oDivItems">
</div>

<?php /*
<hr class="default">
限制投票资格选项:<br>
上站次数限制:<input type="text" name="numlogin" value="0"><br>
发表文章数目限制:<input type="text" name="numpost" value="0"><br>
上站总时数限制:<input type="text" name="numstay" value="0"><br>
帐号注册时间限制:<input type="text" name="numday" value="0"><br>
<hr class="default">
*/ ?>
<center>
<input type="submit" name="submit" value="确定">
[<a href="javascript:history.go(-1)">快速返回</a>]
</form>
</center>
