<?php
    require("funcs.php");

    html_init("gb2312");

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

	if( $yesterd )
		$pngurl = "/bbsstat/todayonline.png";
	else
		$pngurl = "/bbsstat/".$year."/".$month."/".$day."_useronline.png";

	html_init("gb2312");
?>
<center><?php echo BBS_FULL_NAME; ?> -- 用户在线统计 <hr color=green>
<form action="/bbsonline.php" method="get">

<select name="year" class="input" style="WIDTH: 55px">
<option value="2003"<?php if( $year=="2003" ) { ?> selected="selected"<?php } ?>>2003</option>
<option value="2004"<?php if( $year=="2004" ) { ?> selected="selected"<?php } ?>>2004</option>
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
</form>
<?php
	if( $yesterd == 0 ){
?>
<a href="/bbsonline.php">最新数据</a>
<?php
	}
?>
<br>
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

