//javascript code writen by windinsn@smth.org
//dec 12, 2003
var thisYear ;
var thisMonth ;
var thisDay ;

function blogCalendarLink(thisYear,thisMonth,thisDay)
{
	var thisYearStr = new String(thisYear);
	var thisMonthStr = new String(thisMonth+1);
	var thisDayStr = new String(thisDay);
	if (thisMonthStr.length < 2) thisMonthStr = "0" + thisMonthStr ;
	if (thisDayStr.length < 2) thisDayStr = "0" + thisDayStr ;
	
	timeStr = thisYearStr + thisMonthStr + thisDayStr ;
	if( blogCalendarArray[timeStr] )
		return "<font class=f4><a href='" + blogNodeUrl + "&nid=" + blogCalendarArray[timeStr] + "'>" + thisDay + "</a></font>" ;
	else
		return thisDay;
}

function blogCalendar(thisYear,thisMonth,thisDay)
{
	var monthDay = new Array();
	if( thisYear % 4 == 0 )
		monthDay = Array(31,29,31,30,31,30,31,31,30,31,30,31);
	else
		monthDay = Array(31,28,31,30,31,30,31,31,30,31,30,31);
	var monthName = new Array("Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec");
	
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
		
	tmpDay = new Date(thisYear,thisMonth,1);
	firstDay = tmpDay.getDay();
	
	tmpMonthDay = firstDay + monthDay[thisMonth] ;
	leftMonthDay = 6 - (tmpMonthDay - 1)% 7 ;
	totalMonthDay = tmpMonthDay + leftMonthDay ;
	
	var cStr ;
	cStr = "<table cellspacing=0 cellpadding=3 border=0 class=t1>";
	cStr += "<tr>";
	cStr += "<td class=t2><span class=hand title='上一年' onclick='blogCalendar(" + parseInt( thisYear - 1 ) + " , " + parseInt( thisMonth + 1 ) + " , " + thisDay + " )'>&lt;&lt;</span></td>";
	cStr += "<td class=t2><span class=hand title='上一月' onclick='blogCalendar(" + thisYear + " , " + thisMonth + " , " + thisDay + " );'>&lt;</span></td>";
	cStr += "<td class=t2 colspan=3 align=center>" + monthName[thisMonth] + " " + thisYear + "</td>";
	cStr += "<td class=t2><span class=hand title='下一月' onclick='blogCalendar(" + thisYear + " , " + parseInt( thisMonth + 2 ) + " , " + thisDay + " );'>&gt;</span></td>";
	cStr += "<td class=t2><span class=hand title='下一年' onclick='blogCalendar(" + parseInt( thisYear + 1 ) + " , " + parseInt( thisMonth + 1 ) + " , " + thisDay + " )'>&gt;&gt;</span></td>";
	cStr += "</tr>";
	cStr += "<tr><td class=t3>SUN</td><td class=t3>MON</td><td class=t3>TUE</td><td class=t3>WED</td><td class=t3>THU</td><td class=t3>FRI</td><td class=t3>SAT</td></tr>";
	
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
			linkStr = blogCalendarLink(thisYear,thisMonth,( i - firstDay + 1 ));
			if( ( i - firstDay + 1 ) == thisDay )
				linkStr = "<font class=f2>" + linkStr + "</font>";
			cStr += "<td class=" + cellClass + ">" + linkStr + "</td>";
		}
		if( i % 7 == 6 )
			cStr += "</tr>";
	}
	
	
	cStr += "</table>";
	
	bc.innerHTML = cStr;
}
