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
