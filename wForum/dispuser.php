<?php


$needlogin=1;

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/ubbcode.php");

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
require("inc/userdatadefine.inc.php");
$flag=1<<29;
if ($user['userdefine'] & $flag) {
?>
<table width=97% border=0 cellspacing=0 cellpadding=3 align=center>
  <tr> 
    <td><img src="<?php
	if ($user['userface_img']==-2) {
		echo $user['userface_url'];
	} else {
		echo 'userface/image'.$user['userface_img'].'.gif';
	}
?>" width=<?php echo $currentuser['userface_width'];  ?> height=<?php echo $currentuser['userface_height'];  ?> align=absmiddle> 
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
    <td class=tablebody1><?php echo $user['gender']==1?'男':'女'; ?> </td>
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
	echo get_astro($user['birthmonth'],$user['birthday']);
?></td>
  </tr>
  <tr> 
    <td class=tablebody2 width=20% align=right>Ｅｍａｉｌ：</td>
    <td class=tablebody2>
	<?php 
	$reg_email=htmlspecialchars(trim($user['reg_email']),ENT_QUOTES);
	if ($reg_email!='') {
		echo '<a href=mailto:'.$reg_email.'>'.$reg_email.'</a>'; 
	} else {
		echo "<font color=gray>未知</font>";
	}
	?>
</td>
  </tr>
  <tr> 
    <td class=tablebody1 width=20% align=right>Ｑ Ｑ：</td>
    <td class=tablebody1>
	<?php echo showIt($user['OICQ']); ?>
</td>
  </tr>
  <tr> 
    <td class=tablebody2 width=20% align=right>ＩＣＱ：</td>
    <td class=tablebody2>
	<?php echo showIt($user['ICQ']); ?>
</td>
  </tr>
  <tr> 
    <td class=tablebody1 width=20% align=right>ＭＳＮ：</td>
    <td class=tablebody1>
	<?php echo showIt($user['MSN']); ?>
 </td>
  </tr>
  <tr> 
    <td class=tablebody2 width=20% align=right>主 页：</td>
    <td class=tablebody2>
	<?php 
	$homepage=htmlspecialchars(trim($user['homepage']),ENT_QUOTES);
	if ($homepage!='') {
		echo '<a href="'.$homepage.'" target="_blank">'.$homepage.'</a>'; 
	} else {
		echo "<font color=gray>未知</font>";
	}
	?>
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
<?php
}
$flag=1<<30;
if ($user['userdefine'] & $flag) {
?>
<table cellspacing=1 cellpadding=3 align=center class=tableborder1 style="table-layout:fixed;word-break:break-all">
  <col width=20% ><col width=*><col width=40% > 
  <tr> 
    <th colspan=2 align=left>
      用户详细资料</th>
    <td rowspan=14 class=tablebody1 width=40% valign=top>
<b>性格：</b>
<br>
<?php   echo $character[$user['character']]; ?>
<br><br><br>
<b>个人简介：</b><br>
<?php   
	$filename=bbs_sethomefile($user["userid"],"plans");
	if (is_file($filename)) {
		echo dvbcode(bbs_printansifile($filename),0);
	} else {
		echo "<font color=gray>这个家伙很懒，什么也没有留下^_^</font>";
	}
?>
<br>
</td>
  </tr>   
  <tr> 
    <td class=tablebody1 width=20% align=right>真实姓名：</td>
    <td class=tablebody1><?php echo showIt($user['username']);	?></td>
  </tr>
  <tr> 
    <td class=tablebody2 width=20% align=right>国　　家：</td>
    <td class=tablebody2><?php echo showIt($user['country']); ?> </td>
  </tr>
  <tr> 
    <td class=tablebody1 width=20% align=right>省　　份：</td>
    <td class=tablebody1><?php echo showIt($user['province']); ?></td>
  </tr>
  <tr> 
    <td class=tablebody2 width=20% align=right>城　　市：</td>
    <td class=tablebody2><?php  echo showIt($user['city']); ?></td>
  </tr>
  <tr> 
    <td class=tablebody1 width=20% align=right>联系电话：</td>
    <td class=tablebody1>	<?php echo showIt(substr($user['realemail'],strpos($user['realemail'],'$')+1,strpos($user['realemail'],'@')-strpos($user['realemail'],'$')-1)); ?></td>
  </tr>
  <tr> 
    <td class=tablebody2 width=20% align=right>通信地址：</td>
    <td class=tablebody2><?php   echo showIt($user['address']); ?></td>
  </tr>

  <tr> 
    <td class=tablebody1 width=20% align=right>生　　肖：</td>
    <td class=tablebody1><?php echo showIt($shengxiao[$user['shengxiao']]); ?> </td>
  </tr>
  <tr> 
    <td class=tablebody2 width=20% align=right>血　　型：</td>
    <td class=tablebody2><?php    echo showIt($bloodtype[$user['bloodtype']]); ?></td>
  </tr>
  <tr> 
    <td class=tablebody1 width=20% align=right>信　　仰：</td>
    <td class=tablebody1><?php    echo showIt($religion[$user['religion']]) ?></td>
  </tr>
  <tr> 
    <td class=tablebody2 width=20% align=right>职　　业：</td>
    <td class=tablebody2><?php    echo showIt($profession[$user['profession']]); ?></td>
  </tr>
  <tr> 
    <td class=tablebody1 width=20% align=right>婚姻状况：</td>
    <td class=tablebody1><?php    echo showIt($married[$user['married']]); ?></td>
  </tr>
  <tr> 
    <td class=tablebody2 width=20% align=right>最高学历：</td>
    <td class=tablebody2><?php    echo showIt($education[$user['education']]); ?></td>
  </tr>
  <tr> 
    <td class=tablebody1 width=20% align=right>毕业院校：</td>
    <td class=tablebody1><?php    echo showIt($user['graduateschool']); ?></td>
  </tr></table>
<br>
<?php
}
?>
<table cellspacing=1 cellpadding=3 align=center class=tableborder1>
  <tr>
    <th align=left colspan=6> 论坛属性</th>
  </tr>
  <tr>
    <td class=tablebody1 width=15% align=right>积分：</td>

    <td  width=35%  class=tablebody1><b><?php echo $user['score']; ?> </b></td>
    <td width=15% align=right class=tablebody1>精华帖子：</td>
    <td width=35%  class=tablebody1> <b>N/A</b>篇</td>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>魅力值：</td>
    <td  width=35%  class=tablebody1><b>N/A </b></td>
    <td width=15% align=right class=tablebody1>帖子总数：</td>
    <td width=35%  class=tablebody1><b><?php echo $user['numposts']; ?></b> 篇</td>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>论坛等级：</td>
    <td  width=35%  class=tablebody1><b><?php echo bbs_getuserlevel($user['userid']); ?> </b></td>
    <td width=15% align=right class=tablebody1>被删主题：</td>
    <td width=35%  class=tablebody1><b><font color=#FF0000>N/A</font></b> 
      篇</td>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>威望值：</td>
    <td  width=35%  class=tablebody1><b><font color=#FF0000>N/A</font> </b></td>
    <td width=15% align=right class=tablebody1>被删除率：</td>
