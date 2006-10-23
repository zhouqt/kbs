<?php
	require("www2-admin.php");

    admin_check("info");

    if(isset($_POST["modifyuserid"])) {
        $userid = $_POST["modifyuserid"];
        $username = $_POST["username"];
        $realname = $_POST["realname"];
        $address = $_POST["address"];
        $email = $_POST["email"];
        if($_POST["gender"] == "M")
            $gender = 77;
        else
            $gender = 70;
        $birthyear = $_POST["birthyear"];
        $birthmonth = $_POST["birthmonth"];
        $birthday = $_POST["birthday"];
        $title = $_POST["title"];
        $realemail = $_POST["realemail"];
        $numlogins = $_POST["numlogins"];
        $numposts = $_POST["numposts"];
        if(@$_POST["firstlogin"] == "yes")
            $firstlogin = 1;
        else
            $firstlogin = 0;
        if(@$_POST["lastlogin"] == "yes")
            $lastlogin = 1;
        else
            $lastlogin = 0;
        $ret = bbs_admin_setuserinfo($userid, $username, $realname, $address, $email, $gender, $birthyear, $birthmonth, $birthday, $title, $realemail, $numlogins, $numposts, $firstlogin, $lastlogin);
        switch($ret) {
        case 0:
            html_success_quit("资料修改成功。", array("<a href=\"adminfo.php?userid={$userid}\">返回资料修改页面</a>"));
            break;
        case 1:
        case 2:
        case 3:
            html_error_quit("生日不正确。");
            break;
        case 4:
            html_error_quit("不存在的用户职务。");
            break;
        default:
        }
    }


    if(isset($_POST["userid"]))
        $userid = $_POST["userid"];
    else if(isset($_GET["userid"]))
        $userid = $_GET["userid"];
    else
        $userid = $currentuser["userid"];
    
    $userinfo = array();
    $uid = bbs_admin_getuserinfo($userid, $userinfo);
    if($uid == -1)
        html_error_quit("不存在的用户。");
    if($uid > 0) {
        admin_header("改别人资料", "修改用户数据");
        
    for($i=1; $i<256; $i++) {
        $usertitles[$i-1] = bbs_admin_getusertitle($i);
    }
        
?>
<form method="post" action="adminfo.php" class="medium">
<fieldset><legend>要修改的用户ID</legend><div class="inputs">
<label>ID:</label><input type="text" name="userid" value="<?php print($userid); ?>" size="12" maxlength="12">
<input type="submit" value="确定"><br>
为了避免重复提交网页表单导致重要数据破坏，用户ID和密码请用telnet方式登录修改。
</div></fieldset></form>
<form method="post" action="adminfo.php" class="medium" onsubmit="return confirm('确实要修改用户资料吗？');">
<fieldset><legend>个人资料</legend><div class="inputs">
<label>帐号:</label><input type="text" name="modifyuserid" value="<?php echo $userinfo["userid"];?>" size="12" readonly><br/>
<label>昵称:</label><input type="text" name="username" value="<?php echo htmlspecialchars($userinfo["username"],ENT_QUOTES);?>" size="24" maxlength="39"><br/>
<label>真实姓名:</label><input type="text" name="realname" value="<?php echo $userinfo["realname"];?>" size="16" maxlength="39"><br/>
<label>居住地址:</label><input type="text" name="address" value="<?php echo $userinfo["address"];?>" size="40" maxlength="79"><br/>
<label>电子信箱:</label><input type="text" name="email" value="<?php echo $userinfo["email"];?>" size="40" maxlength="79"><br/>
<label>性别:</label><input type="radio" name="gender" value='M'<?php echo ($userinfo["gender"]==77)?" checked":""; ?>>男 <input type="radio" name="gender" value="F"<?php echo ($userinfo["gender"]==77)?"":" checked"; ?>>女<br />
<label>生日:</label><input type="text" name="birthyear" value="<?php echo $userinfo["birthyear"]+1900; ?>" size="4" maxlength="4"> 年 <input type="text" name="birthmonth" value="<?php echo $userinfo["birthmonth"]; ?>" size="2" maxlength="2"> 月 <input type="text" name="birthday" value="<?php echo $userinfo["birthday"]; ?>" size="2" maxlength="2"> 日<br/>
<label>当前职务:</label>
<select name="title">
<option value="0">[没有职务]</option>
<?php
        for($i=1; $i<256; $i++) {
            if($usertitles[$i-1] != "")
                print("<option value=\"{$i}\"" . (($userinfo["title"]==$i)?" selected":"") . ">{$usertitles[$i-1]}</option>");
        }
?>
</select>
<br/>
<label>真实Email:</label><input type="text" name="realemail" value="<?php echo $userinfo["realemail"];?>" size="40" maxlength="79"><br/>
<label>上站次数:</label><input type="text" name="numlogins" value="<?php echo $userinfo["numlogins"];?>" size="6" maxlength="7"><br/>
<label>发表大作:</label><input type="text" name="numposts" value="<?php echo $userinfo["numposts"];?>" size="6" maxlength="7"><br/>
<label>注册时间:</label><?php echo date("D M j H:i:s Y",$userinfo["firstlogin"]);?> <input type="checkbox" name="firstlogin" value="yes">提前1分钟<br/>
<label>最近光临:</label><?php echo date("D M j H:i:s Y",$userinfo["lastlogin"]);?> <input type="checkbox" name="lastlogin" value="yes">设为今天<br/>
</div></fieldset>
<div class="oper">
<input type="submit" name="submit" value="确定" /> <input type="reset" value="重置" />
</div>
</form>
<?php
    }
	page_footer();
?>
