function checkall(form)
{
	for (var i=0;i<form.elements.length;i++)
	{
		var e = form.elements[i];
		if (e.name != 'chkall')
			e.checked = form.chkall.checked; 
	}
}

function bbsconfirm(url,infor)
{
	if(confirm(infor))
	{
		window.location.href=url;
		return true;
	}
	return false;
}

function blogCalendar(thisYear,thisMonth,thisDay)
{
	var monthDay = new Array();
	if( thisYear % 4 == 0 )
		monthDay = Array(31,29,31,30,31,30,31,31,30,31,30,31);
	else
		monthDay = Array(31,28,31,30,31,30,31,31,30,31,30,31);
	var monthName = new Array("January","February","March","April","May","June","July","August","September","October","November","December");
	
	thisMonth -- ;
	document.write('<table cellspacing=1 cellpadding=2 border=0 width=100% class=t1>');
	document.write('</table>');
	
	
}
