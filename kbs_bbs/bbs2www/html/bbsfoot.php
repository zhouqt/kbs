<?php
    $setboard=0;
    require("funcs.php");
    if ($loginok != 1)
        html_nologin();
    else {
        html_init("gb2312","","<meta http-equiv=\"Refresh\" content=\"240; url=/bbsfoot.php\">",1);

		if (isset($_GET["total"]))
			$oldtotal = $_GET["total"];
		else
			$oldtotal = 0;
		settype($oldtotal,"integer");

		if (isset($_GET["unread"]))
			$oldunread = $_GET["unread"];
		else
			$oldunread = 0;
		settype($oldunread,"integer");
?>
<script language="JavaScript">
<!--
function Init() {
  servertime=new Date()
  servertime.setTime(<?php echo time(); ?>*1000)
  staytime=<?php echo (time()-$currentuinfo["logintime"])/60; ?>

  localtime=new Date()
  Time()
}
function Time(){
 var now=new Date()
 var Timer=new Date()
 Timer.setTime(servertime.getTime()+now.getTime()-localtime.getTime());
 var hours=Timer.getHours()
 var minutes=Timer.getMinutes()
 if (hours==0)
 hours=12
 if (minutes<=9)
 minutes="0"+minutes
 var year=Timer.getYear();
 if (year < 1900)   
	 year = year + 1900; 
 myclock=year+"年"+(Timer.getMonth()+1)+"月"+Timer.getDate()+"日"+hours+":"+minutes
 var staysec=(now.getTime()-localtime.getTime())/60000+staytime;
 stayclock=parseInt(staysec/60)+"小时"+parseInt(staysec%60)+"分钟"
 document.clock.myclock.value=myclock
 document.clock.stay.value=stayclock
 setTimeout("Time()",58000)
}
//JavaScript End-->
</script>
<body class="b2"  onload="Init()">
<form name="clock">时间[<input class="b8" TYPE="text" NAME="myclock" size="18">] 在线[<a href="/cgi-bin/bbs/bbsusr" target="f3" class="b8"><?php 
echo bbs_getonlinenumber(); ?></a>] 帐号[<a href=<?php
echo "\"/bbsqry.php?userid=" . $currentuser["userid"] . "\""; ?> target="f3" class="b8"><?php
echo $currentuser["userid"]; ?></a>] <?php
		if (strcmp($currentuser["userid"], "guest") != 0)
		{
		    if (bbs_getmailnum($currentuser["userid"],$total,$unread, $oldtotal, $oldunread)) {
			  if ($unread!=0) {
		        echo "信箱[<a href=\"bbsreadmail.php\" target=\"f3\" class=\"b8\">" . $total . "封(新信" . $unread . ")</a>] ";
			  }
			  else {
		        echo "信箱[<a href=\"bbsreadmail.php\" target=\"f3\" class=\"b8\">" . $total . "封</a>] ";
			  }
			}
			else
		      echo "信箱[<a href=\"bbsreadmail.php\" target=\"f3\" class=\"b8\">0封</a>] ";
		}
?>
停留[<input class="b8" TYPE="text" NAME="stay" size="10">]
</form>
<?php
        html_normal_quit();
    }
?>
