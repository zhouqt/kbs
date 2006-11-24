/**
 * Part of the KBS BBS Code
 * Copyright (c) 2005-2006 KBS Development Team. (http://dev.kcn.cn/)
 * Source file is subject to the pending KBS License.
 *
 * You may use and/or modify the source code only for pure personal study
 * purpose (e.g. using it in a public website is not acceptable), unless
 * you get explicit permission by the KBS Development Team.
 */

/* TODO: obfuscate this file... */
var agt = navigator.userAgent.toLowerCase();
var gOp = (agt.indexOf("opera") != -1);
var gIE = ((agt.indexOf("msie") != -1) && !gOp);
var gFx = (agt.indexOf("gecko") != -1);
var gKon = (agt.indexOf("konqueror") != -1) && (agt.indexOf("safari") == -1);
var gIE5 = false;
if (gIE) {
	gIE5 = (parseFloat( agt.substring( agt.indexOf('msie ') + 5 ) ) < 6);
}
var gIE6Fx = ((gIE && !gIE5) || gFx);
var kbsrc = null; //namespace in this window
var www2dev = (typeof gwww2dev != "undefined");

if (!Array.prototype.push) {
	Array.prototype.push = function() {
		var s = this.length;
		for (var i = 0; i < arguments.length; i++)
			this[s + i] = arguments[i];
		return this.length;
	};
}

function StringBuffer() { this.buffer = []; }
StringBuffer.prototype = {
	append: function(string) { this.buffer.push(string); },
	toString: function() { return this.buffer.join(""); }
};

function w(s) { if (document&&document.write) document.write(s); }


var bootFn = Array();
function addBootFn(fn) {
	bootFn.push(fn);
}


function htmlize(s) {
	s = s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
	s = s.replace(/\x20\x20/g, " &nbsp;");
	return s;
}


