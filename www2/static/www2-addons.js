/**
 * Part of the KBS BBS Code
 * Copyright (c) 2005-2006 KBS Development Team. (http://dev.kcn.cn/)
 * Source file is subject to the pending KBS License.
 *
 * You may use and/or modify the source code only for pure personal study
 * purpose (e.g. using it in a public website is not acceptable), unless
 * you get explicit permission by the KBS Development Team.
 */

function annWriter(path, perm_bm, text, title) {
	this.path = path;
	this.perm_bm = perm_bm;
	this.num = 1;
	this.title = title;
	var str;
	str = '<form id="frmAnnounce" action="bbs0anbm.php?path=' + path + '" method="post">';
	str += '<input type="hidden" id="annAction" name="annAction" value="">';
	str += '<div class="smaller" style="text-align:right">';
	if(text == '')
	{
		str += '精华区管理模式：您在当前目录<span style="color:#FF0000">';
		str += perm_bm ? '有' : '没有';
		str += '</span>管理权限。';
	}
	else
		str += '<span style="color:#FF0000">' + text + '</span>';
	str += '</div>';
	str += '<table class="main wide"><col width="5%" /><col width="8%" /><col width="4%" /><col width="38%" />';
	str += '<col width="10%" /><col width="10%" /><col width="10%" /><tr><th>#</th><th>类型</th><th></th>';
	str += '<th>标题</th><th>版主</th><th>' + (perm_bm?'文件名':'日期') + '</th><th>操作</th></tr><tbody>';
	document.write(str);
}
annWriter.prototype.i = function(type, title, bm, filename, date) {
	var str, itempath;
	str = '<tr><td class="center">' + this.num + '</td><td class="center">';
	switch(type) {
		case 0:
			str += '错误';
			break;
		case 1:
			str += '目录';
			break;
		case 2:
		case 3:
		default:
			str += '文件';
			break;
	}
	str += '</td><td class="center"><input type="checkbox" name="ann' + this.num + '" value="' + filename + '"></td><td>';
	itempath = this.path + '%2F' + filename;
	if (type == 1)
		str += '<a href="bbs0anbm.php?path=' + itempath + '">';
	else if (type >= 2)
		str += '<a href="bbsanc.php?annbm=1&path=' + itempath + '">';
	str += title + '</a></td><td>' + bm + '</td><td>';
	str += this.perm_bm ? (filename + ((type == 1) ? '/' : '')) : date;
	str += '</td><td>';
	if (type == 1)
		str += '<a href="bbs0anbm_editdir.php?path=' + itempath + '&title=' + title + '&bm=' + bm + '">修改</a>';
	else if (type >= 2)
		str += '<a href="bbs0anbm_editfile.php?path=' + itempath + '&title=' + title + '">编辑</a>';
	str += ' <a href="javascript:ann_move(' + this.num + ');">调序</a>';
	str += '<span id="divam' + this.num + '"></span>';
	str += '</td></tr>';
	document.write(str);
	this.num++;
};
annWriter.prototype.f = function() {
	var str;
	str = '</tbody></table>';
	if (this.perm_bm)
	{
		str += '<br><div class="center smaller">';
		str += '[<a href="bbs0anbm_mkdir.php?path=' + this.path + '">创建目录</a>] ';
		str += '[<a href="bbs0anbm_mkfile.php?path=' + this.path + '">创建文件</a>] ';
		str += '[<a href="javascript:ann_clip(\'cut\');">剪切</a>] ';
		str += '[<a href="javascript:ann_clip(\'copy\');">复制</a>] ';
		str += '[<a href="javascript:ann_clip(\'paste\');">粘贴</a>] ';
		str += '[<a href="javascript:ann_delete();">删除</a>] ';
		str += '[<a href="bbsipath.php?inann=1&annpath=' + this.path + '&title=' + this.title + '">丝路</a>]';
		str += '</div>';
	}
	str += '<input type="hidden" id="annCount" name="annCount" value="' + (this.num-1) + '">';
	str += '</form>';
	document.write(str);
};

function ann_delete()
{
	if(confirm('确定要删除这些文件或目录吗？'))
	{
		frmAnnounce.annAction.value = 'delete';
		frmAnnounce.submit();
	}
}
function ann_clip(action)
{
	frmAnnounce.annAction.value = action;
	frmAnnounce.submit();
}
function ann_move(num)
{
	var str = '';
	str += '<br>新序<input type="text" size="3" name="newnum">';
	str += '<input type="hidden" name="oldnum" value="' + num + '">';
	str += '<br><input type="button" value="移动" onclick="ann_move_do();">';
	str += '<input type="button" value="取消" onclick="ann_move_cancel(' + num + ');">';
	document.getElementById('divam' + num).innerHTML = str;
}
function ann_move_do()
{
	frmAnnounce.annAction.value = 'move';
	frmAnnounce.submit();
}
function ann_move_cancel(num)
{
	var thediv = document.getElementById('divam' + num);
	thediv.innerHTML = '';
}

