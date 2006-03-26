function changemn(mn) {
	var o = document.getElementById("div" + mn);
	var m = document.getElementById("img" + mn);
	if (o == null) return;
	if (o.style.display == 'block') {
		o.style.display = 'none';
		if (m) m.src = 'images/close.gif';
	} else {
		o.style.display = 'block';
		if (m) m.src = 'images/open.gif';
	}
}

function submenu( isfav , favselect , group , group2 , yank )
{
	var linkurl, submenuname, imgname;
	if( isfav == 1 )
	{
		linkurl = "bbsleft-submenu.php?select=" + favselect + "&fav=1";
		submenuname = "submenu_fav_" + favselect ;
		imgname = "submenuimg_fav_" + favselect ;
	}
	else
	{
		linkurl = "bbsleft-submenu.php?group=" + group + "&group2=" + group2 + "&yank=" + yank;
		submenuname = "submenu_brd_" + group + "_" + group2 ;
		imgname = "submenuimg_brd_" + group + "_" + group2 ;
	}
	var submenustatus = document.getElementById(submenuname).style.display;
	if (submenustatus == "block")
	{
		document.getElementById(imgname).src="images/close.gif";
		document.getElementById(submenuname).style.display = "none";
	} else {
		document.getElementById(imgname).src="images/open.gif";
		document.getElementById(submenuname).style.display = "block";
		document.getElementById(submenuname).innerHTML="<div class='b1'>‘ÿ»Î÷–°≠°≠</div>";
		window.frames["hiddenframe"].document.location.href = linkurl ;
	}
}

function fillf3(form) {
	var f = top.f3;
	if (f) {
		var url = getURLPath(f.document.location.toString());
		form.action += "?mainurl=" + escape(url);
	}
	return true;
}
