<?php
//入口参数:strContent内容，PostUserGroup用户组ID，PostType使用类型（1表示帖子，2表示公告，3表示短信）

function DvBCode($strContent,$filterHTML, $abgcolor='')
{
	extract($GLOBALS);

	if ($filterHTML) {
		$strContent=dvHTMLEncode($strContent);
	}
//UBB Code

	$strContent=FilterJS($strContent);

//IMG Code
/*
	$search = array("'\[IMG\]((?:http|https|ftp)://[^\[]*)\[\/IMG\]'i",
						"'\[UPLOAD=(gif|jpg|jpeg|bmp|png)\]([^\[]*\.)\\1\[/UPLOAD\]'i");
*/
	$search = array("'\[IMG\]((?:http|https|ftp)://[^\[]*)\[\/IMG\]'i");

	$replace= array("<a onfocus=this.blur() href=\"\\1\" target=_blank><IMG SRC=\"\\1\" border=0 alt=按此在新窗口浏览图片 onload=\"javascript:if(this.width>screen.width-333)this.width=screen.width-333\"></a>",
						"<br><IMG SRC=\"images/files/\\1.gif\" border=0>此主题相关图片如下：<br><A HREF=\"\\2\\1\" TARGET=_blank><IMG SRC=\"\\2\\1\" border=0 alt=按此在新窗口浏览图片 onload=\"javascript:if(this.width>screen.width-333)this.width=screen.width-333\"></A>");

	$strContent=preg_replace($search,$replace,$strContent);
/*
	$strContent=preg_replace("'\[UPLOAD=([^\[]*)\]([^\[]*)\[\/UPLOAD\]'i",
							"<br><IMG SRC=\"images/files/\\1.gif\" border=0> <a href=\"\\2\">点击浏览该文件</a>",
							$strContent);
*/
//多媒体标签
	$search = array("'\[DIR=(\d*),(\d*)\]([^\[]*)\[/DIR\]'i",
						"'\[QT=(\d*),(\d*)\]([^\[]*)\[/QT\]'i",
						"'\[MP=(\d*),(\d*)\]([^\[]*)\[/MP\]'i",
						"'\[RM=(\d*),(\d*)\]([^\[]*)\[/RM\]'i",
						"'\[FLASH(?:=(\d*),(\d*))?\]([^\[]*)\[\/FLASH\]'ie",
						"'\[SOUND\]([^\[]*)\[/SOUND\]'i");

		$replace= array("<object classid=clsid:166B1BCA-3F9C-11CF-8075-444553540000 codebase=http://download.macromedia.com/pub/shockwave/cabs/director/sw.cab#version=7,0,2,0 width=\\1 height=\\2><param name=src value=\\3><embed src=\\3 pluginspage=http://www.macromedia.com/shockwave/download/ width=\\1 height=\\2></embed></object>",
						"<embed src=\\3 width=\\1 height=\\2 autoplay=true loop=false controller=true playeveryframe=false cache=false scale=TOFIT bgcolor=#000000 kioskmode=false targetcache=false pluginspage=http://www.apple.com/quicktime/>",
						"<object align=middle classid=CLSID:22d6f312-b0f6-11d0-94ab-0080c74c7e95 class=OBJECT id=MediaPlayer width=\\1 height=\\2 ><param name=ShowStatusBar value=-1><param name=Filename value=\\3><embed type=application/x-oleobject, codebase=http://activex.microsoft.com/activex/controls/mplayer/en/nsmp2inf.cab#Version=5,1,52,701 flename=mp src=\\3 width=\\1 height=\\2></embed></object>",
						"<OBJECT classid=clsid:CFCDAA03-8BE4-11cf-B84B-0020AFBBCCFA class=OBJECT id=RAOCX width=\\1 height=\\2><PARAM NAME=SRC VALUE=\\3><PARAM NAME=CONSOLE VALUE=Clip1><PARAM NAME=CONTROLS VALUE=imagewindow><PARAM NAME=AUTOSTART VALUE=true></OBJECT><br><OBJECT classid=CLSID:CFCDAA03-8BE4-11CF-B84B-0020AFBBCCFA height=32 id=video2 width=\\1><PARAM NAME=SRC VALUE=\\3><PARAM NAME=AUTOSTART VALUE=-1><PARAM NAME=CONTROLS VALUE=controlpanel><PARAM NAME=CONSOLE VALUE=Clip1></OBJECT>",
						"'<a href=\"\\3\" TARGET=_blank><IMG SRC=pic/swf.gif border=0 alt=点击开新窗口欣赏该FLASH动画! height=16 width=16>[全屏欣赏]</a><br><OBJECT codeBase=http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=4,0,2,0 classid=clsid:D27CDB6E-AE6D-11cf-96B8-444553540000 width='.(strlen('\\1')?'\\1':500).' height='.(strlen('\\2')?'\\2':400).'><PARAM NAME=movie VALUE=\"\\3\"><PARAM NAME=quality VALUE=high><embed src=\"\\3\" quality=high pluginspage=\"http://www.macromedia.com/shockwave/download/index.cgi?P1_Prod_Version=ShockwaveFlash\" type=\"application/x-shockwave-flash\" width='.(strlen('\\1')?'\\1':500).' height='.(strlen('\\2')?'\\2':400).'>\\3</embed></OBJECT>'",
						"<a href=\"\\1\" target=_blank><IMG SRC=pic/mid.gif border=0 alt='背景音乐'></a><bgsound src=\"\\1\" loop=\"-1\">");

	$strContent=preg_replace($search,$replace,$strContent);
    
/*
//积分可见
    if((intval($Board_Setting[11])==1 || $PostUserGroup<4) && $PostType=1)
    {
		$search="'(^.*)\[point=(\d*)\]([^\[]*)\[/point\](.*)'i";
		$po=preg_replace($search,"\\2",$strContent);
		if(is_numeric($po))
			$ii=intval($po);
		else
			$ii=0;

		if($userid==$PostUserID || $myuserep>$ii)
			$replace="\\1<hr noshade size=1>以下为需要积分达到<B>\\2</B>才能浏览的内容<BR>\\3<hr noshade size=1>\\4";
		else
			$replace="\\1<hr noshade size=1><font color=".$Forum_body[8].">此内容要求积分达到\\2以上才可以浏览</font><hr noshade size=1>\\4";

		$strContent=preg_replace($search,$replace,$strContent);
    }
    else
    {
        $search="'\[point=\d*\]([^\[])*\[/point\]'i";
        $strContent=preg_replace($search,"\\1",$strContent);
    }
*/
/*
//魅力可见
    if((intval($Board_Setting[12])==1 || $PostUserGroup<4) && $PostType=1)
    {
		$search="'(^.*)\[UserCP=(\d*)\]([^\[]*)\[/UserCP\](.*)'i";
        $po=preg_replace($search,"\\2",$strContent);
        if(is_numeric($po))
            $ii=intval($po);
        else
            $ii=0;

        if($userid==$PostUserID || $myuserep>$ii)
			$replace="\\1<hr noshade size=1>以下为需要魅力达到<B>\\2</B>才能浏览的内容<BR>\\3<hr noshade size=1>\\4";
		else
			$replace="\\1<hr noshade size=1><font color=".$Forum_body[8].">此内容要求魅力达到\\2以上才可以浏览</font><hr noshade size=1>\\4";

		$strContent=preg_replace($search,$replace,$strContent);
    }
    else
    {
        $search="'\[UserCP=\d*\]([^\[]*)\[/UserCP\]'i";
        $strContent=preg_replace($search,"\\1",$strContent);
    }
*/
/*
//威望可见
    if((intval($Board_Setting[13])==1 || $PostUserGroup<4) && $PostType=1)
    {
		$search="'(^.*)\[Power=(\d*)\]([^\[]*)\[/Power\](.*)'i";
        $po=preg_replace($search,"\\2",$strContent);
        if(is_numeric($po))
            $ii=intval($po);
        else
            $ii=0;

        if($userid==$PostUserID || $mypower>$ii)
			$replace="\\1<hr noshade size=1>以下为需要威望达到<B>\\2</B>才能浏览的内容<BR>\\3<hr noshade size=1>\\4";
		else
			$replace="\\1<hr noshade size=1><font color=".$Forum_body[8].">此内容要求威望达到\\2以上才可以浏览</font><hr noshade size=1>\\4";
    	$strContent=preg_replace($search,$replace,$strContent);
	}
    else
    {
        $search="'\[Power=\d*\]([^\[]*)\[/Power\]'i";
        $strContent=preg_replace($search,"\\1",$strContent);
    }
*/
/*
//文章可见
    if((intval($Board_Setting[14])==1 || $PostUserGroup<4) && $PostType=1)
    {
        $search="'(^.*)\[Post=(\d*)\]([^\[]*)\[/Post\](.*)'i";
        $po=preg_replace($search,"\\2",$strContent);
        if(is_numeric($po))
            $ii=intval($po);
        else
            $ii=0;
    
        if($userid==$PostUserID || $myArticle>$ii)
            $replace="\\1<hr noshade size=1>以下为需要文章数达到<B>\\2</B>才能浏览的内容<BR>\\3<hr noshade size=1>\\4";
        else
            $replace="\\1<hr noshade size=1><font color=".$Forum_body[8].">此内容要求文章数达到\\2以上才可以浏览</font><hr noshade siz
e=1>\\4";
        $strContent=preg_replace($search,$replace,$strContent);
    }
    else
    {
        $search="'\[Post=\d*\]([^\[]*)\[/Post\]'i";
        $strContent=preg_replace($search,"\\1",$strContent);
    }
*/
/*
//金钱可见
    if((intval($Board_Setting[10])==1 || $PostUserGroup<4) && $PostType=1)
    {
        $search="'(^.*)\[Money=(\d*)\]([^\[]*)\[/Money\](.*)'i";
        $po=preg_replace($search,"\\2",$strContent);
        if(is_numeric($po))
            $ii=intval($po);
        else
            $ii=0;
    
        if($userid==$PostUserID || $mymoney>$ii)
            $replace="\\1<hr noshade size=1>以下为需要金钱数达到<B>\\2</B>才能浏览的内容<BR>\\3<hr noshade size=1>\\4";
        else
            $replace="\\1<hr noshade size=1><font color=".$Forum_body[8].">此内容要求金钱数达到\\2以上才可以浏览</font><hr noshade s
iz
e=1>\\4";
        $strContent=preg_replace($search,$replace,$strContent);
    }
    else
    {
        $search="'\[Money=\d*\]([^\[]*)\[/Money\]'i";
        $strContent=preg_replace($search,"\\1",$strContent);
    }
*/
/*
//回复可见
    if((intval($Board_Setting[15])==1 || $PostUserGroup<4) && $PostType=1)
    {
		$search="'\[replyview\]([^\[]*)\[/replyview\]'i";
		if(preg_match($search,$strContent)!=false)
		{
			$vrs=$conn->getRow("select AnnounceID from ".$TotalUseTable." where rootid=".$Announceid." and PostUserID=".$userid);
			if($vrs==null)
				$replace="<hr noshade size=1><font color=".$Forum_body[8].">此内容需要回复才能浏览</font><hr noshade size=1>";
			else
				$replace="<hr noshade size=1>以下内容为需要回复才能浏览<BR>\\1<hr noshade size=1>";

			$strContent=preg_replace($search,"\\1",$strContent);
		}
    }
    else
    {
		$search="'\[replyview\]([^\[]*)\[/replyview\]'i";
		$strContent=preg_replace($search,"\\1",$strContent);
    }
*/
/*
//购买帖子
    if((intval($Board_Setting[23])==1 || $PostUserGroup<4) && $PostType=1)
    {
		$search="'(^.*)\[usemoney=(\d*)\]([^\[]*)\[/usemoney\](.*)'i";
        $po=preg_replace($search,"\\2",$strContent);
        if(is_numeric($po))
            $ii=intval($po);
        else
            $ii=0;
		if($userid==$PostUserID)
		{
			if(isset($PostBuyUser) && $PostBuyUser!="")
			{
				$SplitBuyUser=explode("|",$PostBuyUser);
				$iPostBuyUser="<option value=0>已购买用户</option>";

				foreach($SplitBuyUser as $i=>$User)
					$iPostBuyUser.="<option value=".$i.">".$User."</option>";
			}else{
				$iPostBuyUser="<option value=0>还没有用户购买</option>";
			}
			$replace="\\1<hr noshade size=1>以下为需要金钱数达到<B>\\2</B>才能浏览的内容&nbsp;&nbsp;<select size=1 name=buyuser>".$iPostBuyUser."</select><BR>\\3<hr noshade size=1>\\4";

			$strContent=preg_replace($search,$replace,$strContent);
		}elseif($mymoney>$ii){
			if($userid==$PostUserID || $master || $boardmaster || $superboardmaster)
			{
	            if(isset($PostBuyUser) && $PostBuyUser!="")
    	        {   
        	        $SplitBuyUser=explode("|",$PostBuyUser);
            	    $iPostBuyUser="<option value=0>已购买用户</option>";
                
                	foreach($SplitBuyUser as $i=>$User)
                    	$iPostBuyUser.="<option value=".($i+1).">".$User."</option>";
	            }else{
    	            $iPostBuyUser="<option value=0>还没有用户购买</option>";
        	    }
			}

//			if(isset($PostBuyUser) && $PostBuyUser!="")
//			{
			if(stristr("|".$PostBuyUser."|","|".$membername."|"))
				$replace="\\1<hr noshade size=1>以下为需要花<B>\\2</B>金钱才能购买并浏览的内容，您已经购买本贴<BR>\\3<hr noshade size=1>\\4";
			else
				$replace="\\1<Form action=\"buypost.php\" method=post><font color=".$Forum_body[8].">此帖子内容需要您花\\2金钱来购买浏览&nbsp;&nbsp;<input type=hidden name=boardid value=".$Boardid."><input type=hidden value=".$replyID." name=replyID><input type=hidden value=".$AnnounceID." name=id><input type=hidden value=".$TotalUseTable." name=PostTable><input type=submit name=submit value=好黑啊…我…我买了！>&nbsp;&nbsp;".$iPostBuyUser."</font></form>\\4";
//			}
    	}else{
			$replace="\\1<Form action=\"buypost.php\" method=post><font color=".$Forum_body[8].">此帖子内容需要您花\\2金钱来购买浏览&nbsp;&nbsp;<input type=hidden name=boardid value=".$Boardid."><input type=hidden value=".$replyID." name=replyID><input type=hidden value=".$AnnounceID." name=id><input type=hidden value=".$TotalUseTable." name=PostTable><input type=submit name=submit value=好黑啊…我…我买了！>&nbsp;&nbsp;".$iPostBuyUser."</font></form>\\4";
		}
		$strContent=preg_replace($search,$replace,$strContent);
	}
	else
	{
        $search="'\[usemoney=\d*\]([^\[]*)\[/usemoney\]'i";
        $strContent=preg_replace($search,"\\1",$strContent);
    }
*/
//em code

	$search="'\[em(\d+)\]'ie";

	$replace="'<img src=\"emot/em\\1.gif\" border=0 align=middle>'";

	$strContent=preg_replace($search,$replace,$strContent);

    $search=array("'\[URL(?:=([^\[]*))?\]([^\[]*)\[/URL\]'ie",
                "'\[EMAIL(?:=(\S+@[^\[]*))?\]((?(1)[^\[]*|\S+@[^\[]*))\[/EMAIL\]'ie",
				"'(?<!>|=|\")(?:http|https|ftp|rtsp|mms):(?://|\\\\)[A-Za-z0-9\./=\?%\-&_~`@\[\]\':+!]+'i",//自动识别网址
                //WWW开头网址判断
                //Email地址判断
				"'\[(?:html|code)\](.*)\[/\\1\]'iUs",
				"'\[QUOTE\](.*)\[/QUOTE\]'iU",
				"'\[fly\](.*)\[/fly\]'iU",
				"'\[move\](.*)\[/move\]'iU",
                "'\[(color|face)=([^\[]+)\](.*)\[/\\1\]'iU",
                "'\[align=(center|left|right)\](.*)\[/align\]'iU",
                "'\[(shadow|glow)=(\d*),(#?[a-z0-9]*),(\d*)\](.*)\[/\\1]'iU",
//              "'\[([iub])\]([^\[]*|(?R))\[/\\1\]'i",
				"'\[i\](.*)\[/i\]'iU",
				"'\[u\](.*)\[/u\]'iU",
				"'\[b\](.*)\[/b\]'iU",
                "'\[size=([1-4])\](.*)\[/size\]'iU",
                "'<I></I>'i"
                );

    $replace=array("'<A HREF=\"'.(strlen('\\1')?'\\1':'\\2').'\" TARGET=_blank>\\2</A>'",
                "'<img align=absmiddle src=pic/email1.gif><A HREF=\"mailto:'.(strlen('\\1')?'\\1':'\\2').'\">\\2</A>'",
				"<img align=absmiddle src=pic/url.gif border=0><a target=_blank href=\\0>\\0</a>",
                //WWW替换结果
                //Email替换结果
				"<table width='100%' border='0' cellspacing='0' cellpadding='6' class='".$abgcolor."'><td><b>以下内容为程序代码:</b><br>\\1</td></table>'i",
				"<table style=\"width:100%\" cellpadding=5 cellspacing=1 class=TableBorder1><TR><TD class=".$abgcolor." width=\"100%\">\\1</td></tr></table><br>",
				"<marquee width=90% behavior=alternate scrollamount=3>\\1</marquee>",
				"<MARQUEE scrollamount=3>\\1</marquee>",
                "<font \\1=\\2>\\3</font>",
                "<div align=\\1>\\2</div>",
                "<table width=\\2 ><tr><td style=\"filter:\\1(color=\\3, strength=\\4)\">\\5</td></tr></table>",
//              "<\\1>\\2</\\1>",
				"<i>\\1</i>",
				"<u>\\1</u>",
				"<b>\\1</b>",
                "<font size=\\1>\\2</font>",
                ""
                );

    $strContent=preg_replace($search,$replace,$strContent);


	return $strContent;
} 