function ipathMakeItem(title, path)
{
	return(title + '<br>' + path);
}
function ipathWriter(inAnn) {
	var str;
	this.num = 1;
	this.annPath = currAnnPath;
	this.inAnn = inAnn;
	str = '<form id="frmipath" method="post" action="bbsipath.php?inann=' + inAnn + '&annpath=' + currAnnPath + '&title=' + currAnnTitle + '">';
	str += '<input type="hidden" id="ipathAction" name="ipathAction" value="">';
	str += '<input type="hidden" id="num" name="num" value="0">';
	str += '<h1 class="bt">丝路</h1><div style="text-align:right">[<a href="bbs0anbm.php?path=' + currAnnPath + '">精华区目录</a>]';
	str += '<table class="main wide"><col width="5%" /><col width="60%" /><col width="35%" />';
	str += '<tr><th>#</th><th>标题 / 路径</th><th>操作</th></tr><tbody>';
	document.write(str);
}
ipathWriter.prototype.i = function(title, path) {
	var str;
	ititle[this.num] = title;
	ipath[this.num] = path;
	str = '<tr><td class="center">' + this.num + '</td>';
	str += '<td id="ipathCon' + this.num + '">' + ipathMakeItem(title, path) + '</td>';
	str += '<td>';
	if(title != '')
	{
		str += '<a href="javascript:ipathPaste(\'' + path + '\');">粘贴</a> ';
		str += '<a href="javascript:ipathModify(' + this.num + ');">改标题</a> ';
	}
	if(this.inAnn)
		str += '<a href="javascript:ipathSet(' + this.num + ');">设为当前目录</a>';
	str += '</td></tr>';
	this.num++;
	document.write(str);
}
ipathWriter.prototype.f = function() {
	var str;
	str = '</tbody></table><div id="ipathSetDiv"></div></form>';
	str += '<form id="frmPaste" method="post"><input type="hidden" name="annAction" value="paste"><input type="hidden" name="annCount" value="0"></form>';
	document.write(str);
}

function ipathPaste(path)
{
	path = path.substr(9, path.length - 9);
	frmPaste.action = 'bbs0anbm.php?path=' + path;
	frmPaste.submit();
}
function ipathModify(num)
{
	var str;
	if(ipathEditing	!= 0)
		document.getElementById('ipathCon' + ipathEditing).innerHTML = ipathMakeItem(ititle[ipathEditing], ipath[ipathEditing]);
	if(num != ipathEditing)
	{
		ipathEditing = num;
		if(num > 0)
		{
			str = '<input type="text" name="ipathTitle" size="40" maxlength="80" value="' + ititle[num] + '">';
			str += '<input type="button" onclick="ipathDoModify();" value="修改"><br>' + ipath[num];
			document.getElementById('ipathCon' + num).innerHTML = str;
		}
	}
	else
		ipathEditing = 0;
		
}
function ipathDoModify()
{
	if(ipathEditing != 0)
	{
		frmipath.ipathAction.value = 'modify';
		frmipath.num.value = ipathEditing;
		frmipath.submit();
	}
}
function ipathSet(num)
{
	var co = true;
	var str;
	ipathModify(0);
	if(ititle[num] != "")
		co = confirm('要覆盖这个丝路吗？');
	if(co)
	{
		frmipath.ipathAction.value = 'set';
		frmipath.num.value = num;
		str = '<input type="hidden" name="ipathTitle" value="' + currAnnTitle + '">';
		str += '<input type="hidden" name="ipathPath" value="' + currAnnPath + '">';
		document.getElementById('ipathSetDiv').innerHTML = str;
		frmipath.submit();
	}
}


	





