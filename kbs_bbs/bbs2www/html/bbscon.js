if (!top.mainframe)
{
	document.write(
'<table width="100%" border="0" cellspacing="0" cellpadding="3">' +
'  <tr>' + 
'    <td colspan="2" class="b1">' +
'	    <a href="' + strHomeURL + '" target="_top">' + strBBSName +
'</a>(点击可展开完整界面)' + 
'	    - ' + strDesc +
'	    - 阅读文章' +
'    </td>' +
'  </tr>' +
'  <tr> ' +
'</table>' +
'<br/>'
);
}
else
{
	document.write(
'<table width="100%" border="0" cellspacing="0" cellpadding="3">' +
'  <tr>' + 
'    <td colspan="2" class="b1">' + strBBSName +
'	    - ' + strDesc +
'	    - 阅读文章' +
'    </td>' +
'  </tr>' +
'  <tr> ' +
'</table>' +
'<br/>');
}