function DvSignCode($strContent,$PostUserGroup)
{
	extract($GLOBALS);
	
    if(intval($Forum_Setting[66])!=1)
		$strContent=dvHTMLEncode($strContent);
    else
		$strContent=HTMLcode($strContent);

    if(intval($Forum_Setting[65])!=1 && $PostUserGroup>2)
		return $strContent;

    $strContent=FilterJS($strContent);

//显示图片和Flash
    $search=array("'\[IMG\]((?:http|https|ftp)://[^\[]*)\[/IMG\]'i",
		"'\[FLASH(?:=(\d+),(\d+))?\](http://[^\[]+.swf)\[\/FLASH\]'ie");

    $replace=array("<IMG SRC=\"\\1\" border=0 onload=\"javascript:if(this.width>screen.width-500)this.width=screen.width-500\">",
		   "'<a href=\"\\3\" TARGET=_blank><IMG SRC=pic/swf.gif border=0 alt=点击开新窗口欣赏该FLASH动画! height=16 width=16>[全屏欣赏]</a><br><OBJECT codeBase=http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=4,0,2,0 classid=clsid:D27CDB6E-AE6D-11cf-96B8-444553540000 width='.(strlen('\\1')?'\\1':500).' height='.(strlen('\\2')?'\\2':400).'><PARAM NAME=movie VALUE=\"\\3\"><PARAM NAME=quality VALUE=high><embed src=\"\\3\" quality=high pluginspage=\"http://www.macromedia.com/shockwave/download/index.cgi?P1_Prod_Version=ShockwaveFlash\" type=\"application/x-shockwave-flash\" width='.(strlen('\\1')?'\\1':500).' height='.(strlen('\\2')?'\\2':400).'>\\3</embed></OBJECT>'");

    $alter=array("'<IMG SRC=\"'.$Forum_info[7].'gif.gif\" border=0><a onfocus=this.blur() href=\"\\1\" target=_blank>\\1</a>'",
		"'<IMG SRC=\"'.$Forum_info[7].'swf.gif\" border=0><a href=\\3 target=_blank>\\3</a>'");

	if(intval($Forum_Setting[67])==1 || $PostUserGroup<4)
		$strContent=preg_replace($search,$replace,$strContent);
	else
		$strContent=preg_replace($search,$alter,$strContent);

    $search=array("'\[URL(?:=([^\[]*))?\]([^\[]*)\[/URL\]'ie",
				"'\[EMAIL(?:=(\S+@[^\[]*))?\]((?(1)[^\[]*|\S+@[^\[]*))\[/EMAIL\]'ie",
				"'(?<!>|=|\")(?:http|https|ftp|rtsp|mms):(?://|\\\\)[A-Za-z0-9\./=\?%\-&_~`@\[\]\':+!]+'i",
				//WWW开头网址判断
				//Email地址判断
				"'\[em(\d+)\]'i",
				"'\[(color|face)=([^\[]+)\](.*)\[/\\1\]'iU",
				"'\[align=(center|left|right)\](.*)\[/align\]'iU",
				"'\[(shadow|glow)=(\d*),(#*[a-z0-9]*),(\d*)\](.*)\[/\1]'iU",
//				"'\[([iub])\]([^\[]*|(?R))\[/\1\]'i",
                "'\[i\](.*)\[/i\]'iU",
                "'\[u\](.*)\[/u\]'iU",
                "'\[b\](.*)\[/b\]'iU",
				"'<I></I>'i"
				);

    $replace=array("<A HREF=\"'.(strlen('\\1')?'\\1':'\\2').'\" TARGET=_blank>\\2</A>",
				"<img align=absmiddle src=pic/email1.gif><A HREF=\"mailto:'.(strlen('\\1')?'\\1':'\\2').'\">\\2</A>",
				"<img align=absmiddle src=pic/url.gif border=0><a target=_blank href=\\0>\\0</a>",
				//WWW替换结果
				//Email替换结果
				"<img src=".$Forum_info[7]."em=\\1.gif border=0 align=middle>",
				"<font \\1=\\2>\\3</font>",
				"<div align=\\1>\\2</div>",
				"<table width=\\2 ><tr><td style=\"filter:\\1(color=\\3, strength=\\4)\">\\5</td></tr></table>",
//				"<\\1>\\2</\\1>",
                "<i>\\1</i>",
                "<u>\\1</u>",
                "<b>\\1</b>",
				"<font size=\\1>\\2</font>",
				""
				);

    $strContent=preg_replace($search,$replace,$strContent);

    return $strContent;
}

