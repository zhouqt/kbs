<?php

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");

setStat("∞Ê√ÊÀ—À˜");

show_nav();

showUserMailBoxOrBR();

head_var();

main();

show_footer();

/*
function showBoards($group, &$allBoards) {
	global $yank;
	$boards = bbs_getboards("*", $group, $yank | 2);
	if ($boards != FALSE) {
		$brd_desc = $boards["DESC"];
		$brd_name = $boards["NAME"];
		$brd_flag = $boards["FLAG"];
		$brd_bid = $boards["BID"];
		$rows = sizeof($brd_desc);
		for ($t = 0; $t < $rows; $t++)	{
			$allBoards[$brd_name[$t]] = $brd_desc[$t];
			if ($brd_flag[$t] & BBS_BOARD_GROUP) {
				showBoards($brd_bid[$t], $allBoards);
			}			
		}
	}
}
*/

function getAllBoards() {
	global $yank;
	$allBoards = array();
	$boards = bbs_getboards("*", $group, $yank | 2 | 4);
	if ($boards != FALSE) {
		$brd_desc = $boards["DESC"];
		$brd_name = $boards["NAME"];
		$brd_flag = $boards["FLAG"];
		$brd_bid = $boards["BID"];
		$rows = sizeof($brd_desc);
		for ($t = 0; $t < $rows; $t++)	{
			$allBoards[$brd_name[$t]] = $brd_desc[$t];
		}
	}
	return $allBoards;
}

function main(){
	/*
	$allBoards = array();
	showBoards(0, $allBoards);
	*/
	$allBoards = getAllBoards();
	uksort($allBoards, "strcasecmp");
	$str = "";
	while (list($key, $val) = each($allBoards)) {
		$str .= ",'$key'";
	}

?>
<script language="JavaScript">
<!--
	var allBoards = new Array(<?php echo substr($str, 1); ?>);
	var kxn = new Array();
	var kxn_now;
	function getCompletion() {
		str = getRawObject("board").value.toLowerCase();
		len = str.length;
		if (str == "") return "";
		re = "";
		kxn = new Array();
		for(i=0;i<allBoards.length;i++) {
			if (allBoards[i].toLowerCase().indexOf(str) == 0) {
				re += "<tr><td class=\"kxn1\" id=\"b" + kxn.length + "\"><b>" + allBoards[i].substr(0,len) + "</b>" + allBoards[i].substr(len) + "</td></tr>";
				kxn[kxn.length] = i;
			}
		}
		kxn_now = -1;
		if (kxn.length == 0) return "";
		return "<table id='submenu' cellspacing=0 cellpadding=3 class=\"TableBorder1 sb\">" + re + "</table>";
	}
	function s(i) {
		getRawObject("board").value = allBoards[kxn[i]];
		kxn_now = -1;
	}
	function kd(evt) {
		evt = (evt) ? evt : ((window.event) ? event : null);
		kc = evt.keyCode;
		if (kxn_now >= 0 && kc == 13) {
			getRawObject("board").value = allBoards[kxn[kxn_now]];
			kxn_now = -1;
			//evt.cancelBubble = true;
			//evt.returnValue = false;
		} else if (kc == 9) { //ESC
/*
			oDiv=getRawObject("menuDiv");
			hide(oDiv);
			isvisible = false;
			kxn_now = -1;
			evt.cancelBubble = true;
			evt.returnValue = false;
*/
		}
	}
	function ku(evt) {
		evt = (evt) ? evt : ((window.event) ? event : null);
		if (kxn.length > 0) {
			kc = evt.keyCode;
			if (kc == 40 || kc == 38) {
				obj = getRawObject("b" + kxn_now);
				if (obj != null) obj.className = "kxn1";
				if (kc == 40) {
					kxn_now++;
					if (kxn_now >= kxn.length) kxn_now = 0;
				} else {
					kxn_now--;
					if (kxn_now < 0) kxn_now = kxn.length - 1;
				}
				obj = getRawObject("b" + kxn_now);
				if (obj != null) obj.className = "kxn2";
				return;
			}
		}
		kxn_now = -1;
		sm(evt);
	}
	function kp(evt) {
	}
	function sm(evt) {
		str = getCompletion();
		if (str == "") {
			//HideMenu(evt);
			oDiv=getRawObject("menuDiv");
			hide(oDiv);
			isvisible = false;
		} else {
			ShowMenu_Internal(str, 100, evt, false);
		}
	}
	function os() {
		if (kxn_now == -1) {
			window.document.location.href = "searchboard.php?board=" + escape(getRawObject("board").value);
		}
		return false;
	}
//-->
</script>
<style type="text/css">
.sb {
	border: 1pt solid #718BD6;
}
.kxn1 {
	font-family: 'Courier New';
	font-size: 9pt;
	background-color: white;
}
.kxn2 {
	font-family: 'Courier New';
	font-size: 9pt;
	background-color: lightblue;
}
</style>
<form method="GET" action="searchboard.php" onsubmit="return os();">
<table align="center"><tr><td align="left">
 ‰»Î∞Ê√˚£¨∫’∫’£∫<br>
<input id="board" name="board<?php echo time(); ?>" value="" size="15" class="sb"
	onfocus="sm(event)"
	onkeydown="kd(event)"
	onkeyup="ku(event)"
	onkeypress="kp(event)"
	onblur="HideMenu(event)"
/>
</td></tr></table>
</form>
<?php
}
?>
