<?
  session_start();
  $_SESSION["ReflashTime"]=time();
?>
<?php require("conn.php"); ?>
<?php require("inc/const.php"); ?>
<?php require("inc/chkinput.php"); ?>
<?php 
//=========================================================

// File: smallpaper.php

// Version:5.0

// Date: 2002-9-10

// Script Written by satan

//=========================================================

// Copyright (C) 2001,2002 AspSky.Net. All rights reserved.

// Web: http://www.phpsky.net,http://www.dvbbs.net

// Email: info@aspsky.net,eway@aspsky.net

//=========================================================


$cansmallpaper=false;
$Stats="发布小字报";
if ($Boardid==0)
{

  $Founderr=true;
  $Errmsg.="<br><li>请选择您要发布小字报的版面！";
} 



if (intval($GroupSetting[17])==0)
{

  $Errmsg.="<br><li>您没有发布小字报的权限，请<a href=login.php>登陆</a>或者同管理员联系。";
  $Founderr=true;
}
  else
{

  if (!$Founduser)
  {

    $membername="客人";
  } 

  $cansmallpaper=true;
} 


if ($Founderr)
{

nav();
head_var(2,0,"","");
dvbbs_error();
}
  else
{

nav();
head_var(1,$BoardDepth,0,0);
  if ($_REQUEST['action']=="savepaper")
  {

savepaper();
  }
    else
  {

main();
  } 

activeonline();
  if ($Founderr)
  {
dvbbs_error();  } 

} 

footer();
function main()
{
  extract($GLOBALS);


$conn->query("delete from SmallPaper where s_addtime<subdate(Now(),interval 2 day)");
?>
<form action="smallpaper.php?action=savepaper" method="post"> 
        <table cellpadding=6 cellspacing=1 align=center class=tableborder1>
    <tr>
    <th valign=middle colspan=2>
    请详细填写以下信息(<?php   echo $msg; ?>)</th></tr>
    <tr>
    <td class=tablebody1 valign=middle><b>用户名</b></td>
    <td class=tablebody1 valign=middle><INPUT name=username type=text value="<?php   echo $membername; ?>"> &nbsp; <a href="reg.php">没有注册？</a></td></tr>
    <tr>
    <td class=tablebody1 valign=middle><b>密 码</b></font></td>
    <td class=tablebody1 valign=middle><INPUT name=password type=password value="<?php   echo $memberword; ?>"> &nbsp; <a href="lostpass.php">忘记密码？</a></td></tr>
    <tr>
    <td class=tablebody1 valign=middle><b>标 题</b>(最多80字)</td>
    <td class=tablebody1 valign=middle><INPUT name="title" type=text size=60></td></tr>
    <tr>
    <td class=tablebody1 valign=top width=30%>
<b>内 容</b><BR>
在本版发布小字报将您将付<font color="<?php   echo $Forum_body[8]; ?>"><b><?php   echo $GroupSetting[46]; ?></b></font>元费用<br>
<font color="<?php   echo $Forum_body[8]; ?>"><b>48</b></font>小时内发表的小字报将随机抽取<font color="<?php   echo $Forum_body[8]; ?>"><b>5</b></font>条滚动显示于论坛上<br>
<li>HTML标签： <?php   if ($Forum_Setting[35]==0)
  {
?>不可用<?php   }
    else
  {
?>允许<?php   } ?>
<li>UBB 标签： <?php   if ($Forum_Setting[34]==0)
  {
?>不可用<?php   }
    else
  {
?>允许<?php   } ?>
<li>内容不得超过500字
</td>
    <td class=tablebody1 valign=middle>
<textarea class="smallarea" cols="60" name="Content" rows="8" wrap="VIRTUAL"></textarea>
<INPUT name="boardid" type=hidden value="<?php   echo $Boardid; ?>">
                </td></tr>
    <tr>
    <td class=tablebody2 valign=middle colspan=2 align=center><input type=submit name="submit" value="发 布"></td></tr></table>
</form>
<?php   return $function_ret;
} ?>
<?php 
function savepaper()
{
  extract($GLOBALS);
  global $Founderr,$Errmsg,$sucmsg;


  $UserName=checkStr(trim($_POST["username"]));
  $PassWord=checkStr(trim($_POST["password"]));
  $title=checkStr(trim($_POST["title"]));
  $Content=checkStr($_POST["Content"]);
  if ($chkpost==false)
  {

    $Errmsg.="<br><li>您提交的数据不合法，请不要从外部提交发言。";
    $Founderr=true;
  } 

  if ($UserName=="")
  {

    $Errmsg.="<br><li>请输入姓名";
    $Founderr=true;
  } 

  if ($title=="")
  {

    $Founderr=true;
    $Errmsg.="<br><li>主题不应为空。";
  }
    else
  if (strlen($title)>80)
  {

    $Founderr=true;
    $Errmsg.="<br><li>主题长度不能超过80";
  } 

  if ($Content=="")
  {

    $Errmsg.="<br><li>没有填写内容。";
    $Founderr=true;
  }
    else
  if (strlen($content)>500)
  {

    $Errmsg.="<br><li>发言内容不得大于500";
    $Founderr=true;
  } 


//客人不允许发，验证用户

  if (!$Founderr && $cansmallpaper)
  {

    if ($PassWord!=$memberword)
    {
      $PassWord=md5($PassWord);
    } 

// $rs is of type "adodb.recordset"

    $sql="Select userWealth From user Where UserName='".$UserName."' and UserPassWord='".$PassWord."'";
	$rs=$conn->getRow($sql);
    if ($rs!=null)
    {

      if (intval($rs[0])<intval($GroupSetting[46]))
      {

        $Errmsg.="<br><li>您没有足够的金钱来发布小字报，快到论坛浇点水吧。";
        $Founderr=true;
      }
        else
      {

		$rs[0]=$rs[0]-intval($GroupSetting[46]);
		$conn->query("update user set userWealth=".$rs[0]." where UserName=".$UserName);
                
      } 

    } 

    
    $rs=null;

  } 

  if ($Founderr)
  {

    return $function_ret;

  }
    else
  {

    $sql="insert into SmallPaper (s_boardid,s_username,s_title,s_content,s_addtime) values ".
      "(".
      $Boardid.",'".
      $UserName."','".
      $title."','".
      $Content."','".
	  strftime("%Y-%m-%d %H:%M:%S")."')";
//response.write sql

$conn->query($sql);
    $sucmsg="<li>您成功的发布了小字报。";
dvbbs_suc();
  } 

  return $function_ret;
} 
?>
