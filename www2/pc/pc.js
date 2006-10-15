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

function openScript(url, width, height)
{
	var Win = window.open(url,"openScript",'width=' + width + ',height=' + height + ',resizable=1,scrollbars=yes,menubar=no,status=yes' );
}

function submitwithcopy() {
	f = document.postform;
	if (f.subject.value==''){
		alert('请输入标题!');
		f.subject.focus();
		return false;
	}
/*
	if ((h = document.getElementById("holdtext")) != null) {
		h.innerText = document.postform.blogbody.value; 
		Copied = h.createTextRange(); 
		Copied.execCommand('Copy');
		alert('文章内容已复制到剪贴板，如果发表失败，请重新发表'); 
	}
*/
	if (f.postbutton != null) {
		f.postbutton.value="发表中，请稍候……";
		f.postbutton.disabled=true;
	}
	window.onbeforeunload=null;
	/*f.submit();*/
	return true;
}

function confirmLeave() {
	window.onbeforeunload = function() {
		return "正在编辑，确定离开吗？编辑的内容将全部丢失！";
	}
}

function doCancel() {
	window.onbeforeunload=null;
	history.go(-1);
}

function initEditor() {
	if (document.getElementById("blogbody") == null) return false;
	/*
	HTMLArea.loadPlugin("TableOperations");
	HTMLArea.loadPlugin("SpellChecker");
	*/
	editor = new HTMLArea("blogbody", getBlogConfig());
	editor.generate();
	document.onkeypress = confirmLeave;
	setTimeout( function() {
		var doc = editor._iframe.contentWindow.document;
		if (doc) {
			HTMLArea._addEvent(doc, "keypress", confirmLeave);
		}
	}, 1000);
	return false;
}

/* html area additional functions */
function insertHTML() {
	var html = prompt("Enter some HTML code here");
	if (html) {
		editor.insertHTML(html);
	}
}
function highlight() {
	editor.surroundHTML('<span style="background-color: yellow">', '</span>');
}

function getBlogConfig() {
	var config = new HTMLArea.Config();
	config.fontname = {
		"Arial":	   'arial,helvetica,sans-serif',
		"Courier New":	   'courier new,courier,monospace',
		"Georgia":	   'georgia,times new roman,times,serif',
		"Tahoma":	   'tahoma,arial,helvetica,sans-serif',
		"Times New Roman": 'times new roman,times,serif',
		"Verdana":	   'verdana,arial,helvetica,sans-serif',
		"impact":	   'impact',
		"WingDings":	   'wingdings',
		"宋体":  '宋体',
		"楷体":  '楷体_GB2312',
		"隶书":  '隶书',
		"黑体":  '黑体',
		"幼圆":  '幼圆'
	};
	config.registerButton("code", "Code Format", "images/ed_format_code.gif", false,
		function(editor, id) {
			editor.surroundHTML('<p><table style="BORDER-RIGHT: #999999 1px dashed; BORDER-TOP: #999999 1px dashed;BORDER-LEFT: #999999 1px dashed; WIDTH: 468px; BORDER-BOTTOM: #999999 1px dashed;background-color: #f0f0f0;font-family:courier new,courier,monospace;font-size: 12px; line-height: 12px;" cellspacing="0" cellpadding="10" border="0"><tr><td>', '</td></tr></table><p>');
		}
	);
	config.toolbar[0].splice(10,0,"code");
	return config;
}
<!--
// bbCode control by
// subBlue design
// www.subBlue.com

// Startup variables
var imageTag = false;
var theSelection = false;

// Check for Browser & Platform for PC & IE specific bits
// More details from: http://www.mozilla.org/docs/web-developer/sniffer/browser_type.html
var clientPC = navigator.userAgent.toLowerCase(); // Get client info
var clientVer = parseInt(navigator.appVersion); // Get browser version

var is_ie = ((clientPC.indexOf("msie") != -1) && (clientPC.indexOf("opera") == -1));
var is_nav = ((clientPC.indexOf('mozilla')!=-1) && (clientPC.indexOf('spoofer')==-1)
                && (clientPC.indexOf('compatible') == -1) && (clientPC.indexOf('opera')==-1)
                && (clientPC.indexOf('webtv')==-1) && (clientPC.indexOf('hotjava')==-1));
var is_moz = 0;

var is_win = ((clientPC.indexOf("win")!=-1) || (clientPC.indexOf("16bit") != -1));
var is_mac = (clientPC.indexOf("mac")!=-1);

// Helpline messages
b_help = "粗体: [b]text[/b]  (alt+b)";
i_help = "大写: [i]text[/i]  (alt+i)";
u_help = "下划线: [u]text[/u]  (alt+u)";
q_help = "引用文本: [quote]text[/quote]  (alt+q)";
c_help = "显示代码 : [code]code[/code]  (alt+c)";
l_help = "列表: [list]text[/list] (alt+l)";
o_help = "按序列表: [list=]text[/list]  (alt+o)";
p_help = "插入图像: [img]http://image_url[/img]  (alt+p)";
w_help = "插入链接网址: [url=http://url]URL text[/url]  (alt+w)";
a_help = "关闭所有开启的bbCode标签";
s_help = "字体颜色: [color=red]text[/color]  提示: 您也可以使用如 color=#FF0000 这样的html语句";
f_help = "字体大小: [size=x-small]small text[/size]";

// Define the bbCode tags
bbcode = new Array();
bbtags = new Array('[b]','[/b]','[i]','[/i]','[u]','[/u]','[quote]','[/quote]','[code]','[/code]','[list]','[/list]','[list=]','[/list]','[img]','[/img]','[url=http://]','[/url]');
imageTag = false;

