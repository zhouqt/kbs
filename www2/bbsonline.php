<?php
    require("www2-funcs.php");

	$nowyear = date("Y");
	$nowmonth = date("m");
	$nowday = date("d");

	$yesterd=0;

	if (isset($_GET["year"])){
	    $year = $_GET["year"];
	}else{
		$year = 0;
		$yesterd=1;
	}
	settype($year, "integer");
	if( $year < 1990 || $year > $nowyear )
		$year = $nowyear;

	if (isset($_GET["month"])){
	    $month = $_GET["month"];
	}else
		$month = 0;
	settype($month, "integer");
	if( $month <= 0 || $month > 12 )
		$month = $nowmonth;
	else{
		if( $month < 10 )
			$month = "0".$month;
	}

	if (isset($_GET["day"])){
	    $day = $_GET["day"];
	}else
		$day = 0;
	settype($day, "integer");
	if( $day <= 0 || $day > 31 )
		$day = $nowday;
	else{
		if( $day < 10 )
			$day = "0".$day;
	}

	if( $yesterd ){
		$pngurl = "/bbsstat/todayonline.png";
		$pngurl1 = "/bbsstat/todaylogin.png";
		$pngurl2 = "/bbsstat/todaylogout.png";
		$pngurl3 = "/bbsstat/todaystay.png";
	} else {
		$pngurl = "/bbsstat/".$year."/".$month."/".$day."_useronline.png";
		$pngurl1 = "/bbsstat/".$year."/".$month."/".$day."_login.png";
		$pngurl2 = "/bbsstat/".$year."/".$month."/".$day."_logout.png";
		$pngurl3 = "/bbsstat/".$year."/".$month."/".$day."_stay.png";
	}

	page_header("用户在线统计");
?>
<form action="bbsonline.php" method="get">

<select name="year" class="input" style="WIDTH: 55px">
<?php
	for ($i = 2002; $i <= $nowyear; $i++) {
?>
<option value="<?php echo $i; ?>"<?php if( $year==$i ) { ?> selected="selected"<?php } ?>><?php echo $i; ?></option>
<?php
	}
?>
</select>年

<select name="month" class="input" style="WIDTH: 40px">
<option value="01"<?php if( $month=="01" ) { ?> selected="selected"<?php } ?>>01</option>
<option value="02"<?php if( $month=="02" ) { ?> selected="selected"<?php } ?>>02</option>
<option value="03"<?php if( $month=="03" ) { ?> selected="selected"<?php } ?>>03</option>
<option value="04"<?php if( $month=="04" ) { ?> selected="selected"<?php } ?>>04</option>
<option value="05"<?php if( $month=="05" ) { ?> selected="selected"<?php } ?>>05</option>
<option value="06"<?php if( $month=="06" ) { ?> selected="selected"<?php } ?>>06</option>
<option value="07"<?php if( $month=="07" ) { ?> selected="selected"<?php } ?>>07</option>
<option value="08"<?php if( $month=="08" ) { ?> selected="selected"<?php } ?>>08</option>
<option value="09"<?php if( $month=="09" ) { ?> selected="selected"<?php } ?>>09</option>
<option value="10"<?php if( $month=="10" ) { ?> selected="selected"<?php } ?>>10</option>
<option value="11"<?php if( $month=="11" ) { ?> selected="selected"<?php } ?>>11</option>
<option value="12"<?php if( $month=="12" ) { ?> selected="selected"<?php } ?>>12</option>
</select>月

<select name="day" class="input" style="WIDTH: 40px">
<?php
	for($i=1; $i<=31; $i++){
		$nd = $i;
		if($i < 10 )
			$nd = "0".$i;
?>
<option value="<?php echo $nd;?>"<?php if( $day==$nd ) { ?> selected="selected"<?php } ?>><?php echo $nd?></option>
<?php
	}
?>
</select>日
<input type=submit name="submit" value="Go">
数据开始于 2002年10月9日
<a href="/bbsstat/allonline.png">历史图表</a>
</form>
<?php
	if( $yesterd == 0 ){
?>
<a href="/bbsonline.php">最新数据</a>
<?php
	}
?>

<br>
在线人数统计<br>
<?php
	if( file_exists( $_SERVER["DOCUMENT_ROOT"].$pngurl ) ){
?>
<img src="<?php echo $pngurl;?>"></img>
<?php
	}else{
?>
对不起，暂时无此日统计图表
<?php
	}
?>

<br>
用户登录统计(y轴为6分钟总登陆人次):<br>
<?php
	if( file_exists( $_SERVER["DOCUMENT_ROOT"].$pngurl1 ) ){
?>
<img src="<?php echo $pngurl1;?>"></img>
<?php
	}else{
?>
对不起，暂时无此日统计图表
<?php
	}
?>

<br>
用户退出统计(y轴同上):<br>
<?php
	if( file_exists( $_SERVER["DOCUMENT_ROOT"].$pngurl2 ) ){
?>
<img src="<?php echo $pngurl2;?>"></img>
<?php
	}else{
?>
对不起，暂时无此日统计图表
<?php
	}
?>

<br>
用户平均停留时间统计(y轴以10s为单位):<br>
<?php
	if( file_exists( $_SERVER["DOCUMENT_ROOT"].$pngurl3 ) ){
?>
<img src="<?php echo $pngurl3;?>"></img>
<?php
	}else{
?>
对不起，暂时无此日统计图表
<?php
	}
?>

<?php
	page_footer();
?>

