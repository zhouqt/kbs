<?php
	require("www2-funcs.php");
	login_init();
	toolbox_header("界面修改");

	if (isset($_GET['do'])) {
		$new_wwwparams = @intval($_COOKIE["WWWPARAMS"]);
		if (strcmp($currentuser["userid"], "guest")) {
			bbs_setwwwparameters($new_wwwparams); /* TODO: return value ? */
		}
	}
?>
<script type="text/javascript">
	var settings = {"sizer": 3, "pager": 4, "hot": 5}; /* faint IE5 */
	function setInd(n, v) {
		var ff = getObj(n + 'F');
		var tt = getObj(n + 'T');
		/* some users might not have bold font... that's why I add underline */
		if (ff) {
			ff.style.fontWeight = v ? 'normal' : 'bold';
			ff.style.textDecoration = v ? 'none' : 'underline';
		}
		if (tt) {
			tt.style.fontWeight = v ? 'bold' : 'normal';
			tt.style.textDecoration = v ? 'underline' : 'none';
		}
	}
	function adjSet(n, v) {
		var idx = settings[n];
		if (n == "sizer") {
			getObj("fontSizer").style.display = v ? "block" : "none";
		}
		setInd(n, v);
		var mask = 1 << idx;
		saveParaCookie(v ? mask : 0, mask);
	}
	addBootFn(function() {
		var c = readParaCookie();
		for (var n in settings) {
			var i = settings[n];
			var v = c & (1 << i);
			setInd(n, v);
		}
		var stylenum = (c & 0xF80) >>7;
		getObj('style'+stylenum).checked = true;
	});
	function KCNymsw() {
		alert('还没做这个功能呢... 需要吗？不需要吗？您去 sysop 版喊喊？');
	}
	function pvStyle(cssID) {
		saveParaCookie(cssID << 7, 0xF80);
		var ff = top.window["menu"]; if (ff) ff.resetCss();
		ff = top.window["toogle"]; if (ff) ff.resetCss();
		ff = top.window["f4"]; if (ff) ff.resetCss();
	}
	function chkStyle(cssID) {
		getObj('style' + cssID).checked = true;
		pvStyle(cssID);
	}
</script>
<form action="bbsstyle.php?do" class="small align">
	<fieldset><legend>自定义界面</legend>
		<div class="inputs">
			<label>字体大小:</label>
				<span class="clickable" onclick="sizer(1)">放大</span>
				<span class="clickable" onclick="sizer(-1)">缩小</span>
				<span class="clickable" onclick="sizer(0)">恢复</span>
			<br/>
			<label>字体调整框:</label>
				<span class="clickable" onclick="adjSet('sizer', 0)" id="sizerF">隐藏</span>
				<span class="clickable" onclick="adjSet('sizer', 1)" id="sizerT">显示</span>
			<br/>
			<label>右上角翻页控制:</label>
				<span class="clickable" onclick="adjSet('pager', 0)" id="pagerF">隐藏</span>
				<span class="clickable" onclick="adjSet('pager', 1)" id="pagerT">显示</span>
			<br/>
<?php if (defined('BBS_NEWPOSTSTAT')) { ?>
			<label>热门话题滚动:</label>
				<span class="clickable" onclick="adjSet('hot', 0)" id="hotF">下方</span>
				<span class="clickable" onclick="adjSet('hot', 1)" id="hotT">上方</span>
				<span class="clickable" onclick="KCNymsw();">关闭</span>
			<br/>
<?php } ?>
			<label>界面方案:</label><br>
			<div align="center">
<?php
	// 这里显示每个界面方案的缩略图
	$stylecount=count($style_names);
	$ret = "";
	for($i=0;$i<$stylecount;$i++)
	{
		$ret .= "<p><input type=\"radio\" id=\"style{$i}\" onclick=\"pvStyle($i)\" name=\"styleid\" value=\"{$i}\">";
		$ret .= "<img src=\"{$style_names[$i][1]}\" onClick=\"chkStyle($i);\"><br/>{$style_names[$i][0]}</p>";
	}
	print($ret);
?>
			</div>
			<br/>
		</div>
	</fieldset>
	<div class="oper"><input type="submit" value="保存设置"/> &nbsp;<input type="button" onclick="history.go(-1);" value="快速返回"/></div>
</form>
<div class="large left"><ul>
	<li>修改立即生效。</li>
	<li>如果您希望保证每次登录都使用这个设置，可以点 保存设置（必须先登录）。</li>
</ul></div>
<?php
	page_footer();
?>
