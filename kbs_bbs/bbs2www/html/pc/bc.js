//javascript code writen by windinsn@smth.org
//dec 12, 2003
var thisYear ;
var thisMonth ;
var thisDay ;

function blogCalendarLink(thisYear,thisMonth,thisDay,isToday)
{
	var styleClass = isToday?"bc3":"bc2";
	var noLinkStyleClass = isToday?"bc3":"f1";
	var thisYearStr = new String(thisYear);
	var thisMonthStr = new String(thisMonth+1);
	var thisDayStr = new String(thisDay);
	if (thisMonthStr.length < 2) thisMonthStr = "0" + thisMonthStr ;
	if (thisDayStr.length < 2) thisDayStr = "0" + thisDayStr ;
	
	
	timeStr = thisYearStr + thisMonthStr + thisDayStr ;
	deliChar = (blogNodeUrl.indexOf('?') == -1) ? "?" : "&n";
	if( blogCalendarArray[timeStr] )
		return "<font style=\"background-color:#999999\"><a href='" + blogNodeUrl + deliChar + "id=" + blogCalendarArray[timeStr] + "' class = '" + styleClass + "'>" + thisDay + "</a></font>" ;
	else
		return "<font class = '" + noLinkStyleClass + "' >" + thisDay + "</font>";
}

var theYear; 
var theMonth; 
var theDay=-1; 

function blogCalendar(thisYear,thisMonth,thisDay)
{
	var monthDay = new Array();
	if( thisYear % 4 == 0 )
		monthDay = Array(31,29,31,30,31,30,31,31,30,31,30,31);
	else
		monthDay = Array(31,28,31,30,31,30,31,31,30,31,30,31);
	var monthName = new Array("一月","二月","三月","四月","五月","六月","七月","八月","九月","十月","十一月","十二月");
	
	if( thisMonth < 1 )
	{
		thisMonth = 12 ;
		thisYear -- ;
	}
	if( thisMonth > 12 )
	{
		thisMonth = 1 ;
		thisYear ++ ;
	}
	thisMonth -- ;
	
	if( thisDay > monthDay[thisMonth] )
		thisDay = monthDay[thisMonth] ;
	if( thisDay < 1 )
		thisDay = 1;

       if (theDay==-1) { 
               theYear=thisYear; 
               theMonth=thisMonth; 
               theDay=thisDay; 
       } 
		
	tmpDay = new Date( thisYear ,thisMonth,1);
	firstDay = tmpDay.getDay();
	
	tmpMonthDay = firstDay + monthDay[thisMonth] ;
	leftMonthDay = 6 - (tmpMonthDay - 1)% 7 ;
	totalMonthDay = tmpMonthDay + leftMonthDay ;
	
	var cStr ;
	cStr = "<table cellspacing=0 cellpadding=3 border=0 class=t1>";
	cStr += "<tr>";
	cStr += "<td class=t2><span class=hand title='上一年' onclick='blogCalendar(" + parseInt( thisYear - 1 ) + " , " + parseInt( thisMonth + 1 ) + " , " + thisDay + " )'>&lt;&lt;</span></td>";
	cStr += "<td class=t2><span class=hand title='上一月' onclick='blogCalendar(" + thisYear + " , " + thisMonth + " , " + thisDay + " );'>&lt;</span></td>";
	cStr += "<td class=t2 colspan=3 align=center>" +  thisYear + "年" + monthName[thisMonth] + "</td>";
	cStr += "<td class=t2><span class=hand title='下一月' onclick='blogCalendar(" + thisYear + " , " + parseInt( thisMonth + 2 ) + " , " + thisDay + " );'>&gt;</span></td>";
	cStr += "<td class=t2><span class=hand title='下一年' onclick='blogCalendar(" + parseInt( thisYear + 1 ) + " , " + parseInt( thisMonth + 1 ) + " , " + thisDay + " )'>&gt;&gt;</span></td>";
	cStr += "</tr>";
	cStr += "<tr><td class=t3>日</td><td class=t3>一</td><td class=t3>二</td><td class=t3>三</td><td class=t3>四</td><td class=t3>五</td><td class=t3>六</td></tr>";
	
	j = 0 ;
	for( i = 0 ; i < totalMonthDay ; i ++ )
	{
		if( i % 7 == 0 )
		{
			cStr += "<tr>";
			j ++ ;
		}
		cellClass = ( j % 2 == 1 )?"t4":"t3";
				
		if( i < firstDay || i > monthDay[thisMonth]+firstDay-1 )
			cStr += "<td class=" + cellClass + ">&nbsp;</td>";	
		else
		{
			if( ( i - firstDay + 1 ) == theDay && thisYear == theYear && thisMonth == theMonth )
				linkStr = blogCalendarLink(thisYear,thisMonth,( i - firstDay + 1 ),true);
			else
				linkStr = blogCalendarLink(thisYear,thisMonth,( i - firstDay + 1 ),false);
			cStr += "<td class=" + cellClass + ">" + linkStr + "</td>";
		}
		if( i % 7 == 6 )
			cStr += "</tr>";
	}
	
	
	cStr += "</table>";
	
	document.getElementById("bc").innerHTML = cStr;
}
