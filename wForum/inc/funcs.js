
function submitonce(theform){
//if IE 4+ or NS 6+
if (isIE4 || isW3C){
//screen thru every element in the form, and hunt down "submit" and "reset"
	for (i=0;i<theform.length;i++){
		var tempobj=theform.elements[i]
		if(tempobj.type.toLowerCase()=="submit"||tempobj.type.toLowerCase()=="reset")
//disable em
			tempobj.disabled=true
		}
	}
}
function openScript(url, width, height){
	var Win = window.open(url,"openScript",'width=' + width + ',height=' + height + ',resizable=0,scrollbars=no,menubar=no,status=no' );
}

//***********默认设置定义.*********************
tPopWait=50;//停留tWait豪秒后显示提示。
tPopShow=5000;//显示tShow豪秒后关闭提示
showPopStep=20;
popOpacity=99;

//***************内部变量定义*****************
sPop=null;
curShow=null;
tFadeOut=null;
tFadeIn=null;
tFadeWaiting=null;

document.write("<style type=\"text/css\" id=\"defaultPopStyle\">");
document.write(".cPopText {  background-color: #F8F8F5;color:#000000; border: 1px #000000 solid;font-color: font-size: 12px; padding-right: 4px; padding-left: 4px; height: 20px; padding-top: 2px; padding-bottom: 2px; filter: Alpha(Opacity=0)}");
document.write("</style>");
document.write("<div id='dypopLayer' style='position:absolute;z-index:1000' class='cPopText'></div>");


function showPopupText(evt){
	var o;
	if (typeof event == "undefined") {
		return;
		event = evt;
	}
	if (typeof isIE4 == "undefined") return;
	if (!isIE4) {
		o = event.currentTarget;
	} else {
		o = event.srcElement;
	}
	if (o == null) return;
	MouseX=event.x;
	MouseY=event.y;
	if(o.alt!=null && o.alt!=""){o.dypop=o.alt;o.alt=""};
        if(o.title!=null && o.title!=""){o.dypop=o.title;o.title=""};
	if(o.dypop!=sPop) {
		if (isIE4) {
			sPop=o.dypop;
			clearTimeout(curShow);
			clearTimeout(tFadeOut);
			clearTimeout(tFadeIn);
			clearTimeout(tFadeWaiting);	
			if(sPop==null || sPop=="") {
				dypopLayer.innerHTML="";
				dypopLayer.style.filter="Alpha()";
				dypopLayer.filters.Alpha.opacity=0;	
				}
			else {
				if(o.dyclass!=null) popStyle=o.dyclass 
					else popStyle="cPopText";
				curShow=setTimeout("showIt()",tPopWait);
			}
		} else {
			dypopLayer = getRawObject("dypopLayer");
			if(sPop==null || sPop=="") {
				dypopLayer.innerHTML="";
				dypopLayer.style.display="none";
			} else {
				if(o.dyclass!=null) popStyle=o.dyclass 
				else popStyle="cPopText";
				dypopLayer.className=popStyle;
				dypopLayer.innerHTML = sPop;
				popWidth=dypopLayer.clientWidth;
				popHeight=dypopLayer.clientHeight;
				if(MouseX+12+popWidth>document.body.clientWidth) popLeftAdjust=-popWidth-24
					else popLeftAdjust=0;
				if(MouseY+12+popHeight>document.body.clientHeight) popTopAdjust=-popHeight-24
					else popTopAdjust=0;
				dypopLayer.style.left=MouseX+12+document.body.scrollLeft+popLeftAdjust;
				dypopLayer.style.top=MouseY+12+document.body.scrollTop+popTopAdjust;
				dypopLayer.style.display="inline";

			}
		}			
	}
}

function showIt(){
		dypopLayer.className=popStyle;
		dypopLayer.innerHTML=sPop;
		popWidth=dypopLayer.clientWidth;
		popHeight=dypopLayer.clientHeight;
		if(MouseX+12+popWidth>document.body.clientWidth) popLeftAdjust=-popWidth-24
			else popLeftAdjust=0;
		if(MouseY+12+popHeight>document.body.clientHeight) popTopAdjust=-popHeight-24
			else popTopAdjust=0;
		dypopLayer.style.left=MouseX+12+document.body.scrollLeft+popLeftAdjust;
		dypopLayer.style.top=MouseY+12+document.body.scrollTop+popTopAdjust;
		dypopLayer.style.filter="Alpha(Opacity=0)";
		fadeOut();
}

function fadeOut(){
	if(dypopLayer.filters.Alpha.opacity<popOpacity) {
		dypopLayer.filters.Alpha.opacity+=showPopStep;
		tFadeOut=setTimeout("fadeOut()",1);
		}
		else {
			dypopLayer.filters.Alpha.opacity=popOpacity;
			tFadeWaiting=setTimeout("fadeIn()",tPopShow);
			}
}

function fadeIn(){
	if(dypopLayer.filters.Alpha.opacity>0) {
		dypopLayer.filters.Alpha.opacity-=1;
		tFadeIn=setTimeout("fadeIn()",1);
		}
}
document.onmouseover=showPopupText;