var att = null, strPrint = "", strArticle = "", divArtCon = null;
function attWriter(bid, id, ftype, num, cacheable) {
	this.bid = bid;
	this.id = id;
	this.ftype = ftype;
	this.num = num;
	this.cacheable = cacheable;
}
function prints(s) {
	strPrint += s;
	s = s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
	s = s.replace(/\r[\[\d;]+[a-z]/gi, "");
	s = s.replace(/\x20\x20/g, " &nbsp;").replace(/\n /g, "\n&nbsp;");
	s = s.replace(/\n(: [^\n]*)/g, "<br/><span class=\"f006\">$1</span>");
	if (gKon && s.length > 0) {
		s = s.split("\n").join("<br/>");
	} else {
		s = s.replace(/\n/g, "<br/>");
	}
	if (!gIE5) {
		var urlmatch = new RegExp("((?:http|https|ftp|mms|rtsp)://(&(?=amp;)|[A-Za-z0-9\./=\?%_~@#:;\+\-])+)", "ig");
		s = s.replace(urlmatch, "<a target=\"_blank\" href=\"$1\">$1</a>");
	}
	strArticle += s;
	if (!divArtCon) w(s);
}
function attachURL(name, len, pos) {
	if (att.bid < 0) { return location.href+'&ap='+pos; }; //mail
	var ext = null, o = name.lastIndexOf(".");
	if (!att) return null;
	if (o != -1) {
		ext = name.substring(o + 1).toLowerCase();
	}
	var url = "att.php?";
	if (!att.cacheable) url += "n";
	else if (len > 51200) url += "p";
	else url += "s";
	url += "." + att.bid + "." + att.id;
	if (att.ftype) url += "." + att.ftype + "." + att.num;
	url += "." + pos;
	if (ext) url += "." + htmlize(ext);
	return (url);
}
function attach(name, len, pos) {
	var bImg = false;
	var s = "", url = attachURL(name, len, pos);
	if (!url) return;
	var o = name.lastIndexOf(".");
	if (o != -1) {
		var ext = name.substring(o + 1).toLowerCase();
		bImg = (ext == "jpg" || ext == "jpeg" || ext == "gif"
			 || ext == "ico" || ext == "png"  || ext == "pcx"
			 || ext == "bmp");
	}
	var sizes = len + ' 字节';
	if (len > 8192) sizes = parseInt(len/1024) + 'KB';
	if (bImg) {
		s += '<br /><img src="images/img.gif"/>此主题相关图片如下：'
		  + name + ' (' + sizes + ')<br /><a href="' + url + '" target="_blank">'
		  + '<img src="' + url + '" title="按此在新窗口浏览图片" style="margin: 0.2em 0 0.5em 0;" onload="resizeImg(this)" /></a> ';
	} else {
		s += '<br />附件: <a href="' + url + '">' + name + '</a> (' + sizes + ')<br />';
	}
	strArticle += s;
	if (!divArtCon) w(s);
}
function writeArticle() {
	divArtCon.innerHTML = strArticle;
}


function getCookie(name, def){
	var cname = name + "="; 
	var dc = document.cookie; 
	if (dc.length > 0) { 
		var begin = dc.indexOf(cname); 
		if (begin != -1) { 
			begin += cname.length; 
			var end = dc.indexOf(";", begin);
			if (end == -1) end = dc.length;
			return unescape(dc.substring(begin, end));
		} 
	}
	return def;
}

var kbsAd = false;
function getAds(i,l,n) {
	var ret = "";
	if (!kbsAd) {
		kbsAd = true;
		ret += "<iframe width='0' height='0' src='images/img.gif' frameborder='0' scrolling='no' id='hfad' name='hfad'></iframe>";
	}
	ret += "<a href='" + l + "' target='_blank' onclick='recordAds(" + n + ")'><img src='/images/ads/" + i + "' border='0'/></a>";
	return ret;
}
function writeAds(i,l,n) {
	w(getAds(i,l,n));
}
function recordAds(n) {
	var cc = getCookie('kbsAd', '');
	if (cc.length > 0) cc = cc.split(',');
	else cc = new Array();
	var i, now = parseInt((new Date()).getTime() / 1000);
	for(i=0;i<cc.length-1;i+=2) {
		if (cc[i] == n) {
			if (cc[i+1] > now - 1800) return;
			cc[i+1] = now;
			break;
		}
	}
	if (i >= cc.length - 1) {
		cc.push(n); cc.push(now);
	}
	window.frames['hfad'].document.location.href = 'http://dev.kcn.cn/adsense?' + n;
	var expire = new Date();
	expire.setTime(expire.getTime() + 1800000);
	document.cookie = 'kbsAd=' + cc.join(',') + ';path=/;expires=' + expire.toGMTString();
}

function saveParaCookie(v, mask) {
	var ex = '';
	if (!isLogin()) {
		var expire = new Date();
		expire.setTime(expire.getTime() + 3600000 * 24 * 7);
		ex = ';expires=' + expire.toGMTString();
	}
	var cook = (readParaCookie() & ~mask) | (v & mask);
	document.cookie = 'WWWPARAMS=' + cook + ex + ';path=/';
}
function readParaCookie() {
	return parseInt(getCookie('WWWPARAMS', 0));
}
function showUnread() {
	return (isLogin() && gIE && !gIE5 && (readParaCookie() & 0x1000));
}
function getUserid() {
	return getCookie("UTMPUSERID", "guest");
}
function isLogin() {
	return (getUserid() != "guest");
}

function queryString(param, def) {
	 var url = location.search.toUpperCase() + "&";
	 param = param.toUpperCase();
	 if (!def) def="";
	 var pos = url.indexOf(param + "=");
	 if (pos == -1) return def;
	 pos += param.length + 1;
	 return location.search.substring(pos, url.indexOf("&", pos));
}


function getObj(n) {
	return document.getElementById(n);
}

var bbsconImg = new Array();
function resizeImg(obj) {
	bbsconImg.push(obj);
	obj.o_width = obj.width;
	var maxWidth = document.body.clientWidth - 40;
	if (obj.width > maxWidth) obj.width = maxWidth;
}
function adjustImg() {
	var maxWidth = document.body.clientWidth - 40;
	for (var i in bbsconImg) {
		var obj = bbsconImg[i];
		var o_width = obj.o_width;
		obj.width = (o_width > maxWidth) ? maxWidth : o_width;
	}
}
window.onresize = adjustImg;

function addEvent(obj,ev,func){
	if (obj.attachEvent) {
		obj.attachEvent("on"+ev, func);
	} else if(obj.addEventListener) {
		obj.addEventListener(ev,func,true);
	} else {
		obj["on"+ev] = func;
	}
}
function cancelEvent(ev) {
	if (gIE) {
		ev.cancelBubble = true;
		ev.returnValue = false;
	} else {
		ev.preventDefault();
		ev.stopPropagation();
	}
}

function input_okd(obj, ev) {
	var evt = (ev) ? ev : ((window.event) ? event : null);
	if (evt == null) return true;
	var key = evt.keyCode ? evt.keyCode : evt.charCode;
	if (key == 13) {
		cancelEvent(evt);
		if (obj.form.onsubmit && !obj.form.onsubmit()) {
			return true;
		}
		obj.form.submit();
		return false;
	}
	return true;
}

/* textarea onkeydown event handler. to submit form with ctrl+W(IE only) or ctrl+ENTER */
function textarea_okd(func, ev) {
	var evt = (ev) ? ev : ((window.event) ? event : null);
	if (evt == null) return true;
	var key = evt.keyCode ? evt.keyCode : evt.charCode;
	if ((key == 87 && evt.ctrlKey) || (key == 13 && evt.ctrlKey)) {
		cancelEvent(evt);
		if (typeof func == "function")  {
			func();
		} else {
			func.form.submit();
		}
		return false;
	}
	return true;
}

function setCursorPosition(oInput,oStart,oEnd) {
	oInput.focus();
	if( oInput.setSelectionRange ) {
		oInput.setSelectionRange(oStart,oEnd);
	} else if( oInput.createTextRange ) {
		var range = oInput.createTextRange();
		range.collapse(true);
		range.moveEnd('character',oEnd);
		range.moveStart('character',oStart);
		range.select();
	}
}

/* refresh message frame */
function alertmsg() {
	if (top.fmsg && !top.fmsg.document.getElementById("msgs")) top.fmsg.location.reload();
}


window.onload = function() {
	/* set focus */
	var i,f = getObj("sfocus");
	if (f) {
		f.focus();
		setCursorPosition(f, 0, 0);
	} else {
		f = getObj("sselect");
		if (f) {
			f.focus(); f.select();
		}
	}

	/*
	 * apply col class. this is a workaround for css2.1 - atppp
	 * see also: http://ln.hixie.ch/?start=1070385285&count=1
	 * TODO: consider td's span properties
	 *
	 * also: apply alternate style for rows.
	 *
	 * Don't use this with long table, as IE runs it extremely slowly.
	 */
	var m = document.getElementsByTagName("table");
	for(i=0;i<m.length;i++) {
		var tab = m[i];
		if (tab.className.indexOf("adj") == -1) continue;
		var cols = tab.getElementsByTagName("col");
		if (cols.length == 0) continue;
		var rows = tab.getElementsByTagName("tr");
		for (var j=0;j<rows.length;j++) {
			var row = rows[j];
			row.className = (j%2)?"odd":"even";
			var td = row.getElementsByTagName("td");
			for (var k=0;k<td.length;k++) {
				if (cols[k] && cols[k].className) {
					if (td[k].className) {
						td[k].className += " " + cols[k].className;
					} else {
						td[k].className = cols[k].className;
					}
				}
			}
		}
	}
		
	/* this is a workaround for some weird behavior... ask atppp if you are interested. BUGID 7629 */
	if (gFx) {
		var ll, links = document.getElementsByTagName("link");
		for(i=0; i<links.length; i++) {
			ll = links[i];
			if((ll.getAttribute("rel") == "stylesheet")) {
				ll.disabled = true;
				ll.disabled = false;
			}
		}
	}

	for(i=0; i<bootFn.length; i++) {
		var fn = bootFn[i]; fn();
	}
};

/* check required fields when submitting form. required fields are denoted with classname "req" */
function chkreq(frm) {
	var f = frm.getElementsByTagName("input");
	var i;
	for(i=0;i<f.length;i++) {
		var fe = f[i];
		if (fe.className=="req") {
			if(fe.value==''){
				alert('有必填内容为空');
				fe.focus();
				return false;
			}
		}
	}
	return true;
}

var gTimer;
function footerStart() {
	var prefixZero = function(d) {
		if (d > 9) return d;
		else return "0" + d;
	};
	var MyTimer = function() {
		this.start = (new Date()).getTime();
		this.serverDiff = serverTime * 1000 - this.start;
		this.serverTime = serverTime * 1000;
		this.startTime = this.start - stayTime * 1000;
		this.lastStay = this.lastClock = "";
		this.refreshTime = 0;
		this.userid = getUserid();
	};
	MyTimer.prototype.refresh = function() {
		var now = (new Date()).getTime();
		this.serverTime = this.serverDiff + now;
		var Timer = new Date(this.serverTime);
		var str = Timer.getUTCFullYear() + "年" + prefixZero(Timer.getUTCMonth() + 1) + "月" 
				+ prefixZero(Timer.getUTCDate()) + "日" + prefixZero(Timer.getUTCHours())
				+ ":" + prefixZero(Timer.getUTCMinutes());
		if (this.lastClock != str) {
			this.lastClock = str;
			getObj("divTime").innerHTML = str;
		}
		var staySec = (now - this.startTime) / 60000;
		str = parseInt(staySec/60) + "小时" + parseInt(staySec % 60) + "分钟";
		if (this.lastStay != str) {
			this.lastStay = str;
			getObj("divStay").innerHTML = str;
		}
		var bF5 = (this.userid != getUserid());
		if (bF5) {
			if (top.menu) top.menu.location.href = isLogin() ? 'bbsleft.php' : 'bbsguestleft.html';
		}
		if ((this.refreshTime > 0 && now > this.refreshTime) || bF5) {
			location.reload();
		} else {
			setTimeout("gTimer.refresh()", 1000);
		}
	};
	gTimer = new MyTimer();
	gTimer.refresh();
	
	if (hasMail) {
		/*if (typeof top.hasMsgBox == "undefined") {
			top.hasMsgBox = false;
		}
		if (!top.hasMsgBox) {
			top.hasMsgBox = true;
			/*if(confirm("您有新邮件，现在查收？")) {
				top.f3.location.href = "bbsmailbox.php?path=.DIR&title=%CA%D5%BC%FE%CF%E4";
			}
			top.hasMsgBox = false;
		}*/
		newmailnotice();
	}
	gTimer.refreshTime = (new Date()).getTime() + 540000;
}

function getFindBox(board) { /* TODO: sfocus here might conflict with others */
	var s = 
	'<form action="bbsbfind.php" method="GET" class="medium"><input type="hidden" name="q" value="1"/>\
		<fieldset><legend>版内文章搜索 <span id="bbbHide"></span></legend>\
			<div class="inputs">\
				<label>版面名称:</label><input type="text" maxlength="24" size="24" name="board" value="' + board + '"/><br/>\
				<label>标题含有:</label><input type="text" maxlength="50" size="20" name="title" id="sfocus" /> AND<br/>\
				<label>　　　　 </label><input type="text" maxlength="50" size="20" name="title2"/><br/>\
				<label>标题不含:</label><input type="text" maxlength="50" size="20" name="title3"/><br/>\
				<label>作者帐号:</label><input type="text" maxlength="12" size="12" name="userid"/><br/>\
				<label>时间范围:</label><input type="text" maxlength="4"  size="4"  name="dt" value="7"/> 天以内<br/>\
				<input type="checkbox" name="mg" id="mg"><label for="mg" class="clickable">精华文章</label>\
				<input type="checkbox" name="ag" id="ag"><label for="ag" class="clickable">带附件文章</label>\
				<input type="checkbox" name="og" id="og"><label for="og" class="clickable">不含跟贴</label>\
			</div>\
		</fieldset>\
		<div class="oper"><input type="submit" value="查询"/></div>\
	</form>';
	return s;
}

function showFindBox(board) {
	if (!gIE6Fx || !www2dev) return true;
	var divID = "articleFinder";
	var div = getObj(divID);
	if (!div) {
		div = document.createElement("div");
		div.id = divID;
		div.innerHTML = getFindBox(board);
		div.className = "float";
		div.style.top = "5em";
		div.style.left = "20em";
		div.style.padding = "0.5em";
		div.style.zIndex = 2;
		div.style.backgroundColor = "lightyellow";
		div.style.display = "none";
		if (gIE) {
			div.style.filter = "alpha(opacity=0)";
		}
		document.body.appendChild(div);
		var o = getObj("bbbHide");
		o.innerHTML = "(隐藏)";
		o.className = "clickable";
		o.title = "隐藏搜索窗口";
		o.onclick = function() {
			div.style.display = "none";
		}
	}
	if (gIE) {
		div.filters[0].opacity  = 0;
	} else {
		div.style.opacity = 0;
	}
	div.style.display = "";
	getObj("sfocus").focus();
	
	/* TODO: general fade-in fade-out effect control */
	var opa = 0;
	var fn = function() {
		opa += 10;
		if (gIE) {
			div.filters[0].opacity = opa;
		} else {
			div.style.opacity = opa / 100.0;
		}
		if (opa < 90) {
			setTimeout(fn, 10);
		}
	};
	fn();
	return false;
}

function dosubmit() {
	var p = document.postform;
	p.post.value='发表中，请稍候...';
	p.post.disabled=true;
	p.submit();
}


var hotBoard = '', hotBid = 0, hotMove = true, hotFn = null;
function setHots(h) {
	var i,hots = new Array();
	for(i=0; i<h.length; i++) {
		if (h[i]) {
			hots.push('<a href="bbscon.php?bid=' + hotBid + '&id=' + h[i][0] + '">' + h[i][1] + '</a>' +
			'[<a href="bbstcon.php?board=' + hotBoard + '&gid=' + h[i][0] + '">同主题</a>](' + h[i][2] + ')');
		}
	}
	if (gIE) {
		var str = '';
		for(i=0; i<hots.length; i++) {
			if (hots[i]) str+= hots[i] + '&nbsp;&nbsp;&nbsp;&nbsp;';
		}
		getObj('hotTopics').innerHTML = str;
	} else {
		var index = -1;
		hotFn = function() {
			var ii = '';
			if (hots.length) {
				var move = 1;
				if (!hotMove) {
					if (arguments.length) move = arguments[0];
					else return;
				}
				if (move > 0) {
					index++; if (index >= hots.length) index = 0;
				} else {
					index--; if (index < 0) index = hots.length - 1;
				}
				if (hots.length > 1) {
					ii += '<span class="clickable" onclick="hotFn(-1)" title="上一条">&lt;</span> '
						+ '<span class="clickable" onclick="hotFn(1)" title="下一条">&gt;</span> ';
				}
				ii += (index+1) + "/" + hots.length + ": " + hots[index];
			}
			getObj('hotTopics').innerHTML = ii;
		};
		hotFn();
		setInterval("hotFn()", 5000);
	}
}

function hotTopic(board, bid) { /* TODO: no table, use AJAX */
	/* clear: both is for stupid Firefox */
	var str = '<table cellspacing="0" cellpadding="5" border="0" width="100%" style="margin: 0.5em auto 0 auto;clear:both;"><tr>' +
			  '<td width="100" align="center">[<span class="red">热门话题</span>]</td><td>';
	if (gIE) {
		str += '<marquee onmouseover="this.stop()" onmouseout="this.start()"><span id="hotTopics">载入中...</span></marquee>';
	} else {
		str += '<span id="hotTopics" onmouseover="hotMove=false;" onmouseout="hotMove=true;">载入中...</span>';
	}
	str += '</td></tr></table>';
	w(str + '<iframe width=0 height=0 src="images/img.gif" frameborder="0" scrolling="no" id="hiddenframe" name="hiddenframe"></iframe>');
	hotBoard = escape(board);
	hotBid = bid;
	addBootFn(function() {
		window.frames["hiddenframe"].document.location.href = "bbshot.php?board=" + board;
	});
}


function getURLPath(url) {
	var pos = url.indexOf("://");
	if (pos != -1) {
		var uri = url.substr(pos + 3);
		if ((pos = uri.indexOf("/")) != -1) {
			url = uri.substr(pos);
		}
	}
	return(url);
}

function checkFrame() {
	var msg = '<div id="fontSizer"';
	if (!(readParaCookie() & 8)) {
		msg += ' style="display:none"';
	}
	msg += '>'
	+ '<span class="clickable" onclick="sizer(1)" title="字体大大大...大点">+</span> '
	+ '<span class="clickable" onclick="sizer(-1)" title="字体小小小...小点">-</span> '
	+ '<span class="clickable" onclick="sizer(0)" title="字体变变变...变回去">R</span></div>';
	w(msg);
	if (top == self) { /* TODO: use better way */
		var url = getURLPath(document.location.toString());
		var ex = '(<a href="frames.html?mainurl=' + escape(url) + '">展开完整界面</a>)';
		var o = getObj("idExp");
		if (o) o.innerHTML = ex;
	}
}

var bfsI, bfsD = 4;
var bfsArr = [0,11,12,13,14,16,18,20];
var bfsSma = [0,11,12,12,12,14,15,17];
function sizer(flag) {
	if (flag == 1) {
		if (bfsI < 7) bfsI++;
	} else if (flag == -1) {
		if (bfsI > 1) bfsI--;
	} else bfsI = bfsD;
	saveParaCookie(bfsI, 7);
	document.body.style.fontSize = bfsArr[bfsI] + 'px';
	/* here's the trick/test part: change .smaller definition! */
	var i, t;
	if (!(t = document.styleSheets)) return;
	for(i = 0; i < t.length; i++) {
		if (t[i].title == "myStyle") {
			t = t[i];
			if (t.cssRules) t = t.cssRules;
			else if (t.rules) t = t.rules;
			else return;
			for (i = 0; i < t.length; i++) {
				if (t[i].selectorText == ".smaller") {
					t[i].style.fontSize = bfsSma[bfsI] + 'px';
					break;
				}
			}
			return;
		}
	}
}

function getCssID() { return ((readParaCookie() & 0xF80) >> 7); }
function replaceCssFile(file) {
	var reg = /images\/([0-9]+)\//;
	if (file.match(reg)) {
		return file.replace(reg, 'images/'+getCssID()+'/');
	} return false;
}

function getCssFile(file) {
	return ('images/' + getCssID() + '/' + file + '.css');
}

function writeCssFile(file) {
	w('<link rel="stylesheet" type="text/css" href="' + getCssFile(file) + '" />');
	if (showUnread()) {
		w('<script type="text/javascript" src="kbsrc/content/kbsrc.js"></script>');
		//for now. readystatechange event might trigger IE bug "operation aborted"
		addBootFn(function() {
			try {
				kbsrcIEEntry();
			} catch(e) { }
		});
	}
}

function resetCss() {
	var i, h, t = document.getElementsByTagName("img");
	if (t) {
		for(i = 0; i < t.length; i++) {
			if(t[i].id.substr(0,10) != "stylethumb")
			{
				h = replaceCssFile(t[i].src);
				if (h) t[i].src = h;
			}
		}		
	}
	t = document.getElementsByTagName("link");
	if (t) {
		for(i = 0; i < t.length; i++) {
			h = replaceCssFile(t[i].getAttribute("href"));
			if (h) {
				t[i].setAttribute("href", h);
				return;
			}
		}
	}
	location.reload();
}

function writeCss() {
	writeCssFile('www2-default');
	bfsI = readParaCookie() & 7;
	if (bfsI <= 0 || bfsI >= bfsArr.length) bfsI = bfsD;
	var ret = '<style type="text/css" title="myStyle"><!--';
	ret += 'body{font-size:' + bfsArr[bfsI] + 'px;}';
	ret += '.smaller{font-size:' + bfsSma[bfsI] + 'px;}';
	ret += '--></style>';
	w(ret);
}
function writeCssLeft() { writeCssFile('bbsleft'); }
function writeCssMainpage() { writeCssFile('mainpage'); }


function putImageCode(filename,otherparam) {
	return('<img src="images/'+getCssID()+'/'+filename+'" '+otherparam+'>');
}

function putImage(filename,otherparam)
{
	w(putImageCode(filename,otherparam));
}


function writeBM_html(bmstr, firstBM) {
	if (bmstr.length == 0) return '诚征版主中';
	if (!/^[A-Za-z]/.test(bms)) return bms;
	var bms = bmstr.split(" ");
	var len = firstBM ? 1 : bms.length;
	for(var i=0; i<len; i++) {
		bms[i] = '<a href="bbsqry.php?userid=' + bms[i] + '">' + bms[i] + '</a>';
	}
	return firstBM ? bms[0] : bms.join(" ");
}
function writeBMs(bmstr) {
	w(writeBM_html(bmstr, false));
}

function isBM(bid) {
	var mbids = getCookie("MANAGEBIDS", "0");
	if (mbids == "A") return 1;
	mbids = "." + mbids + ".";
	return (mbids.indexOf("." + bid + ".") != -1);
}


var dir_modes = {"FIND": -2, "ANNOUNCE": -1, "NORMAL": 0, "DIGEST": 1, "MARK": 3, "DELETED": 4, "ORIGIN": 6, "ZHIDING": 11};
function dir_name(ftype) {
	switch(ftype) {
		case -1: return "(精华区)";
		case 1: return "(文摘区)";
		case 3: return "(保留区)";
		case 6: return "(主题模式)";
		default: return "";
	}
}

/* man - 1: 普通管理模式，2: 回收站模式 */
function docWriter(board, bid, start, man, ftype, page, total, apath, showHot, normalB) {
	this.monthStr = ["January","February","March","April","May","June","July","August","September","October","November","December"];
	this.board = escape(board);
	this.bid = bid;
	this.ids = new Array();
	this.start = start;
	this.page = page;
	this.total = total;
	this.apath = apath;
	this.man = man;
	this.ftype = ftype;
	this.num = 0;
	this.baseurl = "bbsdoc.php?board=" + this.board;
	this.showHot = showHot;
	this.normalB = normalB;
	this.hotOpt = (readParaCookie() & 0x60) >> 5;
	if (this.man) this.baseurl += "&manage=1";
	if (this.ftype) this.baseurl += "&ftype=" + this.ftype;

	if (showHot && this.hotOpt == 1) hotTopic(this.board, this.bid);

	var str = '<div class="doc"><div class="docTab smaller">';
	if (!ftype && isLogin()) {
		var url = 'bbspst.php?board=' + this.board;
		str += '<div class="post"><a href="' + url + '">' + putImageCode('postnew.gif','alt="发表话题" class="flimg" onclick="location.href=\'' + url + '\';"') + '</a></div>';
	}

	var mls = [[ftype || man, "普通模式", "bbsdoc.php?board=" + this.board],
			   [ftype != dir_modes["DIGEST"], "文摘区", "bbsdoc.php?board=" + this.board + "&ftype=" + dir_modes["DIGEST"]],
			   [ftype != dir_modes["MARK"], "保留区", "bbsdoc.php?board=" + this.board + "&ftype=" + dir_modes["MARK"]],
			   [ftype != dir_modes["ORIGIN"], "同主题", "bbsdoc.php?board=" + this.board + "&ftype=" + dir_modes["ORIGIN"]],
			   [apath && ftype != dir_modes["ANNOUNCE"], "精华区", "bbs0an.php?path=" + escape(apath)],
			   [ftype != dir_modes["FIND"], "查询", "bbsbfind.php?board=" + this.board, "showFindBox('" + this.board + "')"]];
	var mls_bm = [[ftype != dir_modes["DELETED"], "回收站", "bbsdoc.php?manage=1&board=" + this.board + "&ftype=" + dir_modes["DELETED"]]];
	if (isBM(bid)) mls = mls.concat(mls_bm);
	for (var i = mls.length - 1; i >= 0; i--) {
		links = mls[i];
		if (links[0]) {
			if (links[3]) {
				str += '<a onclick="return ' + links[3] + ';" href="' + links[2] + '">' + links[1] + '</a>';
			} else {
				str += '<a href="' + links[2] + '">' + links[1] + '</a>';
			}
		} else if (ftype != dir_modes["FIND"]) {
			str += '<b>' + links[1] + '</b>';
		} else {
			str += '<b class="clickable" onclick="location.href=\''+links[2]+'\';">' + links[1] + '</b>';
		}
	}
	str += '</div>';

	if (ftype >= 0) {
		if (man) {
			str += '<form name="manage" id="manage" method="post" action="bbsdoc.php?manage=1&board=' + this.board + '&page=' + page;
			if (man == 2) str += '&ftype=' + dir_modes["DELETED"];
			str += '">';
		}
		str += '<table class="main wide">';
		str += '<col width="50"/><col width="50"/>';
		if (man) {
			str += '<col width="50"/>';
		}
		str += '<col width="85"/><col width="60"/><col width="*"/>';
		str += '<tbody><tr><th>序号</th><th>标记</th>';
		if (man) {
			str += '<th>管理</th>';
		}
		var links = '标题';
		if (readParaCookie() & 16) {
			links = '<div class="relative">标题<div class="pagerlink">';
			if (this.page > 1) {
				links += '<a href="' + this.baseurl + '&page=1" title="第一页">&lt;&lt;</a>';
				links += ' <a href="' + this.baseurl + '&page=' + (this.page - 1) + '" title="上一页">&lt;</a>';
			} else {
				links += '&lt;&lt; &lt;';
			}
			if (this.start <= this.total - 20) {
				links += ' <a href="' + this.baseurl + '&page=' + (this.page + 1) + '" title="下一页">&gt;</a>';
				links += ' <a href="' + this.baseurl + '" title="最后一页">&gt;&gt;</a>';
			} else {
				links += ' &gt; &gt;&gt;';
			}
			links += '</div></div>';
		}
		str += '<th>作者</th><th>日期</th><th>' + links + '</th></tr>';
	}
	w(str);
}
docWriter.prototype.o = function(id, gid, author, flag, time, title, size, imported) {
	var rowclass;	
	if (www2dev && top.hlInfo && !this.man) {
		var info = top.hlInfo.split(',');
		if (this.bid == info[0] && gid == info[2])
			rowclass = (id==info[1])?'rowReading':((id==gid)?'rowTopic':'rowRe');
		else
			rowclass = (this.num%2)?'even':'odd';
	}
	else
		rowclass = (this.num%2)?'even':'odd';
	var str = '<tr class="' + rowclass + '">';
	
	var cb_value = (this.man == 2) ? (this.start + this.num) : id; /* 回收站中以序号代替id */
	var bf = flag.charAt(0);
	if (bf.toLowerCase() == 'd') { /* 置顶 */
		str += '<td class="center red strong">提示</td><td class="center">' + putImageCode('istop.gif','alt="提示"') + '</td>';
		if (this.man) {
			str += '<td class="center"><input type="checkbox" name="ding' + this.num + '" value="' + cb_value + '" /></td>';
		}
	} else {
		if (!this.man && this.normalB && (bf == 'N' || bf == '*')) flag = ' ' + flag.charAt(1);
		this.ids.push(id);
		flag = '<span id="kbsrc' + id + '">' + flag + '</span>';
		if (this.man && (imported == 1))
			flag += putImageCode('imported.gif', '');
		str += '<td class="center">' + (this.num + this.start) + '</td><td class="center">' + flag + '</td>';
		if (this.man) {
			str += '<td class="center"><input type="checkbox" name="art' + this.num + '" value="' + cb_value + '" /></td>';
		}
	}
	str += '<td class="center"><a href="bbsqry.php?userid=' + author + '">' + author + '</a></td>';
	var fd = new Date(time * 1000);
	str += '<td><nobr>' + this.monthStr[fd.getMonth()].substr(0,3) + "&nbsp;"
	var dd = fd.getDate();
	str += ((dd < 10) ? ' ' : '') + dd + '</nobr></td>';
	str += '<td>';

	if (title.substr(0,4)!="Re: ") title = "● " + title;

	switch (this.ftype) {
		case dir_modes["ORIGIN"]:
			str += '<a href="bbstcon.php?board=' + this.board + '&gid=' + gid + '">' + title + '</a>';
			break;
		case dir_modes["NORMAL"]:
			str += '<a href="bbscon.php?bid=' + this.bid + '&id=' + id;
			if (bf.toLowerCase() == 'd') str += "&ftype=" + dir_modes["ZHIDING"];
			str += '">' + title + '</a>';
			if (size >= 1000) {
				str += '<span class="red">(' + (Math.floor(size / 100) / 10.0) + 'k)</span>';
			} else {
				str += '<span class="normal">(' + size + ')</span>';
			}
			break;
		default:
			str += '<a href="bbscon.php?bid=' + this.bid + '&id=' + id + '&ftype=' + this.ftype + '&num=' + (this.start + this.num) + '">' + title + '</a>';
			break;
	}
	str += '</td></tr>';
	w(str);
	this.num++;
};
function mansubmit(flag) {
	document.manage.act.value = flag;
	document.manage.submit();
}
docWriter.prototype.t = function() {
	var ret = '';
	ret += '</tbody></table>';
	if (this.man) {
		var bbsman_modes = {"DEL": 1, "MARK": 2, "DIGEST": 3, "NOREPLY": 4,	"ZHIDING": 5, "UNDEL": 6, "PERCENT": 7, "TODEL": 8, "SHARP": 9, "ANNOUNCE": 10, "ANNOUNCEADD": 11, "IMPORT_I": 12, "IMPORT_J": 13};
		ret += '<input type="hidden" name="act" value=""/>';
		if (this.ftype != dir_modes["DELETED"])
		{ /* TODO: 这段似乎可以简化一点... */
			ret += '<br>基本标记： [<a href="javascript:mansubmit(' + bbsman_modes['MARK'] + ');">切换M</a>]';
			ret += ' [<a href="javascript:mansubmit(' + bbsman_modes['DIGEST'] + ');">切换G</a>]';
			ret += ' [<a href="javascript:mansubmit(' + bbsman_modes['PERCENT'] + ');">切换%</a>]';
			ret += ' [<a href="javascript:mansubmit(' + bbsman_modes['TODEL'] + ');">切换X</a>]';
			ret += ' [<a href="javascript:mansubmit(' + bbsman_modes['SHARP'] + ');">切换#</a>]<br>';
			ret += '其他操作： [<a href="javascript:if(confirm(\'确定要删除吗？\')){mansubmit(' + bbsman_modes['DEL'] + ');}">删除</a>]';
			ret += ' [<a href="javascript:mansubmit(' + bbsman_modes['NOREPLY'] + ');">切换不可回复</a>]';
			ret += ' [<a href="javascript:mansubmit(' + bbsman_modes['ZHIDING'] + ');">置底</a>]';
			ret += ' [<a href="javascript:mansubmit(' + bbsman_modes['ANNOUNCE'] + ');">收精华区</a>]<br>';
			ret += '暂 存 档： [<a href="bbsimport.php" target="_blank">看暂存档</a>]';
			ret += ' [<a href="bbsimport.php?act=clear" target="_blank">清空</a>]';
			ret += ' [<a href="javascript:mansubmit(' + bbsman_modes['IMPORT_I'] + ');">保留头尾</a>]';
			ret += ' [<a href="javascript:mansubmit(' + bbsman_modes['IMPORT_J'] + ');">去掉头尾</a>]';
		}
		else
		{
			ret += '<input type="button" value="恢复到版面" onclick="mansubmit(' + bbsman_modes['UNDEL'] + ');"/>';
		}
		ret += '<br></form>';
	}
	ret += '<form action="bbsdoc.php" method="get" class="docPager smaller">';
	ret += '<input type="hidden" name="board" value="' + this.board + '"/>';
	if (this.man) {
		ret += '<input type="hidden" name="manage" value="1"/>';
	}
	if (!this.ftype) {
		if (isLogin()) {
			var url = 'bbspst.php?board=' + this.board;
			ret += '<a href="' + url + '" class="flimg">' + putImageCode('postnew.gif','alt="发表话题" class="flimg" onclick="location.href=\'' + url + '\';"') + '</a>';
		}
	} else {
		ret += '<input type="hidden" name="ftype" value="' + this.ftype + '"/>';
	}

	if (!this.man) {
		ret += '[<a href="javascript:location.reload()">刷新</a>]';
	}
	if (this.page > 1) {
		ret += ' [<a href="' + this.baseurl + '&page=1">第一页</a>]';
		ret += ' [<a href="' + this.baseurl + '&page=' + (this.page - 1) + '">上一页</a>]';
	} else {
		ret += ' [第一页] [上一页]';
	}
	if (this.start <= this.total - 20) {
		ret += ' [<a href="' + this.baseurl + '&page=' + (this.page + 1) + '">下一页</a>]';
		ret += ' [<a href="' + this.baseurl + '">最后一页</a>]';
	} else {
		ret += ' [下一页] [最后一页]';
	}
	ret += ' <input type="submit" value="跳转到"/> ';
	ret += '第 <input type="text" name="start" size="3" onmouseover="this.focus()" onfocus="this.select()" /> 篇/共 ' + this.total + ' 篇';
	ret += '</form>';

	ret += '</div>'; //class="doc"
	w(ret);
	
	if (this.showHot && this.hotOpt == 0) hotTopic(this.board, this.bid);
};
docWriter.prototype.f = function(rss,related,isclub) {
	var i,ret = '<div class="oper smaller">';
	ret += '[<a href="bbsnot.php?board=' + this.board + '">进版画面</a>] ';
	ret += '[<a href="bbsshowvote.php?board=' + this.board + '">版内投票</a>] ';
	ret += '[<a href="bbsshowtmpl.php?board=' + this.board + '">发文模板</a>] ';
	if (this.normalB && !this.man) {
		ret += '<span id="kbsrc_clear" style="display:none;">[<a href="bbsclear.php?board=' + this.board + '&kbsrc=1">清除未读</a>] </span>';
		ret += '<div id="kbsrcInfo">doc,' + this.bid + ',' + this.ids.join(',') + '</div>';
	} else {
		ret += '[<a href="bbsclear.php?board=' + this.board + '">清除未读</a>] ';
	}
	if (rss) {
		ret += '<a href="' + rss + '" title="RSS"><img src="images/xml.gif"/></a>';
	}
	if (related) {
		ret += '<br/>来这个版的人常去的其他版面：';
		for (i=0;i<related.length;i++) {
			ret += '[<a class="b3" href="bbsdoc.php?board=' + related[i] + '"><font class="b3">' + related[i] + '</font></a>]';
		}
	}
	var allAds = [['TrafficInfo','http://www.kooxoo.com/?fromid=smthpic01','smth_piao.gif',300],
				  ['Travel','http://www.kooxoo.com/?fromid=smthpic02','smth_lvyou.gif',301],
				  ['HouseRent','http://www.kooxoo.com/?fromid=smthpic03','smth_house.gif',302],
				  ['HouseSeek','http://www.kooxoo.com/?fromid=smthpic04','smth_rent.gif',303]];
	for (i=0;i<allAds.length;i++) {
		var ad = allAds[i];
		if (this.board == ad[0]) {
			ret += "<div><center>" + getAds(ad[2],ad[1],ad[3]) + "</center></div>";
			break;
		}
	}
	if (isBM(this.bid)) {
		ret += '<br/>管理链接：';
		ret += '[<a href="bbsdeny.php?board=' + this.board + '">封禁名单</a>] ';
		ret += '[<a href="bbsmnote.php?board=' + this.board + '">进版画面</a>] ';
		ret += '[<a href="bbsmvote.php?board=' + this.board + '">管理投票</a>] ';
		if (this.man != 1) {
			ret += '[<a href="bbsdoc.php?manage=1&board=' + this.board + '">管理模式</a>] ';
		}
		if (this.man) {
			ret += '[<a href="bbsdoc.php?board=' + this.board + '">普通模式</a>] ';
		}
		if (isclub) {
			ret += ' [<a href="bbsclub.php?board=' + this.board + '">俱乐部成员</a>]';
		}
		ret += ' [<a href="bbs0anbm.php?path=' + escape(this.apath) + '">精华区管理</a>]';
	}
	ret += '</div>';
	w(ret);
};

function clearArticleDiv(id) {
	if (parent && (divArtCon = parent.document.getElementById("art" + id))) {
		divArtCon.innerHTML = "";
	}
}

function conWriter(ftype, board, bid, id, gid, reid, favtxt, num) {
	this.board = escape(board);
	this.ftype = ftype;
	this.bid = bid;
	this.id = id;
	this.gid = gid;
	this.reid = reid;
	this.favtxt = favtxt;
	this.num = num;
	this.baseurl = "bbscon.php?bid=" + bid + "&id=" + id;

	if (parent && (divArtCon = parent.document.getElementById("art" + id))) {
		addBootFn(writeArticle);
		return;
	}
	w("<div id='kbsrcInfo'>con," + bid + "," + id + "</div>");

	if (!isLogin() && this.ftype) {
		this.headers = "";
		return;
	}
	var ret = '<div class="conPager smaller right">';
	if (isLogin()) { /* TODO: 某些模式应该禁止显示这两个链接 */
		var url = 'bbspst.php?board=' + this.board + '&reid=' + this.id ;
		if (gIE6Fx) {
			ret += '<a onclick="return showReplyForm(\'' + url + '\');" href="' + url + '">' + putImageCode('reply.gif','alt="回复帖子" class="flimg"') + '</a>';
		} else {
			ret += '<a href="' + url + '">' + putImageCode('reply.gif','alt="回复帖子" class="flimg" onclick="location.href=\'' + url + '\';"') + '</a>';
		}
		url = 'bbspst.php?board=' + this.board;
		ret += '<a href="' + url + '" class="flimg">' + putImageCode('postnew.gif','alt="发表话题" class="flimg" onclick="location.href=\'' + url + '\';"') + '</a>';
	}
	if (this.ftype == 0) {
		ret += '[<a href="' + this.baseurl + '&p=p">上一篇</a>] ';
		ret += '[<a href="' + this.baseurl + '&p=n">下一篇</a>] ';
		ret += '[<a href="' + this.baseurl + '&p=tp">同主题上篇</a>] ';
		ret += '[<a href="' + this.baseurl + '&p=tn">同主题下篇</a>]';
	} else {
		ret += '<span style="color:#CCCCCC">[上一篇] [下一篇] [同主题上篇] [同主题下篇]</span>';
	}
	ret += '</div>';
	this.headers = ret;
}
conWriter.prototype.h = function(isTop) {
	if (divArtCon) return;
	var ret = this.headers;
	if (!isTop) ret = '</div>' + ret + '<div id="divReplyForm"></div>';
	else ret += '<div class="article">';
	w(ret);
};
conWriter.prototype.t = function() {
	if (divArtCon) return;
	var PAGE_SIZE = 20;
	var url = "bbsdoc.php?board=" + this.board;
	var zd = false;
	if (this.ftype != dir_modes["ZHIDING"]) {
		url += "&page=" + Math.floor((this.num + PAGE_SIZE - 1) / PAGE_SIZE);
		if (this.ftype) url += "&ftype=" + this.ftype;
	} else zd = true;
	var ao = (!this.ftype || zd);
	var ret = '<div class="oper smaller" id="idConOper">';
	if (ao) {
		var qry = '?board=' + this.board + '&id=' + this.id;
		ret += '[<a href="bbstcon.php?board=' + this.board + '&gid=' + this.gid + '">同主题展开</a>] ';
		ret += '[<a href="bbscon.php?bid=' + this.bid + '&id=' + this.gid + '">同主题第一篇</a>] ';
		ret += '[<a href="bbstcon.php?board=' + this.board + '&gid=' + this.gid + '&start=' + this.id + '">从此处展开</a>] ';
		ret += '[<a href="bbscon.php?bid=' + this.bid + '&id=' + this.reid + '" title="跳转到本文所回复的文章">溯源</a>] ';
	}
	ret += '[<a href="' + url + '">返回版面' + dir_name(this.ftype) + '</a>] ';
	ret += '[<a href="javascript:history.go(-1)">快速返回</a>]<br/>';
	if (ao && isLogin()) {
		ret += '[<a href="bbsshowtmpl.php' + qry + '">模板回复</a>] ';
		ret += '[<a href="bbsfwd.php' + qry + '">转寄</a>] ';
		ret += '[<a href="bbsccc.php' + qry + '">转贴</a>] ';
		if (!zd) ret += '[<a href="bbscommend.php' + qry + '">推荐</a>] ';
		ret += '[' + this.favtxt + '] ';
		ret += '[<a href="bbspstmail.php' + qry + '">回信</a>] ';
		ret += '[<a href="bbsedit.php' + qry + '&ftype=' + this.ftype + '">修改</a>] ';
		if (!zd) {
			ret += '[<a href="bbsprop.php' + qry + '">属性</a>] ';
			ret += '[<a onclick="return confirm(\'你真的要删除本文吗?\')" href="bbsdel.php?board=' + this.board + '&id=' + this.id + '">删除</a>] ';
		}
	}
	ret += '</div>';
	if (gIE) addBootFn(function() { // fuck you IE
		var o = getObj("idConOper");
		o.style.display = "none";
		o.style.display = "block";
	});
	top.hlInfo = this.bid + "," + this.id + "," + this.gid;
	w(ret);
};




function tconWriter(board, bid, gid, start, tpage, pno, serial, prevgid, nextgid) {
	this.board = escape(board);
	this.bid = bid;
	this.gid = gid;
	this.start = start;
	this.serial = serial;
	this.tpage = tpage;
	this.pno = pno;
	this.prevgid = (prevgid==0)?gid:prevgid;
	this.nextgid = (nextgid==0)?gid:nextgid;
}
tconWriter.prototype.h = function() {
	var ret = '<div class="tnav smaller"><span style="float:right">';
	if(isBM(this.bid))
	{
		ret += '[<a href="bbsthreads.php?board=' + this.board + '&gid=' + this.gid + '&start=' + this.start + '">同主题操作</a>] ';
	}
	ret += '[<a href="bbstcon.php?board=' + this.board + '&gid=' + this.prevgid + '">上一主题</a>] ';
	ret += '[<a href="bbstcon.php?board=' + this.board + '&gid=' + this.nextgid + '">下一主题</a>]';
	ret += '</span>【分页： ';
	var u = 'bbstcon.php?board=' + this.board + '&gid=' + this.gid + '&start=' + this.start + '&pno=';
	for(var j = 1; j < this.tpage + 1; j ++ ) {
		if (this.pno != j) {
			ret += '<a href="' + u + j + '"><u>' + j + '</u></a> ';
		} else {
			ret += '<b>' + j + '</b> ';
		}
	}
	ret += '】';
	if (this.pno < this.tpage) ret += '<a href="' + u + (this.pno+1) + '">下一页</a> ';
	ret += '</div>';
	w(ret);
};
tconWriter.prototype.o = function(arts) {
	var ifs = "";
	var ids = new Array();
	for (var i = 0; i < arts.length; i++) {
		var id = arts[i][0];
		var owner = arts[i][1];
		ids.push(id);
		var url = 'bbscon.php?bid=' + this.bid + '&id=' + id;
		var ret = '<br/>';
		ret += '<div class="tconPager smaller left">';
		ret += '[<a href="' + url + '">本篇全文</a>] ';
		if (isLogin()) {
			ret += '[<a href="bbspst.php?board=' + this.board + '&reid=' + id + '">回复文章</a>] ';
		}
		ret += '[本篇作者：<a href="bbsqry.php?userid=' + owner + '">' + owner + '</a>] ';
		if (isLogin()) {
			ret += '[<a href="bbspstmail.php?board=' + this.board + '&id=' + id + '">回信给作者</a>] ';
		}
		ret += '[<a href="bbsdoc.php?board=' + this.board + '">进入讨论区</a>] ';
		ret += '[<a href="#top">返回顶部</a>]';
		ret += '<div class="tnum">' + (this.serial+i+1) + '</div>';
		ret += '</div><div class="article" id="art' + id + '"><div align="center">...载入中...</div></div>';
		ifs += '<iframe width=0 height=0 frameborder="0" scrolling="no" src="' + url + '"></iframe>';
		w(ret);
	}
	w('<div id="kbsrcInfo">tcon,' + this.bid + ',' + ids.join(',') + '</div>');
	w(ifs);
};




function brdWriter(father, select, fix) {
	var ret = '<table class="main wide">';
	if (arguments.length == 2) { //bbsboa.php
		fix = true;
		ret += '<col width="2%"/><col width="2%"/><col width="24%"/><col width="10%"/><col width="31%"/><col width="15%"/><col width="8%"/><col width="8%"/>';
		ret += '<tr><th>#</th><th> </th><th>讨论区名称</th><th>类别</th><th>中文描述</th><th>版主</th><th>篇数</th><th>在线</th></tr>';
		if (select > 0) {
			ret += '<tr><td> </td><td> ' + putImageCode('groupgroup.gif','alt="up" title="回到上一级"') + '</td>';
			ret += '<td colspan="6"><a href="bbsboa.php?group=' + father + '">回到上一级</a></td></tr>';
		}
	} else {
		ret += '<col width="2%"/><col width="2%"/><col width="23%"/><col width="10%"/><col width="27%"/><col width="14%"/><col width="8%"/><col width="6%"/><col width="6%"/>';
		ret += '<tr><th>#</th><th> </th><th>讨论区名称</th><th>类别</th><th>中文描述</th><th>版主</th><th>篇数</th><th>在线</th>';
		if (!fix) ret += '<th> </th>';
		ret += '</tr>';
		if (select != 0) {
			ret += '<tr><td> </td><td> ' + putImageCode('groupgroup.gif','alt="up" title="回到上一级"') + '</td>';
			ret += '<td colspan="'+(fix?6:7)+'"><a href="bbsfav.php?select=' + father + fix + '">回到上一级</a></td></tr>';
		}
	}
	w(ret);
	this.father = father;
	this.select = select;
	this.fix = fix;
	this.index = 0;
	this.kbsrc = new Array();
}
brdWriter.prototype.f = function(select, desc, npos, name) {
	this.index++;
	var ret = '<tr class="' + (this.index%2?"even":"odd") + '"><td class="center">' + this.index + '</td>';
	ret += '<td> ' + putImageCode('groupgroup.gif','alt="＋" title="版面组"') + '</td>';
	ret += '<td><a href="bbsfav.php?select=' + select + this.fix + '">' + htmlize(this.fix?name:desc) + '</a></td>';
	if (!this.fix) desc = '';
	ret += '<td class="center">[目录]</td><td colspan="4">' + desc + ' </td>';
	if (!this.fix) ret += '<td class="center"><a href="bbsfav.php?select=' + this.select + '&deldir=' + npos + '">删除</a></td>';
	ret += '</tr>';
	w(ret);
};
brdWriter.prototype.o = function(group, unread, bid, lastpost, cls, name, desc, bms, artcnt, npos, online) {
	this.index++;
	var brdlink, ret = '<tr class="' + (this.index%2?"even":"odd") + '"><td class="center">' + this.index + '</td>';
	if (group) {
		ret += '<td>' + putImageCode('groupgroup.gif','alt="＋" title="版面组"') + '</td>';
	} else {
		var unread_tag = (unread ? "" : ' style="display: none"') + ' id="kbsrc' + bid + 'u"';
		var read_tag = (!unread ? "" : ' style="display: none"') + ' id="kbsrc' + bid + 'r"';
		ret += '<td>' + putImageCode('newgroup.gif','alt="◆" title="未读标志"' + unread_tag);
		ret += putImageCode('oldgroup.gif','alt="◇" title="已读标志"' + read_tag) + '</td>';
		this.kbsrc.push(bid);
		this.kbsrc.push(lastpost); 
	}
	brdlink = 'bbsdoc.php?board=' + escape(name);
	ret += '<td>&nbsp;<a href="' + brdlink + '">' + htmlize(name) + '</a></td>';
	ret += '<td class="center">' + htmlize(cls) + '</td>';
	if (group) {
		ret += '<td colspan="3">&nbsp;&nbsp;<a href="' + brdlink + '">' + htmlize(desc) + '</a>[目录]</td>';
	} else {
		ret += '<td>&nbsp;&nbsp;<a href="' + brdlink + '">' + htmlize(desc) + '</a></td>';
		ret += '<td class="center">' + writeBM_html(bms, true) + '</td>';
		ret += '<td class="right">' + artcnt + '</td>';
	}
	ret += '<td class="right">' + online + '</td>';
	if (!this.fix) ret += '<td class="center"><a href="bbsfav.php?select=' + this.select + '&delete=' + npos + '">删除</a></td>';
	ret += '</tr>';
	w(ret);
};
brdWriter.prototype.t = function() {
	w('<div id="kbsrcInfo">brd,' + this.kbsrc.join(',') + '</div></table>');
};

function tabWriter(num, tabC, caption, header) {
	/* header[i][0]: title, header[i][1]: width, header[i][2]: class */
	this.header = header;
	if (typeof header == "string") {
		switch (header) {
			case 'boards':
				break;
		}
	}
	this.num = num;
	this.row = 0;
	var ret = '<table class="' + tabC + '">';
	if (caption) ret += '<caption>' + caption + '</caption>';
	for(var i = 0; i < header.length; i++) {
		ret += '<col';
		if (header[i][1]) ret += ' width="' + header[i][1] + '"';
		ret += '/>';
	}	
	ret += '<tr>';
	for(i = 0; i < header.length; i++) {
		ret += '<th>' + header[i][0] + '</th>';
	}
	ret += '</tr><tbody>';
	w(ret);
}
tabWriter.prototype.pr = function(col, content) {
	var ret = '<td';
	var c = this.header[col];
	if (c) {
		if (c[2]) ret += ' class="' + c[2] + '"';
	}
	ret += '>' + content + '</td>';
	return ret;
};
tabWriter.prototype.r = function() {
	var ret = '<tr class="' + (((this.row++)%2)?'even':'odd') + '">';
	var i = 0, j = 0;
	if (this.num) {
		ret += this.pr(0, this.row);
		j++;
	}
	for(i = 0; i < arguments.length; i++,j++) {
		ret += this.pr(j, arguments[i]);
	}
	ret += '</tr>';
	w(ret);
};
tabWriter.prototype.t = function() {
	w("</tbody></table>");
};
