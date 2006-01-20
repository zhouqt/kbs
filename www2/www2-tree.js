/**
 * Part of the KBS BBS Code
 * Copyright (c) 2005-2006 KBS Development Team. (http://dev.kcn.cn/)
 * Source file is subject to the pending KBS License.
 *
 * You may use and/or modify the source code only for pure personal study
 * purpose (e.g. using it in a public website is not acceptable), unless
 * you get explicit permission by the KBS Development Team.
 */

var gTreeArts = new Array();
function treeWriter(board, gid, arts) {
	this.board = escape(board);
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
	var url = 'bbscon.php?board=' + this.board + '&id=' + id;
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

