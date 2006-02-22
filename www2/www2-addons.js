/**
 * Part of the KBS BBS Code
 * Copyright (c) 2005-2006 KBS Development Team. (http://dev.kcn.cn/)
 * Source file is subject to the pending KBS License.
 *
 * You may use and/or modify the source code only for pure personal study
 * purpose (e.g. using it in a public website is not acceptable), unless
 * you get explicit permission by the KBS Development Team.
 */

function annWriter(path, perm_bm) {
	this.path = path;
	this.perm_bm = perm_bm;
	this.num = 1;
	var str;
	str = '<form id="frmAnnounce" action="bbs0anbm.php?path=' + path + '" method="post">';
	str += '<div class="smaller" style="float:right">精华区管理模式：您在当前目录<span style="color:#FF0000">';
	str += perm_bm ? '有' : '没有';
	str += '</span>管理权限。</div>';
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
		str += '<a href="bbsanc.php?path=' + itempath + '">';
	str += title + '</a></td><td>' + bm + '</td><td>';
	str += this.perm_bm ? (filename + ((type == 1) ? '/' : '')) : date;
	str += '</td><td>';
	if (type == 1)
		str += '<a href="bbs0anbm_editdir.php?path=' + itempath + '">修改</a>';
	else if (type >= 2)
		str += '<a href="bbs0anbm_editfile.php?path=' + itempath + '">编辑</a>';
	str += ' <a href="#">调序</a>';
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
		str += '[<a href="javascript:ann_cutcopy(0);">剪切</a>] ';
		str += '[<a href="javascript:ann_cutcopy(1);">复制</a>] ';
		str += '[<a href="javascript:ann_paste();">粘贴</a>] ';
		str += '[<a href="javascript:ann_delete();">删除</a>]';
	}
	str += '</form>';
	document.write(str);
};