function CheckAll(form)  {
	var e;
	for (var i=0;i<form.elements.length;i++)    {
		e = form.elements[i];
		if (e.name != 'chkall')
			e.checked = form.chkall.checked; 
	}
}

//下拉菜单相关代码
 var h;
 var w;
 var l;
 var t;
 var topMar = 1;
 var leftMar = -2;
 var space = 1;
 var isvisible;
 var MENU_SHADOW_COLOR='#999999';//定义下拉菜单阴影色
 var global = window.document
 
 global.fo_currentMenu = null
 global.fo_shadows = new Array

function HideMenu(evt) 
{
 var mX;
 var mY;
 var vDiv;
 var mDiv;
	if (isvisible == true) {
		evt = (evt) ? evt : ((window.event) ? event : null);
		if (evt) {
			evt.cancelBubble = true;
			vSrc = (evt.target) ? evt.target :   ((evt.srcElement) ? evt.srcElement : null);

			if (vSrc.nodeType == 3) {
			   vSrc = vSrc.parentNode;
			}
			oDiv=getRawObject("menuDiv");

			mX = evt.clientX + document.body.scrollLeft;
			mY = evt.clientY + document.body.scrollTop;

			if ((mX < parseInt(oDiv.style.left)) || (mX > parseInt(oDiv.style.left)+getObjectWidth(oDiv)) || (mY < parseInt(oDiv.style.top)-h) || (mY > parseInt(oDiv.style.top)+getObjectHeight(oDiv))){
				hide(oDiv);
				isvisible = false;
			}
		}
	}
}

function ShowMenu_Internal(vMnuCode,tWidth,evt,isTd) {
    evt = (evt) ? evt : ((window.event) ? event : null);
    if (evt) {
		evt.cancelBubble = true;
	   vSrc = (evt.target) ? evt.target :   ((evt.srcElement) ? evt.srcElement : null);
	   
       if (vSrc.nodeType == 3) {
           vSrc = vSrc.parentNode;
       }
		if (isTd) vMnuCode = "<table id='submenu' cellspacing=1 cellpadding=3 style='width:"+tWidth+"' class=TableBorder1 onmouseout='HideMenu()'><tr height=23><td nowrap class=TableBody1 align=center>" + vMnuCode + "</td></tr></table>";
		oDiv=getRawObject("menuDiv");

		h = vSrc.offsetHeight;
		w = vSrc.offsetWidth;
		l = vSrc.offsetLeft + leftMar+4;
		t = vSrc.offsetTop + topMar + h + space-2;

		vParent = vSrc.offsetParent;
		while (vParent.tagName.toUpperCase() != "BODY")
		{
			l += vParent.offsetLeft;
			t += vParent.offsetTop;
			vParent = vParent.offsetParent;
		}
		oDiv.innerHTML = vMnuCode;
		shiftTo(oDiv,l,t)
		show(oDiv);
		isvisible = true;
		if( isIE4 ){
			makeRectangularDropShadow(submenu, MENU_SHADOW_COLOR, 4)
		}
	}
	return false;
}
function ShowMenu(a,b,c) {
	ShowMenu_Internal(a,b,c,true);
}

function makeRectangularDropShadow(el, color, size)
{
	var i;
	for (i=size; i>0; i--)
	{
		var rect = document.createElement('div');
		var rs = rect.style
		rs.position = 'absolute';
		rs.left = (el.style.posLeft + i) + 'px';
		rs.top = (el.style.posTop + i) + 'px';
		rs.width = el.offsetWidth + 'px';
		rs.height = el.offsetHeight + 'px';
		rs.zIndex = el.style.zIndex - i;
		rs.backgroundColor = color;
		var opacity = 1 - i / (i + 1);
		rs.filter = 'alpha(opacity=' + (100 * opacity) + ')';
		el.insertAdjacentElement('afterEnd', rect);
		global.fo_shadows[global.fo_shadows.length] = rect;
	}
}

var stylelist, manage, talk, query;

