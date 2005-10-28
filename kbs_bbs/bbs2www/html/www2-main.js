function getCookie(name){
	var cname = name + "="; 
	var dc = document.cookie; 
	if (dc.length > 0) { 
		begin = dc.indexOf(cname); 
		if (begin != -1) { 
			begin += cname.length; 
			end = dc.indexOf(";", begin);
			if (end == -1) end = dc.length;
			return unescape(dc.substring(begin, end));
		} 
	}
	return null;
}

function queryString(Param, Default) 
{
	 //  Extracts and returns the parameter value from the URL for the specified parameter.
	 //  If the parameter does not occur is the URL, the default value is returned.
	 //  This function is not Case-sensitive.
	 var sURL = location.search;
	 var sURLUpper = sURL.toUpperCase() + "&";
	 Param = Param.toUpperCase();
	 if (!Default) Default="";

	 var Pos = sURLUpper.indexOf (Param + "=");
	 if (Pos==-1) return Default;
	 Pos = Pos + Param.length + 1;
	 return sURL.substring (Pos, sURLUpper.indexOf("&", Pos));
}


function getObj(n) {
	return document.getElementById(n);
}

var bbsconImg = new Array();
function resizeImg(obj) {
	bbsconImg[bbsconImg.length] = obj;
	obj.o_width = obj.width;
	maxWidth = document.body.clientWidth - 40;
	if (obj.width > maxWidth) obj.width = maxWidth;
}
function adjustImg() {
	var maxWidth = document.body.clientWidth - 40;
	for (var i in bbsconImg) {
		obj = bbsconImg[i];
		o_width = obj.o_width;
		obj.width = (o_width > maxWidth) ? maxWidth : o_width
	}
}
window.onresize = adjustImg;


var agt = navigator.userAgent.toLowerCase();
var is_ie = ((agt.indexOf("msie") != -1) && (agt.indexOf("opera") == -1));
function cancelEvent(ev) {
	if (is_ie) {
		ev.cancelBubble = true;
		ev.returnValue = false;
	} else {
		ev.preventDefault();
		ev.stopPropagation();
	}
}

/* textarea onkeydown event handler. to submit form with ctrl+W(IE only) or ctrl+ENTER */
function textarea_okd(func, evt) {
	var evt = (evt) ? evt : ((window.event) ? event : null);
	if (evt == null) return;
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

function bbs_confirm(url,infor){
	if(confirm(infor)){
		window.location.href=url;
		return true;
	}
	return false;
}

var bootFn = null;
window.onload = function() {
	/* set focus */
	var f = document.getElementById("sfocus");
	if (f) {
		f.focus();
		setCursorPosition(f, 0, 0);
	} else {
		f = document.getElementById("sselect");
		if (f) {
			f.focus(); f.select();
		}
	}
	
	/*
	 * apply col class. this is a workaround for css2.1 - atppp
	 * see also: http://ln.hixie.ch/?start=1070385285&count=1
	 * TODO: consider td's span properties
	 *
	 * also: apply alternative style for rows.
	 */
	var m = document.getElementsByTagName("table");
	for(var i=0;i<m.length;i++) {
		var tab = m[i];
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
	
	if (bootFn) {
		bootFn();
	}
};

/* check required fields when submitting form. required fields is denoted with classname "req" */
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
	};
	var myTimer = new MyTimer();
	myTimer.refresh();
	setInterval(function() {
		myTimer.refresh();
	}, 1000);
	
	if (hasMail) {
		if (typeof top.hasMsgBox == "undefined") {
			top.hasMsgBox = false;
		}
		if (!top.hasMsgBox) {
			top.hasMsgBox = true;
			if(confirm("您有新邮件，现在查收？")) {
				top.f3.location.href = "bbsmailbox.php?path=.DIR&title=%CA%D5%BC%FE%CF%E4";
			}
			top.hasMsgBox = false;
		}
	}
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
		<div class="oper"><input type="submit" value="递交查询结果"/></div>\
	</form>';
	return s;
}

function showFindBox(board) {
	return; /* disable for now */
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
		if (is_ie) {
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
	if (is_ie) {
		div.filters[0].opacity  = 0;
	} else {
		div.style.MozOpacity = 0;
	}
	div.style.display = "";
	getObj("sfocus").focus();
	
	/* TODO: general fade-in fade-out effect control */
	var opa = 0;
	var fn = function() {
		opa += 10;
		if (is_ie) {
			div.filters[0].opacity = opa;
		} else {
			div.style.MozOpacity = opa / 100.0;
		}
		if (opa < 90) {
			setTimeout(fn, 10);
		}
	};
	fn();
	return false;
}

function goAttachWindow(){
	var hWnd = window.open("bbsupload.php","_blank","width=600,height=300,scrollbars=yes");
	if ((document.window != null) && (!hWnd.opener))
	   hWnd.opener = document.window;
	hWnd.focus();
	return false;
}

function dosubmit() {
	document.postform.post.value='发表中，请稍候...';
	document.postform.post.disabled=true;
	document.postform.submit();
}



function hotTopic(board) { /* TODO: no table, use AJAX */
	document.write('<center><table cellspacing="0" cellpadding="5" border="0" width="98%"><tr>' +
				   '<td width="100" align="center">[<span class="red">热门话题</span>]</td>' + 
				   '<td><marquee onmouseover="this.stop()" onmouseout="this.start()">' +
				   '<span id="hotTopics">载入中...</span></marquee></td></tr></table></center>');
	document.write('<iframe width=0 height=0 src="" frameborder="0" scrolling="no" id="hiddenframe" name="hiddenframe"></iframe>');
	bootFn = function() {
		window.frames["hiddenframe"].document.location.href = "bbshot.php?board=" + board;
	};
}



function checkFrame() {
	if (top == self) { /* TODO: use better way */
		var url = document.location.toString();
		var uri = url.substr(7);
		if ((pos = uri.indexOf("/")) != -1) {
			url = uri.substr(pos);
		}
		/* TODO: php or html? */
		var msg = '<a href="frames.html?mainurl=' + escape(url) + '">展开完整界面</a>';
		document.write('<div id="checkframe">' + msg + '</div>');
	}
}


var writeBM_str;

function writeBM_getStr(start) {
	var ret = '';
	for(var i = start; i < writeBM_str.length; i++) {
		if (i > start + 3) {
			break;
		} else {
			var bm = writeBM_str[i];
			ret += ' <a href="bbsqry.php?userid=' + bm + '">' + bm + '</a>';
		}
	}
	if (start > 0) {
		ret += ' <span class="clickable" onclick="writeBM_page(' + (start-1) + ')" title="版主前滚翻">&lt;&lt;</span>';
	} else if (writeBM_str.length > 4) {
		ret += ' <span class="gray">&lt;&lt;</span>';
	}
	if (start < writeBM_str.length - 4) {
		ret += ' <span class="clickable" onclick="writeBM_page(' + (start+1) + ')" title="版主后滚翻">&gt;&gt;</span>';
	} else if (writeBM_str.length > 4) {
		ret += ' <span class="gray">&gt;&gt;</span>';
	}
	return ret;
}

function writeBM_page(start) {
	getObj("idBMs").innerHTML = writeBM_getStr(start);
}

function writeBMs(bmstr) {
	if (typeof bmstr == "string") {
		document.write(' ' + bmstr);
	} else {
		writeBM_str = bmstr;
		document.write('<span id="idBMs">' + writeBM_getStr(0) + '</span>');
	}
}