// Shows the help messages in the helpline window
function helpline(help) {
	document.postform.helpbox.value = eval(help + "_help");
}


// Replacement for arrayname.length property
function getarraysize(thearray) {
	for (i = 0; i < thearray.length; i++) {
		if ((thearray[i] == "undefined") || (thearray[i] == "") || (thearray[i] == null))
			return i;
		}
	return thearray.length;
}

// Replacement for arrayname.push(value) not implemented in IE until version 5.5
// Appends element to the array
function arraypush(thearray,value) {
	thearray[ getarraysize(thearray) ] = value;
}

// Replacement for arrayname.pop() not implemented in IE until version 5.5
// Removes and returns the last element of an array
function arraypop(thearray) {
	thearraysize = getarraysize(thearray);
	retval = thearray[thearraysize - 1];
	delete thearray[thearraysize - 1];
	return retval;
}


function checkForm() {
	bbstyle(-1);
	return true;
}

function emoticon(text) {
	var txtarea = document.postform.blogbody;
	text = ' ' + text + ' ';
	if (txtarea.createTextRange && txtarea.caretPos) {
		var caretPos = txtarea.caretPos;
		caretPos.text = caretPos.text.charAt(caretPos.text.length - 1) == ' ' ? caretPos.text + text + ' ' : caretPos.text + text;
		txtarea.focus();
	} else {
		txtarea.value  += text;
		txtarea.focus();
	}
}

function bbfontstyle(bbopen, bbclose) {
	var txtarea = document.postform.blogbody;

	if ((clientVer >= 4) && is_ie && is_win) {
		theSelection = document.selection.createRange().text;
		if (!theSelection) {
			txtarea.value += bbopen + bbclose;
			txtarea.focus();
			return;
		}
		document.selection.createRange().text = bbopen + theSelection + bbclose;
		txtarea.focus();
		return;
	}
	else if (txtarea.selectionEnd && (txtarea.selectionEnd - txtarea.selectionStart > 0))
	{
		mozWrap(txtarea, bbopen, bbclose);
		return;
	}
	else
	{
		txtarea.value += bbopen + bbclose;
		txtarea.focus();
	}
	storeCaret(txtarea);
}


function bbstyle(bbnumber) {
	var txtarea = document.postform.blogbody;

	txtarea.focus();
	donotinsert = false;
	theSelection = false;
	bblast = 0;

	if (bbnumber == -1) { // Close all open tags & default button names
		while (bbcode[0]) {
			butnumber = arraypop(bbcode) - 1;
			txtarea.value += bbtags[butnumber + 1];
			buttext = eval('document.postform.addbbcode' + butnumber + '.value');
			eval('document.postform.addbbcode' + butnumber + '.value ="' + buttext.substr(0,(buttext.length - 1)) + '"');
		}
		imageTag = false; // All tags are closed including image tags :D
		txtarea.focus();
		return;
	}

	if ((clientVer >= 4) && is_ie && is_win)
	{
		theSelection = document.selection.createRange().text; // Get text selection
		if (theSelection) {
			// Add tags around selection
			document.selection.createRange().text = bbtags[bbnumber] + theSelection + bbtags[bbnumber+1];
			txtarea.focus();
			theSelection = '';
			return;
		}
	}
	else if (txtarea.selectionEnd && (txtarea.selectionEnd - txtarea.selectionStart > 0))
	{
		mozWrap(txtarea, bbtags[bbnumber], bbtags[bbnumber+1]);
		return;
	}

	// Find last occurance of an open tag the same as the one just clicked
	for (i = 0; i < bbcode.length; i++) {
		if (bbcode[i] == bbnumber+1) {
			bblast = i;
			donotinsert = true;
		}
	}

	if (donotinsert) {		// Close all open tags up to the one just clicked & default button names
		while (bbcode[bblast]) {
				butnumber = arraypop(bbcode) - 1;
				txtarea.value += bbtags[butnumber + 1];
				buttext = eval('document.postform.addbbcode' + butnumber + '.value');
				eval('document.postform.addbbcode' + butnumber + '.value ="' + buttext.substr(0,(buttext.length - 1)) + '"');
				imageTag = false;
			}
			txtarea.focus();
			return;
	} else { // Open tags

		if (imageTag && (bbnumber != 14)) {		// Close image tag before adding another
			txtarea.value += bbtags[15];
			lastValue = arraypop(bbcode) - 1;	// Remove the close image tag from the list
			document.postform.addbbcode14.value = "Img";	// Return button back to normal state
			imageTag = false;
		}

		// Open tag
		txtarea.value += bbtags[bbnumber];
		if ((bbnumber == 14) && (imageTag == false)) imageTag = 1; // Check to stop additional tags after an unclosed image tag
		arraypush(bbcode,bbnumber+1);
		eval('document.postform.addbbcode'+bbnumber+'.value += "*"');
		txtarea.focus();
		return;
	}
	storeCaret(txtarea);
}

// From http://www.massless.org/mozedit/
function mozWrap(txtarea, open, close)
{
	var selLength = txtarea.textLength;
	var selStart = txtarea.selectionStart;
	var selEnd = txtarea.selectionEnd;
	if (selEnd == 1 || selEnd == 2)
		selEnd = selLength;

	var s1 = (txtarea.value).substring(0,selStart);
	var s2 = (txtarea.value).substring(selStart, selEnd)
	var s3 = (txtarea.value).substring(selEnd, selLength);
	txtarea.value = s1 + open + s2 + close + s3;
	return;
}

// Insert at Claret position. Code from
// http://www.faqts.com/knowledge_base/view.phtml/aid/1052/fid/130
function storeCaret(textEl) {
	if (textEl.createTextRange) textEl.caretPos = document.selection.createRange().duplicate();
}

//-->
