<?php


$needlogin=1;

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/userdatadefine.inc.php");

preprocess();

setStat("察看用户信息");

show_nav();

if (isErrFounded()) {
	html_error_quit() ;
} else {
	?>
	<br>
	<TABLE cellSpacing=0 cellPadding=0 width=97% border=0 align=center>
	<?php

	if ($loginok==1) {
		showUserMailbox();
?>
</table>
<?php
	}
	head_var();
	showUserData($user,$user_num);
}

//showBoardSampleIcons();
show_footer();

function preprocess() {
	global $user,$user_num;
	$userarray=array();
	if (($user_num=bbs_getuser($_GET['id'],$userarray))==0) {
		foundErr("查找用户数据失败！");
		return false;
	}
	$user=$userarray;
	return true;

}

function showUserData($user, $user_num) {
	print_r($user);
?>
<table width=97% border=0 cellspacing=0 cellpadding=3 align=center>
  <tr> 
    <td><img src="<?php
	if ($user['userface_img']==-2) {
		echo $user['userface_url'];
	} else {
		echo 'userface/image'.$user['userface_img'].'.gif';
	}
?>" width=20 height=21 align=absmiddle> 
<b><?php echo $user['userid']; ?></b> 
</td>
    <td align=right>
当前位置：[测试帖子列表]<img src=pic/zhuangtai.gif width=16 height=16 align=absmiddle> 
      状态：
正常  [在线：0Mins]
  </td>
  </tr>
</table>

<table cellspacing=1 cellpadding=3 align=center  style="table-layout:fixed;word-break:break-all" class=tableborder1>
  <col width=20% ><col width=*><col width=40% > 
  <tr> 
    <th colspan=2 align=left>基本资料</th>
    <td rowspan=9 align=center class=tablebody1 width=40% valign=top>
<font color=gray>无</font>
    </td>
  </tr>   
  <tr> 
    <td class=tablebody1 width=20% align=right>性 别：</td>
    <td class=tablebody1><?php echo $user['gender']?'男':'女'; ?> </td>
  </tr>
  <tr> 
    <td class=tablebody2 width=20% align=right>出 生：</td>
    <td class=tablebody2>
<?php
	if ( ($user['birthyear']!=0) && ($user['birthmonth']!=0) && ($user['birthday']!=0)) {
		echo $user['birthyear'].'年'.$user['birthmonth'].'月'.$user['birthday'].'日';
	} else {
		echo "<font color=gray>未知</font>";
	}?>
 </td>
  </tr>
  <tr> 
    <td class=tablebody1 width=20% align=right>星 座：</td>
    <td class=tablebody1>
<?php
	if ( ($user['birthmonth']!=0) && ($user['birthday']!=0)) {
		echo $user['birthyear'].'年'.$user['birthmonth'].'月'.$user['birthday'].'日';
	} else {
		echo "<font color=gray>未知</font>";
	}?>
?></td>
  </tr>
  <tr> 
    <td class=tablebody2 width=20% align=right>Ｅｍａｉｌ：</td>
    <td class=tablebody2>
<a href=mailto:eway@aspsky.net>eway@aspsky.net</a>
</td>
  </tr>
  <tr> 
    <td class=tablebody1 width=20% align=right>Ｑ Ｑ：</td>
    <td class=tablebody1>
<font color=gray>未填</font>
</td>
  </tr>
  <tr> 
    <td class=tablebody2 width=20% align=right>ＩＣＱ：</td>
    <td class=tablebody2>
<font color=gray>未填</font>
</td>
  </tr>
  <tr> 
    <td class=tablebody1 width=20% align=right>ＭＳＮ：</td>
    <td class=tablebody1>
<font color=gray>未填</font>
 </td>
  </tr>
  <tr> 
    <td class=tablebody2 width=20% align=right>主 页：</td>
    <td class=tablebody2>
<font color=gray>未填</font>
</td>
  </tr>
  <tr> 
    <td class=tablebody1 width=20% align=right valign=top>&nbsp;</td>
    <td class=tablebody1>&nbsp;</td>
    <td class=tablebody1 align=center width=40% >
      <b><a href="javascript:openScript('messanger.asp?action=new&touser=admin',500,400)">给他留言</a> | <a href="friendlist.asp?action=addF&myFriend=admin" target=_blank>加为好友</a></b></td>
  </tr>
</table>
<br>


<table cellspacing=1 cellpadding=3 align=center class=tableborder1>
  <tr>
    <th align=left colspan=6> 论坛属性</th>
  </tr>
  <tr>
    <td class=tablebody1 width=15% align=right>经验值：</td>

    <td  width=35%  class=tablebody1><b>94023 </b></td>
    <td width=15% align=right class=tablebody1>精华帖子：</td>
    <td width=35%  class=tablebody1> <b>0</b>篇</td>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>魅力值：</td>
    <td  width=35%  class=tablebody1><b>94023 </b></td>
    <td width=15% align=right class=tablebody1>帖子总数：</td>
    <td width=35%  class=tablebody1><b>47</b> 篇</td>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>论坛等级：</td>
    <td  width=35%  class=tablebody1><b>管理员 </b></td>
    <td width=15% align=right class=tablebody1>被删主题：</td>
    <td width=35%  class=tablebody1><b><font color=#FF0000>-2</font></b> 
      篇</td>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>威望值：</td>
    <td  width=35%  class=tablebody1><b><font color=#FF0000>1000</font> </b></td>
    <td width=15% align=right class=tablebody1>被删除率：</td>
<td width=35%  class=tablebody1><b></b> <font color=#FF0000><b>
4.26%
</b></font> 
    </td>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>门  派：</td>
    <td  width=35%  class=tablebody1><b>
无门无派
 </b></td>
    <td class=tablebody1 width=15% align=right>登陆次数：</td>
    <td width=35%  class=tablebody1><b>18</b> 
    </td>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>注册日期：</td>
    <td  width=35%  class=tablebody1><b>2002-5-19 1:19:38</b></td>
    <td width=15% align=right class=tablebody1>上次登录：</td>
    <td width=35%  class=tablebody1><b>2003-7-31 11:14:03</b></td>
  </tr>
</table>
<br>
<table cellspacing=1 cellpadding=3 align=center class=tableborder1>
  <tr> 
    <th align=left colspan=4>
      资产情况</th>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>现金货币：</td>
    <td width=35%  class=tablebody1><b>15340</b></td>
    <td colspan=2 valign=top rowspan=4 class=tablebody1>论坛职务：
      <hr size=1 width=100 align=left>
管理员<br>
      </td>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>股票市值：</td>
    <td  width=35%  class=tablebody1><b>0</b></td>
  </tr>

  <tr> 
    <td class=tablebody1 width=15% align=right>银行存款：</td>
    <td width=35%  class=tablebody1><b>0</b></td>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>总 资 产：</td>
    <td width=35%  class=tablebody1><b>15340</b></td>
  </tr>
</table>
<br>
<table cellspacing=1 cellpadding=3 align=center class=tableborder1>
  <tr> 
    <th align=left id=tabletitlelink>
      最近活动－－<a href="queryResult.asp?stype=1&nSearch=3&keyword=admin&SearchDate=ALL&boardid=0">搜索该用户所有帖子</a></th>
  </tr>
<tr> 
    <td class=tablebody1 align=left>
&nbsp;<img src=face/face1.gif width=14 height=14>&nbsp;<a href=dispbbs.asp?boardid=1&replyid=15&id=7&skin=1>jkhkh...</a>&nbsp;--&nbsp;2003-7-25 17:19:56<br>&nbsp;<img src=face/face1.gif width=14 height=14>&nbsp;<a href=dispbbs.asp?boardid=1&replyid=14&id=7&skin=1>khjklj...</a>&nbsp;--&nbsp;2003-7-25 17:19:12<br>&nbsp;<img src=face/face1.gif width=14 height=14>&nbsp;<a href=dispbbs.asp?boardid=1&replyid=13&id=7&skin=1>jkhj,...</a>&nbsp;--&nbsp;2003-7-25 17:18:44<br>&nbsp;<img src=face/face1.gif width=14 height=14>&nbsp;<a href=dispbbs.asp?boardid=1&replyid=12&id=7&skin=1>kjkj...</a>&nbsp;--&nbsp;2003-7-25 17:18:35<br>&nbsp;<img src=face/face1.gif width=14 height=14>&nbsp;<a href=dispbbs.asp?boardid=1&replyid=11&id=7&skin=1>nmhjhjk...</a>&nbsp;--&nbsp;2003-7-25 17:18:25<br>
</td>
  </tr>
</table>
<BR>

<table class=tableborder1 cellspacing=1 cellpadding=3 align=center>
<tr><th height="25" align=left colspan=2>快捷管理选项</th></tr>

<tr><td class=tablebody1 height=25 colspan=2>
<B>用户管理选项</B>：   『 <a href=admin_lockuser.asp?action=lock_1&name=admin title=锁定该用户不允许登陆和发言>锁定</a> | <a href=admin_lockuser.asp?action=lock_2&name=admin title=屏蔽该用户在论坛的发言>屏蔽</a> | <a href=admin_lockuser.asp?action=lock_3&name=admin title=解除该用户在论坛的锁定和屏蔽>解除</a> | <a href="admin_lockuser.asp?action=power&name=admin" title=对用户进行分值奖励>奖励</a> | <a href="admin_lockuser.asp?action=getpermission&name=admin&userid=1">编辑该用户论坛权限</a> 』
</td></tr>

<tr>
<FORM METHOD=POST ACTION="admin_lockuser.asp?action=DelTopic">
<td class=tablebody1 valign=middle width="50%">
<B>帖子管理选项</B>：   删除该用户&nbsp;
<input type="hidden" value="1" name="SetUserID">
<input type="hidden" value="admin" name="name">
<select name="delTopicDate" size=1>

<option value="1">1</option>

<option value="2">2</option>

<option value="3">3</option>

<option value="4">4</option>

<option value="5">5</option>

<option value="6">6</option>

<option value="7">7</option>

<option value="8">8</option>

<option value="9">9</option>

<option value="10">10</option>

</select>&nbsp;天内的主题&nbsp;<input type="submit" name="submit" value="执行">
</td>
</form>
<FORM METHOD=POST ACTION="admin_lockuser.asp?action=DelUserReply">
<td class=tablebody1 width="50%">
   删除该用户&nbsp;
<input type="hidden" value="1" name="SetUserID">
<input type="hidden" value="admin" name="name">
<select name="delTopicDate" size=1>

<option value="1">1</option>

<option value="2">2</option>

<option value="3">3</option>

<option value="4">4</option>

<option value="5">5</option>

<option value="6">6</option>

<option value="7">7</option>

<option value="8">8</option>

<option value="9">9</option>

<option value="10">10</option>

</select>&nbsp;天内的回复在 
<select size=1 name="delbbs">
<option value="bbs1" selected >数据表1</option>
</select>
 中&nbsp;<input type="submit" name="submit" value="执行">
</td>
</FORM></tr>

<tr><td class=tablebody1 valign=middle height=25 colspan=2>
<B>用户最后来访IP</B>：   <a href="look_ip.asp?action=lookip&ip=127.0.0.1">127.0.0.1</a>&nbsp;&nbsp;点击IP查看用户来源及操作
</td></tr>

</table>

<?php
}

?>