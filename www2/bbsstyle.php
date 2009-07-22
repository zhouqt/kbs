<?php
	require("www2-funcs.php");
	login_init();
	toolbox_header("界面修改");

	if (isset($_GET['do']) && isset($_COOKIE["WWWPARAMS"])) {
		if (strcmp($currentuser["userid"], "guest")) {
			bbs_setwwwparameters($_COOKIE["WWWPARAMS"]); /* TODO: return value ? */
		}
	}
?>
<script type="text/javascript"><!--
	var settings = {"sizer": 3, "pager": 4, "hot": 5, "unread": 12, "replyform": 13}; /* faint IE5 */
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
	function KCNymsw() {
		alert('还没做这个功能呢... 需要吗？不需要吗？您去 sysop 版喊喊？');
	}
	var oldCssID = null, nowCssID = null;
	function setStyleDiv(cssID) {
		for (var i=0; i<32; i++) {
			var o = getObj("stylediv" + i);
			if (o) {
				o.style.borderColor = (i!=cssID) ? 'white' : 'blue';
			} else break;
		}
	}
	function chkStyle(cssID) {
		setStyleDiv(cssID);
		nowCssID = cssID;
	}
	function applyStyle() {
		if (nowCssID == oldCssID) return;
		oldCssID = nowCssID;
		saveParaCookie(nowCssID << 7, 0xF80);
		resetCss();
		var ff = top.window["menu"]; if (ff) ff.resetCss();
		ff = top.window["toogle"]; if (ff) ff.resetCss();
		ff = top.window["f4"]; if (ff) ff.resetCss();
	}
	function bootFn() {
		var c = readParaCookie();
		for (var n in settings) {
			var i = settings[n];
			var v = c & (1 << i);
			setInd(n, v);
		}
		oldCssID = nowCssID = getCssID();
		setStyleDiv(nowCssID);
		if (!isLogin()) {
			getObj("cmdSave").disabled = true;
			getObj("idpsave").style.visibility = "hidden";
		} else {
			getObj("idUnread").style.display = "block";
		}
	}
//-->
</script>
<style type="text/css">
form.stylesel {
	text-align: center;
	margin: 0px auto 10px auto;
	width: 500px;
}
div.stylesel {
	float:left;
	margin: 10px;
	padding: 7px;
	border: 3px solid white;
}
div.stylesel img {
	width: 200px;
	height: 150px;
	margin-bottom: 10px;
	border: 0;
}
</style>
<form action="?" class="medium align" method="get">
<input type="hidden" name="do" value="1" />
	<fieldset><legend>界面选项</legend>
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
			<label>帖子下快速回复:</label>
				<span class="clickable" onclick="adjSet('replyform', 0)" id="replyformF">启用</span>
				<span class="clickable" onclick="adjSet('replyform', 1)" id="replyformT">禁用</span>
			<br/>
		</div>
	</fieldset>
</form>
<form action="?" class="stylesel" method="get" onsubmit="applyStyle()"">
<input type="hidden" name="do" value="1" />
	<fieldset><legend>界面方案:</legend>
		<div align="center">
<?php
	// 这里显示每个界面方案的缩略图
	$stylecount = count($style_names);
	$ret = "";
	for($i=0;$i<$stylecount;$i++)
	{
		$ret .= "<div id='stylediv{$i}' class='stylesel clickable' onClick='chkStyle($i);'>";
		$ret .= "<img id='stylethumb{$i}' alt='预览图' src='images/{$i}/thumb.jpg' /><br/>{$style_names[$i]}</div>";
	}
	print($ret);
?>
		</div>
		<div class="oper clear"><input type="button" onclick="applyStyle();" value="应用界面方案"/></div>
	</fieldset>
	<div class="oper"><input id="cmdSave" type="submit" value="保存设置"/> &nbsp; <input type="button" onclick="history.go(-1);" value="快速返回"/><br/>
		<span id="idpsave">[ 点 保存设置 以保证每次登录都使用这个设置。]</span></div>
</form>
<form action="?" class="medium align" method="get" style="display:none;" id="idUnread">
<input type="hidden" name="do" value="1" />
	<fieldset><legend>未读标记显示</legend>
		未读标记的显示需要客户端相当的处理能力，如果您现在的机器较差，请勿开启此功能。
		<ol>
			<li>修改此功能请点上面的 保存设置 并重新登录才能生效。</li>
			<li>IE6 或 IE7 浏览器可以直接使用。</li>
			<li>Firefox 用户请安装 <a href="http://www.newsmth.net/kbsrc/index.html" target="_blank">KBSRC 扩展</a>。</li>
			<li>其余浏览器不支持。</li>
			<li>浏览结束请注意点左边的“离开本站”注销登录，以保证保存未读记录，程序也会每十分钟左右自动保存一次。</li>
		</ol>
		<hr/>
		<div class="inputs">
			<label>显示未读标记:</label>
				<span class="clickable" onclick="adjSet('unread', 0)" id="unreadF">隐藏</span>
				<span class="clickable" onclick="adjSet('unread', 1)" id="unreadT">显示</span>
			<br/>
		</div>
	</fieldset>
</form>
<script type="text/javascript">
	bootFn();
</script>
<?php
	page_footer();
?>
