if (!top.mainframe)
{
	document.write(
'<table width="100%" border="0" cellspacing="0" cellpadding="3">' +
'  <tr>' + 
'    <td class="b2">' +
'	    <a href="' + strHomeURL + '" target="_top">' + strBBSName +
'</a>(点击可展开完整界面)' + 
'	    - ' + strDesc +
'	    - 阅读文章' +
'    </td>' +
'  </tr>'
);
}
else
{
	document.write(
'<table width="100%" border="0" cellspacing="0" cellpadding="3">' +
'  <tr>' + 
'    <td class="b2"><a href="/mainpage.php">' + strBBSName + '</a>' +
'	    - ' + strDesc +
'	    - 阅读文章' +
'    </td>' +
'  </tr>'
);
}
