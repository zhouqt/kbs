/**
 * Part of the KBS BBS Code
 * Copyright (c) 2005-2006 KBS Development Team. (http://dev.kcn.cn/)
 * Source file is subject to the pending KBS License.
 *
 * You may use and/or modify the source code only for pure personal study
 * purpose (e.g. using it in a public website is not acceptable), unless
 * you get explicit permission by the KBS Development Team.
 */

function annWriter(path, perm_bm, text) {
	this.path = path;
	this.perm_bm = perm_bm;
	this.num = 1;
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
		// str += '[<a href="bbsipath.php?inann=1&annpath=' + this.path + '">丝路</a>]';
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