function defineMenus() {
    stylelist = ' \
<a class=\"SMItem\" href=\"changestyle.php?style=defaultstyle\">默认模板</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=crystalpurplestyle\">水晶紫色</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=dotgreenstyle\">ｅ点小镇</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=emotiongraystyle\">心情灰色</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=autumnstyle\">秋意盎然</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=bluegrandstyle\">蓝色庄重</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=greenqieutlyelegantstyle\">绿色淡雅</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=bluegreenelegantstyle\">蓝雅绿</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=purpleqieutlyelegantstyle\">紫色淡雅</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=lightpurplestyle\">淡紫色</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=orangeredstyle\">橘子红了</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=rednightstyle\">红红夜思</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=pinkmemorystyle\">粉色回忆</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=greengrassstyle\">青青河草</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=thickgreenstyle\">浓浓绿意</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=brownredstyle\">棕红预览</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=littlecoffeestyle\">淡咖啡</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=seaskystyle\">碧海晴天</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=bluecrystalstyle\">蓝色水晶</a><br> \
<a class=\"SMItem\" href=\"changestyle.php?style=snowstyle\">雪花飘飘</a><br> \
';

    manage= ' \
<a class=\"MItem\" href="usermailbox.php?boxname=inbox">我的信箱</a><br> \
<a class=\"MItem\" href="favboard.php">用户收藏版面</a><br> \
<a class=\"MItem\" href="usermanagemenu.php">用户控制面板</a><br> \
<a class=\"MItem\" href="modifyuserdata.php">基本资料修改</a><br> \
<a class=\"MItem\" href="userparam.php">用户自定义参数</a><br> \
<a class=\"MItem\" href="bbssig.php">用户签名档</a><br> \
<a class=\"MItem\" href="changepasswd.php">修改密码</a><br> \
<a class=\"MItem\" href="alteryank.php">切换版面列表方式</a> \
';

    talk = ' \
<a class=\"MItem\" href="showmsgs.php">察看在线短信</a><br> \
<a class=\"MItem\" href="javascript:sendMsg()">发短信</a><br>';
if (siteconf_SMS_SUPPORT) {
	talk += '<a class=\"MItem\" href="javascript:sendSMSMsg()">发送手机短信</a><br>';
}
talk += ' \
<a class=\"MItem\" href="friendlist.php">编辑好友列表</a><br> \
<a class=\"MItem\" href="showonlinefriend.php">在线好友</a><br> \
<a class=\"MItem\" href="showonlineuser.php">在线用户</a><br> \
<a class=\"MItem\" href="dispuser.php">查询用户</a> \
';

    query = ' \
<a href="searchboard.php">搜索版面</a><br> \
<form action="searchboard.php" method="get" style="margin: 0px;"> \
<input class="TableBorder2" name="board" value="输入关键字" size="12" onmouseover="this.focus()" onfocus="this.select()" /></form> \
<hr class="TableBorder2"><a class="MItem" href="query.php" title="直接点上面的“搜索”菜单将会搜索当前版面">搜索文章</a> \
';
}

var timerID=0;
function dosendmsg(){
	oMessager=getRawObjectFrom("messager",window.frames['webmsg']);
	oMessager.submit();
}

function msg_textchange(str){
	oMsg=getRawObjectFrom("oMsgText",window.frames['webmsg']);
	oMsg.value=str;
}

function msg_typechanged(checked){
	oIsSMS=getRawObjectFrom("isSMS",window.frames['webmsg']);
	oIsSMS.checked=checked;
}

function msg_idchange(str){
	oID=getRawObjectFrom("odestid",window.frames['webmsg']);
	oID.value=str;
}
function closeWindow(){
	hide("floater");
//	document.all.floater.style.visibility='hidden';
	window.frames['webmsg'].document.location.href='getmsg.php';
}
function refreshWindow(){
	timerID=window.setTimeout("window.frames['webmsg'].document.location.href='getmsg.php';", 60000);
}
function pauseMsg(){
	window.clearTimeout(timerID);
}
function replyMsg(id){
	window.frames['webmsg'].document.location.href='sendmsg.php?destid='+id;
}
function sendMsg(){
	window.frames['webmsg'].document.location.href='sendmsg.php';
}

function sendSMSMsg(){
	window.frames['webmsg'].document.location.href='sendmsg.php?type=sms';
}


/* article flag functions. reference: libBBS/article.c get_article_flag(). - atppp */
function article_is_digest(flag) {
	upperflag = flag.toUpperCase();
	return ( 'M' == upperflag || 'B' == upperflag || 'G' == upperflag);
}

function article_is_zhiding(flag) {
	return(flag == "d" || flag == "D");
}

function article_is_noreply(flag) {
	return ( 'O' == flag || 'U' == flag || '8' == flag || ';' == flag);
}

function article_is_unread(flag) {
	if (!siteconf_SHOW_POST_UNREAD) return false;
	return ( 'D' == flag || 'G' == flag || 'M' == flag  || 'B' == flag || 'N' == flag || '*' == flag);
}


/* 服务器时钟 */
var serverDiff;
function initTime(st) {
	serverDiff = st * 1000 - new Date().getTime();
	showTime();
	setInterval("showTime()", 1000);
}
function prefixZero(d) {
	if (d > 9) return d;
	else return "0" + d;
}
function showTime() {
	if ((KCN = getRawObject("serverTime")) == null) return;
	var Timer = new Date(serverDiff + new Date().getTime());
	str = Timer.getUTCFullYear() + "-" + prefixZero(Timer.getUTCMonth() + 1) + "-" + prefixZero(Timer.getUTCDate()) + " " + prefixZero(Timer.getUTCHours()) + ":" + prefixZero(Timer.getUTCMinutes()) + ":" + prefixZero(Timer.getUTCSeconds());
	KCN.innerHTML = "服务器时间: " + str;
}


function resizeImg(obj) {
	if (obj.width>screen.width-333) obj.width=screen.width-333;
}

function refreshLeft() {
    if (top != self) {
        top.menu.doShowLeft();
        top.menu.location.href = "bbsleft.php";
    }
}