<?php
        
    require("funcs.php");

	$hostname=bbs_sysconf_str("MYSQLHOST");
	$username=bbs_sysconf_str("MYSQLUSER");
	$password=bbs_sysconf_str("MYSQLPASSWORD");
	$dbname=bbs_sysconf_str("MYSQLSMSDATABASE");

function al_print_form($al){

?>
<table border="0">
<tr><td>姓名:</td><td><input name="t_name" maxlength=13 size=13 value=<?php echo htmlspecialchars($al[2]);?>></td></tr>
<tr><td>分组:</td><td><input name="t_group" maxlength=13 size=13 value=<?php echo htmlspecialchars($al[15]);?>></td></tr>
<tr><td>bbsid:</td><td><input name="t_bbsid" maxlength=13 size=13 value=<?php echo htmlspecialchars($al[3]);?>></td></tr>
<tr><td>学校:</td><td><input name="t_school" size=50  value=<?php echo htmlspecialchars($al[4]);?>></td></tr>
<tr><td>邮政编码:</td><td><input name="t_zipcode" maxlength=6 size=6  value=<?php echo htmlspecialchars($al[5]);?>></td></tr>
<tr><td>家庭住址:</td><td><input name="t_homeaddr" size=50 value=<?php echo htmlspecialchars($al[6]);?>></td></tr>
<tr><td>工作地址:</td><td><input name="t_companyaddr" size=50 value=<?php echo htmlspecialchars($al[7]);?>></td></tr>
<tr><td>家庭电话:</td><td><input name="t_tel_h" maxlength=20 size=20 value=<?php echo htmlspecialchars($al[8]);?>></td></tr>
<tr><td>工作电话:</td><td><input name="t_tel_o" maxlength=20 size=20 value=<?php echo htmlspecialchars($al[9]);?>></td></tr>
<tr><td>手机号码:</td><td><input name="t_mobile" maxlength=13 size=13 value=<?php echo htmlspecialchars($al[10]);?>></td></tr>
<tr><td>email:</td><td><input name="t_email" maxlength=30 size=30 value=<?php echo htmlspecialchars($al[11]);?>></td></tr>
<tr><td>qq:</td><td><input name="t_qq" maxlength=10 size=10 value=<?php echo htmlspecialchars($al[12]);?>></td></tr>
<tr><td>生日:</td><td><input name="t_birth_year" maxlength=4 size=4 value=<?php echo substr($al[13],0,4);?>>年<input name="t_birth_month" maxlength=2 size=2 value=<?php echo substr($al[13],5,2);?>>月<input name="t_birth_day" maxlength=2 size=2 value=<?php echo substr($al[13],8,2);?>>日</td></tr>
<tr><td>备注:</td><td><input name="t_memo" size=50 value=<?php echo htmlspecialchars($al[14]);?>></td></tr>
</table>
<center>
<input type=submit name=submit value="提交">
<input type=reset name=reset value="重置">
</center>
</form>

<?php

}
    if($loginok !=1)
    	html_nologin(); 
    else
	{
		html_init("gb2312");

		if( $_GET["start"] ){
			$startnum = $_GET["start"];
		}else if( $_POST["start"] ){
			$startnum = $_POST["start"];
		}else
			$startnum = 0;
		settype($startnum,"integer");

		if( $_GET["count"] ){
			$count = $_GET["count"];
		}else if( $_POST["count"] ){
			$count = $_POST["count"];
		}else
			$count = 10;
		settype($count,"integer");

		if( $_GET["desc"] ){
			$desc = $_GET["desc"];
		}else if( $_POST["desc"] ){
			$desc = $_POST["desc"];
		}else
			$desc = 0;
		settype($desc, "integer");
		if($desc == 1)
			$descstr="DESC";
		else
			$descstr="ASC";

		if( $_GET["order"] ){
			$order = $_GET["order"];
		}else if( $_POST["order"] ){
			$order = $_POST["order"];
		}
		if( $order != "bbsid" && $order != "name" )
			$order="groupname";

		$db = mysql_connect($hostname, $username, $password) or die(mysql_error());
		mysql_select_db($dbname, $db) or die(mysql_error());

		if( $_GET["action"] ){
			$action = $_GET["action"];
		}

		if(isset($action) && $action == "del"){
			if( $_GET["id"] ){
				$id = $_GET["id"];
			}
			else{
				html_error_quit("参数错误1");
			}
			settype($id, "integer");
			if($id < 0)
				html_error_quit("参数错误2");

			$sqlstr = "DELETE FROM addr WHERE userid=\"".$currentuser["userid"]."\" AND id=".$id;
			$result = mysql_query($sqlstr) or die(mysql_error());

		}else if(isset($action) && $action=="detail"){
			if( $_GET["id"] ){
				$id = $_GET["id"];
			}
			else{
				html_error_quit("参数错误1");
			}
			settype($id, "integer");
			if($id < 0)
				html_error_quit("参数错误2");

			$sqlstr = "SELECT * FROM addr WHERE userid=\"".$currentuser["userid"]."\" AND id=".$id;

			$result = mysql_query($sqlstr) or die(mysql_error());

			if( ! $result ){
				html_error_quit("数据错误");
			}else{
				$row = mysql_fetch_row($result);
				if( ! $row )
					html_error_quit("数据错误1");
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- 通讯录条目显示 [用户: <?php echo $currentuser["userid"]; ?>]</p>
<a href="javascript:location=location">刷新</a>
</center>
<hr class=default>
<table border="1" width="613" align="center" cellpadding="0" cellspacing="0">
<tr><td>姓名</td><td><?php echo htmlspecialchars($row[2]);?></td></tr>
<tr><td>bbsid</td><td><?php echo htmlspecialchars($row[3]);?></td></tr>
<tr><td>分组</td><td><?php echo htmlspecialchars($row[15]);?></td></tr>
<tr><td>生日</td><td><?php echo htmlspecialchars($row[13]);?></td></tr>
<tr><td>学校</td><td><?php echo htmlspecialchars($row[4]);?></td></tr>
<tr><td>邮政编码</td><td><?php echo htmlspecialchars($row[5]);?></td></tr>
<tr><td>家庭地址</td><td><?php echo htmlspecialchars($row[6]);?></td></tr>
<tr><td>家庭电话</td><td><?php echo htmlspecialchars($row[9]);?></td></tr>
<tr><td>公司地址</td><td><?php echo htmlspecialchars($row[7]);?></td></tr>
<tr><td>公司电话</td><td><?php echo htmlspecialchars($row[8]);?></td></tr>
<tr><td>手机</td><td><?php echo htmlspecialchars($row[10]);?></td></tr>
<tr><td>email</td><td><?php echo htmlspecialchars($row[11]);?></td></tr>
<tr><td>qq</td><td><?php echo htmlspecialchars($row[12]);?></td></tr>
<tr><td>备注</td><td><pre><?php echo $row[14];?></pre></td></tr>
</table>
<center>
<a href="javascript:history.go(-1)">返回</a>
</center>
</body>
<?php
				html_normal_quit();
			}
		}else if(isset($action) && ( $action=="add" || $action=="edit" ) ){
			if( $action == "edit" ){
				if( $_GET["id"] ){
					$edit_id = $_GET["id"];
				}else if ($_POST["id"]){
					$edit_id = $_POST["id"];
				}
				else
					html_error_quit("参数错误");

				settype($edit_id, "integer");
				if($edit_id < 0)
					html_error_quit("参数错误7");
			}else {
				$sqlstr = "SELECT COUNT(*) FROM addr WHERE userid=\"".$currentuser["userid"];

				$result = mysql_query($sqlstr) or die(mysql_error());
				if( ! $result ){
					html_error_quit("数据错误");
				}else{
					$row = mysql_fetch_row($result);
					if( ! $row )
						html_error_quit("数据错误1");
					$allcount = $row[0];
					settype($allcount, "integer");
echo $allcount;
					if( $allcount > 400 )
						html_error_quit("通讯录满");
				}
			}


			if( $_GET["submit"] || $_POST["submit"] ){
				if( $_POST["t_name"] ){
					$t_name = $_POST["t_name"];
				}else{
					html_error_quit("用户名输入错误");
				}
				$t_name = substr($t_name,0,12);
				//$t_name[12]=0;

				if( $_POST["t_group"] ){
					$t_group = $_POST["t_group"];
					$t_group = substr($t_group,0,12);
					//$t_group[12]=0;
				}else{
					$t_group="";
				}

				if( $_POST["t_bbsid"] ){
					$t_bbsid = $_POST["t_bbsid"];
					$t_bbsid = substr($t_bbsid,0,12);
					//$t_bbsid[12]=0;
				}else{
					$t_bbsid="";
				}

				if( $_POST["t_school"] ){
					$t_school = $_POST["t_school"];
					$t_school = substr($t_school,0,99);
					//$t_school[99]=0;
				}else{
					$t_school="";
				}

				if( $_POST["t_zipcode"] ){
					$t_zipcode = $_POST["t_zipcode"];
					$t_zipcode = substr($t_zipcode,0,6);
					//$t_zipcode[6]=0;
				}else{
					$t_zipcode="";
				}

				if( $_POST["t_homeaddr"] ){
					$t_homeaddr = $_POST["t_homeaddr"];
					$t_homeaddr = substr($t_homeaddr,0,99);
					//$t_homeaddr[99]=0;
				}else{
					$t_homeaddr="";
				}

				if( $_POST["t_companyaddr"] ){
					$t_companyaddr = $_POST["t_companyaddr"];
					$t_companyaddr = substr($t_companyaddr,0,99);
					//$t_companyaddr[99]=0;
				}else{
					$t_companyaddr="";
				}

				if( $_POST["t_tel_h"] ){
					$t_tel_h = $_POST["t_tel_h"];
					//$t_tel_h[19]=0;
					$t_tel_h = substr($t_tel_h,0,19);
				}else{
					$t_tel_h="";
				}

				if( $_POST["t_tel_o"] ){
					$t_tel_o = $_POST["t_tel_o"];
					$t_tel_o = substr($t_tel_o,0,19);
					//$t_tel_o[19]=0;
				}else{
					$t_tel_o="";
				}

				if( $_POST["t_mobile"] ){
					$t_mobile = $_POST["t_mobile"];
					$t_mobile = substr($t_mobile,0,12);
					//$t_mobile[12]=0;
				}else{
					$t_mobile="";
				}

				if( $_POST["t_email"] ){
					$t_email = $_POST["t_email"];
					$t_email = substr($t_email,0,29);
					//$t_email[29]=0;
				}else{
					$t_email="";
				}

				if( $_POST["t_qq"] ){
					$t_qq = $_POST["t_qq"];
					$t_qq = substr($t_qq,0,9);
					//$t_qq[9]=0;
				}else{
					$t_qq="";
				}

				if( $_POST["t_birth_year"] ){
					$t_birth_year = $_POST["t_birth_year"];
					settype($t_birth_year, "integer");
					if( $t_birth_year < 1000 || $t_birth_year > 9000 )
						$t_birth_year=1900;
				}else{
					$t_birth_year = 1900;
				}

				if( $_POST["t_birth_month"] ){
					$t_birth_month = $_POST["t_birth_month"];
					settype($t_birth_month, "integer");
					if( $t_birth_month < 1 || $t_birth_month > 12 )
						$t_birth_month=1;
				}else{
					$t_birth_month = 1;
				}

				if( $_POST["t_birth_day"] ){
					$t_birth_day = $_POST["t_birth_day"];
					settype($t_birth_day, "integer");
					if( $t_birth_day < 1 || $t_birth_day > 12 )
						$t_birth_day=1;
				}else{
					$t_birth_day = 1;
				}

				if( $_POST["t_memo"] ){
					$t_memo = $_POST["t_memo"];
					$t_memo = substr( $t_memo, 0, 99);
					//$t_memo{99}=0;
				}else{
					$t_memo="";
				}

				if( $action == "add" ){
					$sqlstr = "INSERT INTO addr VALUES (NULL, '".$currentuser["userid"]."', '".addslashes($t_name)."','".addslashes($t_bbsid)."','".addslashes($t_school)."','".addslashes($t_zipcode)."','".addslashes($t_homeaddr)."','".addslashes($t_companyaddr)."','".addslashes($t_tel_o)."','".addslashes($t_tel_h)."','".addslashes($t_mobile)."','".addslashes($t_email)."','".addslashes($t_qq)."',\"".$t_birth_year."-".$t_birth_month."-".$t_birth_day."\",'".addslashes($t_memo)."','".addslashes($t_group)."' );";
				}else{
					$sqlstr = "UPDATE addr SET name='".addslashes($t_name)."',bbsid='".addslashes($t_bbsid)."',school='".addslashes($t_school)."',zipcode='".addslashes($t_zipcode)."',homeaddr='".addslashes($t_homeaddr)."',companyaddr='".addslashes($t_companyaddr)."',tel_o='".addslashes($t_tel_o)."',tel_h='".addslashes($t_tel_h)."',mobile='".addslashes($t_mobile)."',email='".addslashes($t_email)."',qq='".addslashes($t_qq)."',birthday=\"".$t_birth_year."-".$t_birth_month."-".$t_birth_day."\",memo='".addslashes($t_memo)."',groupname='".addslashes($t_group)."' WHERE id=$edit_id AND userid='".$currentuser["userid"]."' ;";
				}
				//echo $sqlstr;

				$result = mysql_query($sqlstr) or die(mysql_error());

				if( !$result ){
					html_error_quit("失败");
				}else{
?>
<a href="/bbsal.php?start=<?php echo $startnum;?>&count=<?php echo $count;?>&order=<?php echo $order;?>&desc=<?php echo $desc;?>"><?php if($action == "add") echo "添加"; else echo "修改"; ?>成功，返回</a>
<?php
					html_normal_quit();
				}
			}else if($action == "add"){
?>
<center><p><?php echo BBS_FULL_NAME; ?> -- 通讯录条目增加 [用户: <?php echo $currentuser["userid"]; ?>]</p>
</center>
<hr class=default>
<form action="/bbsal.php?start=<?php echo $startnum;?>&count=<?php echo $count;?>&order=<?php echo $order;?>&desc=<?php echo $desc;?>&action=add" method=post>
<?php
				al_print_form($al);

				html_normal_quit();
			}else{

				$sqlstr = "SELECT * FROM addr WHERE userid=\"".$currentuser["userid"]."\" AND id=".$edit_id;

				$result = mysql_query($sqlstr) or die(mysql_error());

				if( ! $result ){
					html_error_quit("数据错误");
				}else{
					$row = mysql_fetch_row($result);
					if( ! $row )
						html_error_quit("数据错误1");
?>
<center><p><?php echo BBS_FULL_NAME; ?> -- 通讯录条目修改 [用户: <?php echo $currentuser["userid"]; ?>]</p>
</center>
<hr class=default>
<form action="/bbsal.php?start=<?php echo $startnum;?>&count=<?php echo $count;?>&order=<?php echo $order;?>&desc=<?php echo $desc;?>&action=edit&id=<?php echo $edit_id;?>" method=post>
<?php
					al_print_form($row);
					html_normal_quit();
				}
			}
		}

		$sqlstr = "SELECT * FROM addr WHERE userid=\"".$currentuser["userid"]."\" ORDER BY ".$order." ".$descstr." LIMIT ".$startnum.",".$count;

		$result = mysql_query($sqlstr) or die(mysql_error());

		if( ! $result ){
			html_error_quit("没有数据");
		}else{
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- 通讯录管理 [用户: <?php echo $currentuser["userid"]; ?>]</p>
<a href="javascript:location=location">刷新</a>
</center>
<hr class=default>
<table border="1" width="613" align="center" cellpadding="0" cellspacing="0">
<tr><td>序号</td><td>姓名</td><td>bbsid</td><td>分组</td><td>注释</td><td></td></tr>
<?php
			$i=0;
			while($row = mysql_fetch_row($result)){
?>
<tr>
<td><?php echo $startnum+$i+1;?></td>
<td><a href="/bbsal.php?start=<?php echo $startnum;?>&count=<?php echo $count;?>&action=detail&id=<?php echo $row[0];?>&order=<?php echo $order;?>&desc=<?php echo $desc;?>"><?php echo htmlspecialchars($row[2]);?></a></td>
<td><?php echo htmlspecialchars($row[3]);?></td>
<td><?php echo htmlspecialchars($row[15]);?></td>
<td><pre><?php echo $row[14];?></pre></td>
<td>
<a onclick="return confirm('你真的要删除这条记录吗?')" href="/bbsal.php?start=<?php if($i==0 && $startnum > 0) echo ($startnum-1); else echo $startnum;?>&count=<?php echo $count;?>&action=del&id=<?php echo $row[0];?>&order=<?php echo $order;?>&desc=<?php echo $desc;?>">删除</a>
<a href="/bbsal.php?start=<?php echo $startnum;?>&count=<?php echo $count;?>&action=edit&id=<?php echo $row[0];?>&order=<?php echo $order;?>&desc=<?php echo $desc;?>">编辑</a>
</td>
</tr>
<?php
				$i++;
			}
			mysql_free_result($result);
		}
?>
</table>
<center>
<hr class=default>
<a href="javascript:location=location">刷新</a>
<?php
		if( $startnum > 0 ){
?>
<a href="/bbsal.php?start=<?php if($startnum - $count > 0) echo ($startnum-$count); else echo "0";?>&count=<?php echo $count;?>&order=<?php echo $order;?>&desc=<?php echo $desc;?>">上一页</a>
<?php	}else{
?>
上一页
<?php
		}

		if( $i >= $count ){
?>
<a href="/bbsal.php?start=<?php echo ($startnum+$count-1);?>&count=<?php echo $count;?>&order=<?php echo $order;?>&desc=<?php echo $desc;?>">下一页</a>
<?php	}else{
?>
下一页
<?php
		}
?>
<a href="/bbsal.php?start=<?php echo $startnum;?>&count=<?php echo $count;?>&order=<?php echo $order;?>&desc=<?php echo $desc;?>&action=add">增加条目</a>

<script language="javascript">
<!--//
function doOrder(){
	var oSelectType=document.getElementById("oOrder");
	var type=oSelectType.value;

	if(type=="name")
		window.location="/bbsal.php?start=<?php echo ($startnum);?>&count=<?php echo $count;?>&order=name&desc=<?php echo $desc;?>";
	else if(type=="bbsid")
		window.location="/bbsal.php?start=<?php echo ($startnum);?>&count=<?php echo $count;?>&order=bbsid&desc=<?php echo $desc;?>";
	else
		window.location="/bbsal.php?start=<?php echo ($startnum);?>&count=<?php echo $count;?>&order=groupname&desc=<?php echo $desc;?>";

	return;
}
//-->
</script>

&nbsp;&nbsp;&nbsp;按<select name="type" class="input"  style="WIDTH: 55px" id="oOrder" onChange="doOrder();">
<option value="name"<?php if( $order=="name" ) { ?> selected="selected"<?php } ?>>姓名</option>
<option value="bbsid"<?php if( $order=="bbsid" ) { ?> selected="selected"<?php } ?>>bbsid</option>
<option value="groupname"<?php if( $order=="groupname" ) { ?> selected="selected"<?php } ?>>分组</option>
</select>

<script language="javascript">
<!--//
function doRefresh(){
	var oSelectType=document.getElementById("oType");
	var type=oSelectType.value;

	if(type=="1")
		window.location="/bbsal.php?start=<?php echo ($startnum);?>&count=<?php echo $count;?>&order=<?php echo $order;?>&desc=0";
	else
		window.location="/bbsal.php?start=<?php echo ($startnum);?>&count=<?php echo $count;?>&order=<?php echo $order;?>&desc=1";

	return;
}
//-->
</script>

<select name="type" class="input"  style="WIDTH: 48px" id="oType" onChange="doRefresh();">
<option value="1"<?php if( $desc==0 ) { ?> selected="selected"<?php } ?>>增序</option>
<option value="2"<?php if( $desc==1 ) { ?> selected="selected"<?php } ?>>倒序</option>
</select>排列

<script language="javascript">
<!--//
function doCount(){
	var oSelectType=document.getElementById("oCount");
	var type=oSelectType.value;

	if(type=="10")
		window.location="/bbsal.php?start=<?php echo ($startnum);?>&count=10&order=<?php echo $order;?>&desc=<?php echo $desc;?>";
	else if(type == "50")
		window.location="/bbsal.php?start=<?php echo ($startnum);?>&count=50&order=<?php echo $order;?>&desc=<?php echo $desc;?>";
	else if(type == "30")
		window.location="/bbsal.php?start=<?php echo ($startnum);?>&count=30&order=<?php echo $order;?>&desc=<?php echo $desc;?>";
	else
		window.location="/bbsal.php?start=<?php echo ($startnum);?>&count=20&order=<?php echo $order;?>&desc=<?php echo $desc;?>";

	return;
}
//-->
</script>

每页显示<select name="type" class="input"  style="WIDTH: 40px" id="oCount" onChange="doCount();">
<option value="10"<?php if( $count==10 ) { ?> selected="selected"<?php } ?>>10</option>
<option value="20"<?php if( $count==20 ) { ?> selected="selected"<?php } ?>>20</option>
<option value="30"<?php if( $count==30 ) { ?> selected="selected"<?php } ?>>30</option>
<option value="50"<?php if( $count==50 ) { ?> selected="selected"<?php } ?>>50</option>
</select>个

</center>
</body>
<?php
	}
?>