var gTreeArts = new Array();
function treeWriter(board, bid, gid, arts) {
	this.board = escape(board);
	this.bid = bid;
	this.gid = gid;
	var i, tI = new Array();
	for (i = 0; i < arts.length; i++) {
		var node = {"id": arts[i][0], "reid": arts[i][1], "owner": arts[i][2], 
			"first_child": -1, "last_child": -1, "next_sibling": -1, "showed": false};
		gTreeArts[i] = node;
		tI[node.id] = i + 1;
		if (i > 0 && tI[node.reid]) {
			var parent = gTreeArts[tI[node.reid] - 1];
			if (parent.first_child == -1) parent.first_child = i;
			if (parent.last_child != -1) gTreeArts[parent.last_child].next_sibling = i;
			parent.last_child = i;
		}
	}
	this.ifs = "";
}
treeWriter.prototype.s = function(idx, flag) { /* flag: -1: root, 1: last */
	if (gTreeArts[idx].showed) return;
	gTreeArts[idx].showed = true;
	var id = gTreeArts[idx].id;
	var owner = gTreeArts[idx].owner;
	var url = 'bbscon.php?bid=' + this.bid + '&id=' + id;
	var ret = '<br/>';
	var c = "treeFold";
	if (flag == -1) c = "treeFoldRoot";
	else if (flag == 1) c = "treeFoldLast";
	
	ret += '<div class="' + c + '">';
	if (flag == 0) {
		ret += '<div class="treeFoldLeaf"> </div>';
	}
	ret += '<div class="tconPager smaller left">';
	ret += '[<a href="' + url + '">本篇全文</a>] ';
	if (isLogin()) {
		ret += '[<a href="bbspst.php?board=' + this.board + '&reid=' + id + '">回复文章</a>] ';
		ret += '[<a href="bbspstmail.php?board=' + this.board + '&id=' + id + '">回信给作者</a>] ';
	}
	ret += '[本篇作者：<a href="bbsqry.php?userid=' + owner + '">' + owner + '</a>] ';
	ret += '[<a href="bbsdoc.php?board=' + this.board + '">进入讨论区</a>] ';
	ret += '[<a href="#top">返回顶部</a>]';
	ret += '<div class="tnum">' + (idx+1) + '</div>';
	ret += '</div><div class="article" id="art' + id + '"><div align="center">...载入中...</div></div>';
	this.ifs += '<iframe width=0 height=0 frameborder="0" scrolling="no" src="' + url + '"></iframe>';
	document.write(ret);

	var cur = gTreeArts[idx].first_child;
	while(cur != -1) {
		this.s(cur, (cur == gTreeArts[idx].last_child) ? 1 : 0);
		cur = gTreeArts[cur].next_sibling;
	}
	
	document.write("</div>");
};
treeWriter.prototype.o = function() {
	var i;
	this.s(0, -1);
	for(i=1;i<gTreeArts.length;i++) this.s(i, -1); //没连上根的那些枝条
	document.write(this.ifs);
};






function AnsiState() {
    this.s = 0;
}
AnsiState.prototype = {
	ESC_SET: 0x01,
	FONT_SET: 0x02,
	NEW_LINE: 0x04,
	QUOTE_LINE: 0x08,
	QUOTEHEADER_LINE: 0x10,
	SET: function(b) { this.s |= b; },
	CLR: function(b) { this.s &= ~b; },
	ISSET: function(b) { return (this.s & b); },
	ZERO: function() { this.s = 0; }
};

function StyleState() {
    this.s = 0;
    this.ansi = new Array();
    this.ansi_t = 0;
}
StyleState.prototype = {
	SET_FG: function(c) { this.s = (this.s & ~0x07) | (c & 0x07) },
	SET_BG: function(c) { this.s = (this.s & ~0x70) | ((c & 0x07) << 4) },
	GET_FG: function() { return (this.s & 0x0F); },
	GET_BG: function() { return ((this.s & 0x70) >> 4); },
	CLR_FG: function() { this.s &= ~0x0F; },
	CLR_BG: function() { this.s &= ~0xF0; },
	ZERO: function() { this.s = 0; },
	SET: function(b) { this.s |= b; },
	CLR: function(b) { this.s &= ~b; },
	ISSET: function(b) { return (this.s & b); },
	STYLE_UL: 0x0100,
	STYLE_BLINK: 0x0200,
	STYLE_ITALIC: 0x0400,
	FG_BOLD: 0x08,
	COLOR_BLACK: 0x00,
	COLOR_RED: 0x01,
	COLOR_GREEN: 0x02,
	COLOR_YELLOW: 0x03,
	COLOR_BULE: 0x04,
	COLOR_MAGENTA: 0x05,
	COLOR_CYAN: 0x06,
	COLOR_WHITE: 0x07,
	STYLE_QUOTE: 0x0000,
	COLOR_QUOTE: 0x06,
	STYLE_QUOTEHEADER: 0x0800,
	COLOR_QUOTEHEADER: 0x03|0x08,
	BG_SET: 0x80,
	printStyle: function() {
	    var bg, fg, font_class;
	    if (this.ISSET(this.BG_SET)) {
	        bg = 8;
	    } else {
	        bg = this.GET_BG();
	    }
	    fg = this.GET_FG();
	    if (fg == 0 && bg == 0) {
	        font_class = "ff07";
	    } else {
	        font_class = "fb" + bg + " ff" + fg;
	    }
	    if (this.ISSET(this.STYLE_UL))
	        font_class += " fund";
	    if (this.ISSET(this.STYLE_ITALIC)) /* ?? no where set this */
	        font_class += " fita";
	    if (this.ISSET(this.STYLE_BLINK))
	        font_class += " fbli";
	    return "<font class=\"" + font_class + "\">";
	},
	addAnsi: function() {
	    this.ansi_t++;
	    this.ansi[this.ansi_t] = 0;
	},
	clearAnsi: function() {
	    this.ansi_t = 0;
	    this.ansi[0] = 0;
	},
	generateFontStyle: function() {
	    for (var i = 0; i <= this.ansi_t; i++) {
	        if (this.ansi[i] == 0) {
	            this.ZERO();
	            this.SET(this.COLOR_WHITE);
	        } else if (this.ansi[i] == 1)
	            this.SET(this.FG_BOLD);
	        else if (this.ansi[i] == 4)
	            this.SET(this.STYLE_UL);
	        else if (this.ansi[i] == 5)
	            this.SET(this.STYLE_BLINK);
	        else if (this.ansi[i] >= 30 && this.ansi[i] <= 37) {
	            this.SET_FG(this.ansi[i] - 30);
	        } else if (this.ansi[i] >= 40 && this.ansi[i] <= 47) {
	            this.SET_BG(this.ansi[i] - 40);
	        }
	    }
	}
};