<td width=35%  class=tablebody1><b></b> <font color=#FF0000><b>
N/A
</b></font> 
    </td>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>门  派：</td>
    <td  width=35%  class=tablebody1><b>
<?php echo showIt($groups[$user['group']]); ?>
 </b></td>
    <td class=tablebody1 width=15% align=right>登陆次数：</td>
    <td width=35%  class=tablebody1><b><?php echo $user['numlogins']; ?></b> 
    </td>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>注册日期：</td>
    <td  width=35%  class=tablebody1><b><?php echo strftime("%Y-%m-%d %H:%M:%S", $user['firstlogin']); ?></b></td>
    <td width=15% align=right class=tablebody1>上次登录：</td>
    <td width=35%  class=tablebody1><b><?php echo strftime("%Y-%m-%d %H:%M:%S", $user['lastlogin']); ?></b></td>
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
    <td width=35%  class=tablebody1><b><?php echo $user['money']; ?></b></td>
    <td colspan=2 valign=top rowspan=4 class=tablebody1>论坛职务：
      <hr size=1 width=100 align=left>
<?php echo bbs_getuserlevel($user['userid']); ?><br>
      </td>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>股票市值：</td>
    <td  width=35%  class=tablebody1><b>N/A</b></td>
  </tr>

  <tr> 
    <td class=tablebody1 width=15% align=right>银行存款：</td>
    <td width=35%  class=tablebody1><b>N/A</b></td>
  </tr>
  <tr> 
    <td class=tablebody1 width=15% align=right>总 资 产：</td>
    <td width=35%  class=tablebody1><b>N/A</b></td>
  </tr>
</table>
<br>

<table class=tableborder1 cellspacing=1 cellpadding=3 align=center>
<tr><th height="25" align=left colspan=2>快捷管理选项</th></tr>

<tr><td class=tablebody1 height=25 colspan=2>
<B>用户管理选项</B>：   『 <a href=# onclick="alert('该功能尚在开发中。');" title=锁定该用户不允许登陆和发言>锁定</a> | <a href=# onclick="alert('该功能尚在开发中。');" title=屏蔽该用户在论坛的发言>屏蔽</a> | <a href=# onclick="alert('该功能尚在开发中。');" title=解除该用户在论坛的锁定和屏蔽>解除</a> | <a href=# onclick="alert('该功能尚在开发中。');" title=对用户进行分值奖励>奖励</a> | <a href=# onclick="alert('该功能尚在开发中。');">编辑该用户论坛权限</a> 』
</td></tr>



<tr><td class=tablebody1 valign=middle height=25 colspan=2>
<B>用户最后来访IP</B>：   <a href=# onclick="alert('该功能尚在开发中。');"><?php echo $user['lasthost']; ?></a>&nbsp;&nbsp;点击IP查看用户来源及操作
</td></tr>

</table>

<?php
}

?>