function reUBBCode($strContent)
{
	//$strContent=dvHTMLEncode($strContent);

    $search=array("'&nbsp;'",
                "'\[quote\].*\[/quote\]'iU",
                "'\[point=\d*\].*\[/point\]'iU",
                "'\[post=\d*\].*\[/post\]'iU",
                "'\[power=\d*\].*\[/power\]'iU",
                "'\[usercp=\d*\].*\[/usercp\]'iU",
                "'\[money=\d*\].*\[/money\]'iU",
                "'\[usemoney=\d*\].*\[/usemoney\]'iU",
                "'\[replyview\].*\[/replyview\]'iU",
				"'\[UPLOAD=([^\[]*)\]([^\[]*)\[\/UPLOAD\]'iU",
                "'<I></I>'i");

    $replace=array(" ",
                "",
                "&nbsp",
                "&nbsp",
                "&nbsp",
                "&nbsp",
                "&nbsp",
                "&nbsp",
                "&nbsp",
				"",
                "");

    $strContent=preg_replace($search,$replace,$strContent);

    return $strContent;
}

function FilterJS($v)
{
    if(isset($v))
    {
        $pattern =array("/javascript/i",
                        "/javascript:/i",
                        "/js:/i",
                        "/about:/i",
                        "/file:/i",
                        "/document.cookie/i",
                        "/vbscript:/i",
                        "/vbs:/i",
                        "/on(mouse|exit|error|click|key)/i");

        $replace =array("&#106avascript",
                        "&#106script:",
                        "&#106s:",
                        "about&#58",
                        "file&#58",
                        "documents&#46cookie",
                        "&#118bscript:",
                        "&#118bs:",
                        "&#111n\\1");

        $v=preg_replace($pattern,$replace,$v);
    }
    return $v;
}

function dvHTMLEncode($v)
{
    if(isset($v))
    {
        $pattern =array(">",
                        "<",
                        chr(32),
                        chr(9),
                        chr(34),
                        chr(39),
                        chr(13),
                        "\n\n",
                        "\n");

        $replace =array("&gt;",
                        "&lt;",
                        "<I></I>&nbsp;",
                        "&nbsp;",
                        "&quot;",
                        "&#39;",
                        "",
                        "</P><P>",
                        "<BR>");

        $v=str_replace($pattern,$replace,$v);
    }
    return $v;
}
?>
