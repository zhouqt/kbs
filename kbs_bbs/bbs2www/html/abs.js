/* this file dispaly active board on smthbbs
** @author: windinsn@smth.org
*/
// active board varables defination start
var delayTime = 2000;
var aboardWidth = '100%';
var aboardHeight = '71';
// active board varables defination end

// active board information start 
var defaultHeader = '<table cellpadding="5" cellspacing="0" border="0" width="'+aboardWidth+'" height="'+aboardHeight+'" style="border-color:#CCCCCC;border-width:1px;border-style:solid;font-size:12px;"><tr><td>';
var defaultBody;
var defaultFooter = '</td></tr></table>';
activeBoards = new Array(
			'欢迎光临BBS水木清华站',
			'Welcome to SMTH BBS'
			);
var totalBoards = activeBoards.length;
// active board information end

var i = -1;
function display_active_boards()
{
	(i >= totalBoards - 1)?(i=0):(i++);
	defaultBody = defaultHeader + activeBoards[i] + defaultFooter;
	//document.write(defaultBody);
	aboards.innerHTML = defaultBody;
	setTimeout('display_active_boards()',delayTime);
}


