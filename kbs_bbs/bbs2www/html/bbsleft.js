function changemn(mn) {
	o = document.getElementById("div" + mn);
	m = document.getElementById("img" + mn);
	if (o == null) return;
	if (o.style.display == 'block') {
		o.style.display = 'none';
		if (m) m.src = 'images/close.gif';
	} else {
		o.style.display = 'block';
		if (m) m.src = 'images/open.gif';
	}
}

function openchat()
{
	url='bbschat';
	chatWidth=screen.availWidth;
	chatHeight=screen.availHeight;
	winPara='toolbar=no,location=no,status=no,menubar=no,scrollbars=auto,resizable=yes,left=0,top=0,'+'width='+(chatWidth-10)+',height='+(chatHeight-27);
	window.open(url,'_blank',winPara);
}

function openreg()
{
	open('bbsreg', '', 'width=600,height=460,resizable=yes,scrollbars=yes');
}

function submenu( isfav , favselect , group , group2 , yank )
{
	if( isfav == 1 )
	{
		linkurl = "/bbsleft-submenu.php?select=" + favselect + "&fav=1";
		submenuname = "submenu_fav_" + favselect ;
		imgname = "submenuimg_fav_" + favselect ;
	}
	else
	{
		linkurl = "/bbsleft-submenu.php?group=" + group + "&group2=" + group2 + "&yank=" + yank;
		submenuname = "submenu_brd_" + group + "_" + group2 ;
		imgname = "submenuimg_brd_" + group + "_" + group2 ;
	}
	submenustatus = document.getElementById(submenuname).style.display;
	if (submenustatus == "block")
	{
		document.getElementById(imgname).src="/images/close.gif";
		document.getElementById(submenuname).style.display = "none";
	} else {
		document.getElementById(imgname).src="images/open.gif";
		document.getElementById(submenuname).style.display = "block";
		document.getElementById(submenuname).innerHTML="<div class='b1'>‘ÿ»Î÷–°≠°≠</div>";
		window.frames["hiddenframe"].document.location.href = linkurl ;
	}
}

function fillf3(form) {
	if (f = top.f3) {
		url = f.document.location.toString();
		uri = url.substr(7);
		if ((pos = uri.indexOf("/")) != -1) {
			url = uri.substr(pos + 1);
		}
		form.action += "?mainurl=" + escape(url);
	}
	return true;
}