function htmlOutput(s) {
    switch(s) {
        case '&':
            return "&amp;";
        case '<':
            return "&lt;";
        case '>':
            return "&gt;";
        case ' ':
            return "&nbsp;";
        default:
            return s;
    }
}

function printRawAnsi(s, start, end) {
    var ret = "";
    for (var i = start; i < end; i++) {
        if (s.charAt(i) == '\r')
            ret += "*";
        else if (s.charAt(i) == '\n')
            ret += "<br/>";
        else
            ret += htmlOutput(s.charAt(i));
    }
    return ret;
}

function convertAnsi(s) {
    var ansiState = new AnsiState();
    var styleState = new StyleState();
    var ret = new StringBuffer();
    var buflen = s.length;
    var ansi_begin = 0;
    styleState.clearAnsi();
    styleState.ZERO();
    styleState.SET(styleState.COLOR_WHITE);

    for (var i = 0; i < buflen; i++) {
        if (ansiState.ISSET(ansiState.NEW_LINE)) {
            ansiState.CLR(ansiState.NEW_LINE);
            if (i < (buflen - 1) && (s.charAt(i) == ':' && s.charAt(i + 1) == ' ')) {
                ansiState.SET(ansiState.QUOTE_LINE);
                if (ansiState.ISSET(ansiState.FONT_SET))
                    ret.append("</font>");
                /*
                 * set quoted line styles 
                 */
                styleState.SET(styleState.STYLE_QUOTE);
                styleState.CLR_FG();
                styleState.CLR_BG();
                styleState.SET(styleState.COLOR_QUOTE);
                ret.append(styleState.printStyle() + ":");
                ansiState.SET(ansiState.FONT_SET);
                ansiState.CLR(ansiState.ESC_SET);
                /*
                 * clear ansi_val[] array 
                 */
                styleState.clearAnsi();
                continue;
            } else
                ansiState.CLR(ansiState.QUOTE_LINE);

            if (i < (buflen - 3) && (s.substring(i, i + 3) == "【 在")) {
                ansiState.SET(ansiState.QUOTEHEADER_LINE);
                if (ansiState.ISSET(ansiState.FONT_SET))
                    ret.append("</font>");
                /*
                 * set quote header line styles 
                 */
                styleState.SET(styleState.STYLE_QUOTEHEADER);
                styleState.CLR_FG();
                styleState.CLR_BG();
                styleState.SET(styleState.COLOR_QUOTEHEADER);
                ret.append(styleState.printStyle() + "【");
                ansiState.SET(ansiState.FONT_SET);
                ansiState.CLR(ansiState.ESC_SET);
                /*
                 * clear ansi_val[] array 
                 */
                styleState.clearAnsi();
                continue;
            } else
                ansiState.CLR(ansiState.QUOTEHEADER_LINE);
        }
        if (i < (buflen - 1) && (s.charAt(i) == '\r' && s.charAt(i + 1) == '[')) {
            if (ansiState.ISSET(ansiState.ESC_SET)) {
                /*
                 *[*[ or *[13;24*[ */
                ret.append(printRawAnsi(s, ansi_begin, i));
            }
            ansiState.SET(ansiState.ESC_SET);
            ansi_begin = i;
            i++;                /* skip the next '[' character */
        } else if (s.charAt(i) == '\n') {
            if (ansiState.ISSET(ansiState.ESC_SET)) {
                /*
                 *[\n or *[13;24\n */
                ret.append(printRawAnsi(s, ansi_begin, i));
                ansiState.CLR(ansiState.ESC_SET);
            }
            if (ansiState.ISSET(ansiState.QUOTE_LINE)) {
                /*
                 * end of a quoted line 
                 */
                ret.append("</font>");
                styleState.CLR(styleState.STYLE_QUOTE);
                styleState.CLR(styleState.COLOR_QUOTE);
                ansiState.CLR(ansiState.QUOTE_LINE);
                ansiState.CLR(ansiState.FONT_SET);
            }
            if (ansiState.ISSET(ansiState.QUOTEHEADER_LINE)) {
                /*
                 * end of a quote header line 
                 */
                ret.append("</font>");
                styleState.CLR(styleState.STYLE_QUOTEHEADER);
                styleState.CLR(styleState.COLOR_QUOTEHEADER);
                ansiState.CLR(ansiState.QUOTEHEADER_LINE);
                ansiState.CLR(ansiState.FONT_SET);
            }
            ret.append("<br/>");
            ansiState.SET(ansiState.NEW_LINE);
        } else {
            var c = s.charAt(i);
            if (ansiState.ISSET(ansiState.ESC_SET)) {
                if (c == 'm') {
                    /*
                     *[0;1;4;31m */
                    if (ansiState.ISSET(ansiState.FONT_SET)) {
                        ret.append("</font>");
                        ansiState.CLR(ansiState.FONT_SET);
                    }
                    if (i < buflen - 1) {
                        styleState.generateFontStyle();
                        ret.append(styleState.printStyle());
                        ansiState.SET(ansiState.FONT_SET);
                        ansiState.CLR(ansiState.ESC_SET);
                        /*
                         * STYLE_ZERO(font_style);
                         */
                        /*
                         * clear ansi_val[] array 
                         */
                        styleState.clearAnsi();
                    }
                } else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
                    /*
                     *[23;32H */
                    /*
                     * ignore it 
                     */
                    ansiState.CLR(ansiState.ESC_SET);
                    //styleState.ZERO();
                    /*
                     * clear ansi_val[] array 
                     */
                    styleState.clearAnsi();
                    continue;
                } else if (c == ';') {
                    styleState.addAnsi();
                } else if (c >= '0' && c <= '9') {
                    styleState.ansi[styleState.ansi_t] *= 10;
                    styleState.ansi[styleState.ansi_t] += (c - '0');
                } else {
                    /*
                     *[1;32/XXXX or *[* or *[[ */
                    /*
                     * not a valid ANSI string, just output it 
                     */
                    ret.append(printRawAnsi(s, ansi_begin, i + 1));
                    ansiState.CLR(ansiState.ESC_SET);
                    /*
                     * clear ansi_val[] array 
                     */
                    styleState.clearAnsi();
                }
            } else
                ret.append(printRawAnsi(s, i, i + 1));
        }
    }
    if (ansiState.ISSET(ansiState.FONT_SET)) {
        ret.append("</font>");
        ansiState.CLR(ansiState.FONT_SET);
    }
    return ret.toString();
}

/* WARNING: now I can only deal with only one ansi container in a page */
var oldPrints, strAnsi = '';
function triggerAnsiDiv(obj,objInner) {
	if (!((gIE && !gIE5) || gFx)) return;
	if (!(obj = getObj(obj))) return;
	if (!(objInner = getObj(objInner))) return;
	oldPrints = prints;
	divArtCon = objInner;
	prints = function(s) { strAnsi += s; oldPrints(s); };
	addBootFn(function() {
		objInner.innerHTML = strArticle;
		var o = document.createElement("div");
		o.className = "AnsiSwitch";
		o.innerHTML = "ANSI";
		o.title = "切换 Ansi 显示";
		obj.insertBefore(o,objInner);
		obj.isAnsi = false;
		addEvent(o,"click",function() {
			obj.isAnsi = !obj.isAnsi;
			obj.className = obj.isAnsi?"AnsiArticleColor":"AnsiArticleBW";
			objInner.innerHTML = obj.isAnsi?convertAnsi(strAnsi):strArticle;
		});
	});
}
