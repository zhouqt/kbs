<?php
    require("www2-admin.php");

    admin_check("editbrd");

    if(isset($_POST["oldfilename"])) {
        $boardname = $_POST["oldfilename"];
        $filename = $_POST["filename"];
        $bm = $_POST["bm"];
        $chinesebname = $_POST["title"];
        $secnum = $_POST["secnum"];
        $btype = $_POST["btype"];
        $innflag = $_POST["innflag"];
        $title = sprintf("%-1.1s[%-4.4s]%-6.6s%s", constant("BBS_SECCODE{$secnum}"), $btype, $innflag, $chinesebname);
        $des = $_POST["des"];
        $flag = $_POST["flag"];
        if(@$_POST["anony"] == "o")
            $flag |= BBS_BOARD_ANONY;
        else
            $flag &= ~BBS_BOARD_ANONY;
        if(@$_POST["notjunk"] == "o")
            $flag &= ~BBS_BOARD_JUNK;
        else
            $flag |= BBS_BOARD_JUNK;
        if(@$_POST["notpoststat"] == "o")
            $flag &= ~BBS_BOARD_POSTSTAT;
        else
            $flag |= BBS_BOARD_POSTSTAT;
        if(@$_POST["group"] == "o")
            $flag |= BBS_BOARD_GROUP;
        else
            $flag &= ~BBS_BOARD_GROUP;
        if(@$_POST["outflag"] == "o")
            $flag |= BBS_BOARD_OUTFLAG;
        else
            $flag &= ~BBS_BOARD_OUTFLAG;
        if(@$_POST["attach"] == "o")
            $flag |= BBS_BOARD_ATTACH;
        else
            $flag &= ~BBS_BOARD_ATTACH;
        if(@$_POST["emailpost"] == "o")
            $flag |= BBS_BOARD_EMAILPOST;
        else
            $flag &= ~BBS_BOARD_EMAILPOST;
        if(@$_POST["noreply"] == "o")
            $flag |= BBS_BOARD_NOREPLY;
        else
            $flag &= ~BBS_BOARD_NOREPLY;
        if(@$_POST["clubread"] == "o")
            $flag |= BBS_BOARD_CLUB_READ;
        else
            $flag &= ~BBS_BOARD_CLUB_READ;
        if(@$_POST["clubwrite"] == "o")
            $flag |= BBS_BOARD_CLUB_WRITE;
        else
            $flag &= ~BBS_BOARD_CLUB_WRITE;
        if(@$_POST["clubhide"] == "o")
            $flag |= BBS_BOARD_CLUB_HIDE;
        else
            $flag &= ~BBS_BOARD_CLUB_HIDE;
        $parentbname = $_POST["parentb"];
        $annpath_section = $_POST["annpath"];
        $level = bbs_admin_resolvepermtable("level", BBS_NUMPERMS);
        $title_level = $_POST["title_level"];
        $ret = bbs_admin_setboardparam($boardname, $filename, $bm, $title, $des, $flag, $parentbname, $annpath_section, $level, $title_level);
        switch($ret) {
            case 0:
                html_success_quit("版面属性修改成功。", array("<a href=\"admeditbrd.php?board={$filename}\">请点击这里返回</a>"));
                break;
            case -1:
                html_error_quit("您所要修改的版面不存在。");
                break;
            case -2:
                html_error_quit("无法更改版面名称，同名版面已经存在。");
                break;
            case -3:
                html_error_quit("版面名称包含不符合规定的字符。");
                break;
            case -4:
                html_error_quit("您在“所属目录”中填写的版面不存在。");
                break;
            case -5:
                html_error_quit("您在“所属目录”中填写的版面不是一个目录讨论区。");
                break;
            case -6:
                html_error_quit("精华区分类不存在。");
                break;
            case -7:
                html_error_quit("身份不存在。");
                break;
            default:
        }
    }

    if(isset($_GET["board"]))
        $boardname = $_GET["board"];
    else if(isset($_POST["board"]))
        $boardname = $_POST["board"];
    else
        $boardname = "";

    if($boardname != "") {
        $boardparams = array();
        $ret = bbs_admin_getboardparam($boardname, $boardparams);
        switch($ret) {
        case -1:
            html_error_quit("不存在的版面。");
            break;
        case -2:
            html_error_quit("无法读取版面属性。");
            break;
        case -3:
            html_error_quit("初始化数组失败，请联系技术人员。");
            break;
        case 0:
            $bid = $boardparams["BID"];
            $filename = $boardparams["FILENAME"];
            $clubnum = $boardparams["CLUBNUM"];
            $bm = $boardparams["BM"];
            $title = substr($boardparams["TITLE"], 13, 256);
            $secnum = substr($boardparams["TITLE"], 0, 1);
            $btype = substr($boardparams["TITLE"], 2, 4);
            $innflag = substr($boardparams["TITLE"], 7, 6);
            $des = $boardparams["DES"];
            $flag = $boardparams["FLAG"];
            $anony = ($boardparams["FLAG"] & BBS_BOARD_ANNONY) ? " checked" : "";
            $notjunk = ($boardparams["FLAG"] & BBS_BOARD_JUNK) ? "" : " checked";
            $notpoststat = ($boardparams["FLAG"] & BBS_BOARD_POSTSTAT) ? "" : " checked";
            $group = ($boardparams["FLAG"] & BBS_BOARD_GROUP) ? " checked" : "";
            $parentb = $boardparams["GROUP"];
            if($parentb != "") {
                $parentbarr = array();
                $parentbstatus = is_null(bbs_safe_getboard(0, $parentb, $parentbarr)) ? "(异常)" : "";
            }
            else {
                $parentb = "";
                $parentbstatus = "";
            }
            $outflag = ($boardparams["FLAG"] & BBS_BOARD_OUTFLAG) ? " checked" : "";
            $attach = ($boardparams["FLAG"] & BBS_BOARD_ATTACH) ? " checked" : "";
			$emailpost = ($boardparams["FLAG"] & BBS_BOARD_EMAILPOST) ? " checked" : "";
			$noreply = ($boardparams["FLAG"] & BBS_BOARD_NOREPLY) ? " checked" : "";
			$clubread = ($boardparams["FLAG"] & BBS_BOARD_CLUB_READ) ? " checked" : "";
			$clubwrite = ($boardparams["FLAG"] & BBS_BOARD_CLUB_WRITE) ? " checked" : "";
			$clubhide = ($boardparams["FLAG"] & BBS_BOARD_CLUB_HIDE) ? " checked" : "";
			if(($clubread == "") && ($clubwrite == ""))
				$clubhide .= " disabled";
			$annpath = $boardparams["ANNPATH"];
            $annpath_section = $boardparams["ANNPATH_SECTION"];
            $annpath_status = $boardparams["ANNPATH_STATUS"];
            $annpath_status_str = array("有效", "无效", "异常");
            $level = $boardparams["LEVEL"];
            $title_level = $boardparams["TITLELEVEL"];
            break;
        default:
        }
    }
    
    for($i=1; $i<256; $i++) {
        $usertitles[$i-1] = bbs_admin_getusertitle($i);
    }
    
    admin_header("修改版面", "修改讨论区说明与设定");
