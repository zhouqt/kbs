FDRboxWid = 700;
FDRboxHgt = 25;
FDRborWid = 0;
FDRborCol = "#070707";
FDRborSty = "solid";
FDRbackCol = "";
FDRboxPad = 4;

FDRtxtAln = "center";
FDRlinHgt = "6pt";
FDRfntFam = "Verdana,Arial,Helvetica,sans-serif,ËÎÌו";
FDRfntCol = "";
FDRfntSiz = "9pt";
FDRfntWgh = "";
FDRfntSty = "normal";

FDRgifSrc = "images/fade.gif";
FDRgifInt = 60;

FDRblendInt = 6;
FDRblendDur = 1;
FDRmaxLoops = 0;

FDRendWithFirst = true;
FDRreplayOnClick = true;

FDRjustFlip = false;
FDRhdlineCount = 0;

loopCount=0;

FDRjustFlip = (window.FDRjustFlip) ? FDRjustFlip : false;
FDRhdlineCount = (window.FDRhdlineCount) ? FDRhdlineCount : 1;

FDRfinite = (FDRmaxLoops > 0);
FDRisOver = false;
FDRloadCount = 0;

blendTimer = null;

if (!window.prefix) prefix = "";
window.onload = FDRcountLoads;

function FDRcountLoads(e) {
	setTimeout("FDRinit()",1);
}

function FDRinit(){
	var ooElFader;
	if(!window.arNews) {
		if(!window.arTXT || !window.arURL) return;
		if(arTXT.length != arURL.length) return;
		arNews = [];
		for (i=0;i<arTXT.length;i++){
			arNews[arNews.length] = arTXT[i];
			arNews[arNews.length] = arURL[i];
		}
	}
	if (!(oElFader=getRawObject("elFader"))) return;
	oElFader.innerHTML ="";
	if (!document.styleSheets.length) document.createStyleSheet();

	with (oElFader.style) {
		errorOffset =  0;
		width = (FDRboxWid - (errorOffset * 2)) +'px';
		height = (FDRboxHgt - (errorOffset * 2))+'px';

		backgroundColor = FDRbackCol;
		overflow = "hidden";
		color = FDRfntCol;
		fontWeight = FDRfntWgh;
		fontSize = FDRfntSiz;
		fontStyle = FDRfntSty;
		fontFamily = FDRfntFam;
		lineHeight = FDRlinHgt;
		textAlign = FDRtxtAln;
		cursor = "default";
		visibility = "visible";
		borderWidth = FDRborWid;
		borderStyle = FDRborSty;
		borderColor = FDRborCol;
		padding  = FDRboxPad+'px';

		if(!FDRjustFlip) filter = "blendTrans(duration=" + FDRblendDur + ")";
	}
	oElFader.onselectstart = function(){return false};

	if (isIE4) {
		IEhasFilters = (oElFader.filters.blendTrans) ? true : false;
	}

	oElFader.onmouseover = function (){
		FDRisOver = true;
	}
	oElFader.onmouseout = function(){
		FDRisOver = false;
		status = "";
	}
	
	FDRstart(0);
}

function FDRstart(ind){
    newsCount = ind;
    if (FDRfinite) loopCount = 0;
    FDRdo();
    blendTimer = setInterval("FDRdo()",FDRblendInt*1000)
}

function FDRdo() {

	if(!blendTimer && loopCount>0) return;

    if (FDRfinite && loopCount==FDRmaxLoops) {
        FDRend();
		return;
    }
	FDRfade();

    if (newsCount == arNews.length) {
        newsCount = 0;
        if (FDRfinite) loopCount++;
    }
}

function FDRmakeStr(){
	tempStr = "";
	for (i=0;i<FDRhdlineCount;i++){
		if(newsCount>=arNews.length)break;
		dispStr = arNews[newsCount];
		linkStr = arNews[newsCount+1];
		isLink = linkStr.length;
		if (isLink) {
			tempStr += "<P><A CLASS=newslink "
					+ "HREF='" + prefix + linkStr + "'>"
		            + dispStr + "</A></P>"
		}
		else {
			tempStr +=  "<P>" +dispStr+"</P>";

		}
		newsCount += 2;
	}
	return tempStr;
}

function FDRfade(){

	newsStr = FDRmakeStr();
	
	if(isIE4 && IEhasFilters) oElFader.filters.blendTrans.Apply();
	oElFader.innerHTML = newsStr;
	if(isIE4 && IEhasFilters)oElFader.filters.blendTrans.Play();

	if(FDRhdlineCount==1) window.status = (FDRisOver && isLink) ? (prefix + linkStr) : "";
}

function FDRslide(){
    elGif.top += slideInc;
    if (elGif.top >= 0) {elGif.visibility = "hide";return}
    setTimeout("FDRslide()",FDRgifInt);
}

function FDRdblClickNS(){
	oElFader.releaseEvents(Event.DBLCLICK);
	FDRstart(startIndex);
	return false;
}

function FDRend(){
	clearInterval(blendTimer);
	blendTimer = null;
	if (FDRendWithFirst) {
		newsCount = 0;
		FDRfade();
	}
	if (FDRreplayOnClick) {
		startIndex = FDRendWithFirst ? (FDRhdlineCount * 2) : 0;
		oElFader.title = "Double click to replay";
		oElFader.ondblclick = function(){
			this.ondblclick = null;
			this.title = "";
			FDRstart(startIndex);
		}
    }
}


