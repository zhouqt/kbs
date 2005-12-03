/*
** @author windinsn@smth.org
*/
/*
** getCookie(name)
*/
var wwwparams        = 'WWWPARAMS';
var defaultStyleFile = 'default.css';
var midStyleFile     = 'default-mf.css';
var bigStyleFile     = 'default-bf.css';
var smlStyleFile     = 'default-sf.css';
var bbsStyle;



function getCookie(name)
{
	var cookieFound = false;
	var start = 0;
	var end   = 0;
	var cookieString = document.cookie;
	var i = 0;
	while(i <= cookieString.length){
		start = i;
		end   = start + name.length;
		if(cookieString.substring(start,end)==name){
			cookieFound = true;
			break;
		}
		i++;
	}
	if(cookieFound){
		start = end + 1;
		end = cookieString.indexOf(";",start);
		if(end < start)
			end = cookieString.length;
		return unescape(cookieString.substring(start,end));
	}
	return false;
}
/*
** getBbsStyle()
*/
function getBbsStyleFile()
{
	var bbsStyle = getCookie(wwwparams);
	if(!bbsStyle)
		return defaultStyleFile;
	var styleString = new String(bbsStyle);
	switch(styleString.substring(0,1))
	{
		case "1":
			return smlStyleFile;
			break;
		case "2":
			return midStyleFile;
			break;
		case "3":
			return bigStyleFile;
			break;
		default:
	}
	return defaultStyleFile;
}

function prints(s) {
	document.write(s);
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

function textarea_okd(func, evt) {
	evt = (evt) ? evt : ((window.event) ? event : null);
	if (evt == null) return;
	key = evt.keyCode ? evt.keyCode : evt.charCode;
	if ((key == 87 && evt.ctrlKey) || (key == 13 && evt.ctrlKey)) {
		cancelEvent(evt);
		func();
		return false;
	}
	return true;
}