?>
<script type="text/javascript">
function loadBoardParam() {
    var bname = document.getElementById('board').value;
    location = 'admeditbrd.php?board=' + bname;
    return false;
}
function setinnflag(ifstr) {
    document.getElementById('innflag').value = ifstr;
}
function clubtypeChange() {
	var cr, cw;
	cr = document.getElementById('clubread').checked;
	cw = document.getElementById('clubwrite').checked;
	if(!(cr || cw))
		document.getElementById('clubhide').disabled = true;
	else
		document.getElementById('clubhide').disabled = false;
}
</script>
<form method="post" action="admeditbrd.php" class="medium" onsubmit="return loadBoardParam();">
<fieldset><legend>要修改的版面</legend><div class="inputs">
<label>版面英文名称:</label><input type="text" id="board" name="board" size="20" maxlength="30" value="<?php echo $boardname; ?>">
<input type="submit" value="确定"></div></fieldset></form>
<?php if($boardname != "") { ?>
<form method="post" action="admeditbrd.php" class="medium">
<fieldset><legend>修改版面属性</legend><div class="inputs">
<input type="hidden" name="oldfilename" value="<?php echo $filename; ?>">
<label>讨论区序号:</label><?php echo $bid; ?><br>
<label>俱乐部序号:</label><?php echo $clubnum; ?><br>
<label>讨论区名称:</label><input type="text" name="filename" size="20" maxlength="30" value="<?php echo $filename; ?>"><br>
<label>讨论区管理:</label><input type="text" name="bm" size="30" maxlength="59" value="<?php echo $bm; ?>"><br>
<label>讨论区说明:</label><input type="text" name="title" size="20" maxlength="50" value="<?php echo $title; ?>"><br>
<label>讨论区分区:</label><select name="secnum">
<?php
    for($i=0; $i<BBS_SECNUM; $i++) {
        print("<option value=\"{$i}\"" . ((constant("BBS_SECCODE{$i}")==$secnum)?" selected":""). ">&lt;" . constant("BBS_SECCODE{$i}") . "&gt; " . constant("BBS_SECNAME{$i}_0") . " " . constant("BBS_SECNAME{$i}_1") . "</option>");
    }
?>
</select><br>
<label>讨论区分类:</label>[<input type="text" name="btype" size="4" maxlength="4" value="<?php echo $btype; ?>">]<br>
<label>转信标签:</label><input type="text" id="innflag" name="innflag" size="6" maxlength="6" value="<?php echo $innflag; ?>">
    &lt;<a href="javascript:setinnflag('      ');">无转信</a>&gt;
    &lt;<a href="javascript:setinnflag(' ●   ');">双向转信</a>&gt;
    &lt;<a href="javascript:setinnflag(' ⊙   ');">单向转信</a>&gt;<br>
<label>讨论区描述:</label><input type="text" name="des" size="30" maxlength="194" value="<?php echo $des; ?>"><br>
<input type="hidden" name="flag" value="<?php echo $flag; ?>">
<label>匿名讨论区:</label><input type="checkbox" value="o" name="anony"<?php echo $anony; ?>>用户可以匿名在版面发文。<br>
<label>统计文章数:</label><input type="checkbox" value="o" name="notjunk"<?php echo $notjunk; ?>>在版面发文则用户文章数增加。<br>
<label>统计十大:</label><input type="checkbox" value="o" name="notpoststat"<?php echo $notpoststat; ?>>版面文章参加十大热门话题统计。<br>
<label>目录讨论区:</label><input type="checkbox" value="o" name="group"<?php echo $group; ?>>容纳其他版面的版面。<br>
<label>所属目录:</label><input type="text" name="parentb" value="<?php echo $parentb; ?>"> <?php echo $parentbstatus; ?><br>
<label>向外转信:</label><input type="checkbox" value="o" name="outflag"<?php echo $outflag; ?>>可以向其它站点转信。<br>
<label>上传附件:</label><input type="checkbox" value="o" name="attach"<?php echo $attach; ?>>文章后面可以粘贴附件。<br>
<label>E-mail发文:</label><input type="checkbox" value="o" name="emailpost"<?php echo $emailpost; ?>>可以将接收到email发布到版面。<br>
<label>不可回复:</label><input type="checkbox" value="o" name="noreply"<?php echo $noreply; ?>>不能在版面回复文章。<br>
<label>俱乐部类型:</label><input type="checkbox" id="clubread" value="o" name="clubread" onclick="clubtypeChange();"<?php echo $clubread; ?>>读限制
	<input type="checkbox" id="clubwrite" value="o" name="clubwrite" onclick="clubtypeChange();"<?php echo $clubwrite; ?>>写限制
	<input type="checkbox" id="clubhide" value="o" name="clubhide"<?php echo $clubhide; ?>>隐藏<br>
<label>精华区路径:</label><select name="annpath">
<?php
    for($i=0; $i<BBS_SECNUM; $i++) {
        print("<option value=\"{$i}\"" . (($i==$annpath_section)?" selected":"") . ">[" . constant("BBS_SECCODE{$i}") . "] " . constant("BBS_SECNAME{$i}_0") . " " . constant("BBS_GROUP{$i}") . "</option>");
    }
?>
</select> <?php echo $annpath_status_str[$annpath_status]; ?><br>
<label>权限限制:</label>
<?php
    print("<table align=\"center\">");
    bbs_admin_permtable("level", $level, 0, 16); 
    bbs_admin_permtable("level", $level, 17, BBS_NUMPERMS);
    print("</table>");
?>
<label>身份限制:</label>
<select name="title_level">
<option value="0">[没有限制]</option>
<?php
        for($i=1; $i<256; $i++) {
            if($usertitles[$i-1] != "")
                print("<option value=\"{$i}\"" . (($title_level==$i)?" selected":"") . ">{$usertitles[$i-1]}</option>");
        }
?>
</select><br><br>
<?php // 这个水木特有的积分限制我不管了，要不ecore做做吧。 ?>
<div align="center"><input type="submit" value="修改"> <input type="reset" value="重置"></div>
</div></fieldset></form>
<?php } ?><br>
<?php
    page_footer();
?>
