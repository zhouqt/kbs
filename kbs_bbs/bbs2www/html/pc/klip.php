<?php
	/**
	** @id:windinsn dec 20,2003
	** enable KlipFolio Channel
	**/
	require("pcfuncs.php");
	
	$userid = addslashes($_GET["id"]);
	$uid = (int)($_GET["id"]);
	
	$link = pc_db_connect();
	$pc = pc_load_infor($link,$userid,$uid);
	pc_db_close($link);
	if(!$pc)
	{
		html_init("gb2312");
		html_error_quit("所请求的Blog不存在!");
		exit();
	}
	
	if( pc_cache( $pc["MODIFY"] ) )
		return;
	
	header("Content-Type: text/Klip");
	header("Content-Disposition: inline;filename=".$pc["USER"].".Klip");
	
?>
<!--  COPYRIGHT AND TRADEMARK NOTICE

The Klip File Format is copyright 2001-2003, Serence Inc.
Klip and associated marks are trademarks of Serence Inc.
All rights reserved.

http://www.serence.com/site.php?action=ser_legal,legal_tmip

Please note that information you enter here will be available to the public

------------------------------------------------------------

This Klip File is Used For SMTH.Blog System , Script Writen by windinsn@smth.org

-->
<klip>
	<owner>
		<author><?php echo $pc["USER"]; ?></author>
		<copyright>Copyright <?php echo $pc["USER"]; ?>.bbs@<?php echo $pcconfig["SITE"]; ?></copyright>
		<email><?php echo $pc["USER"]; ?>.bbs@<?php echo $pcconfig["SITE"]; ?></email>
		<web>http://<?php echo $pcconfig["SITE"]; ?></web>
	</owner>
	<identity>
		<title><?php echo $pc["NAME"]; ?></title>
		<uniqueid><?php echo $pc["USER"]; ?>.blog at <?php echo $pcconfig["SITE"]; ?></uniqueid>
		<version>1.0</version>
		<lastmodified><?php echo time_format($pc["MODIFY"]); ?></lastmodified>
		<description><?php echo $pc["DESC"]; ?></description>
		<keywords><?php echo $pc["THEM"]; ?></keywords>
	</identity>
		
	<locations>
		<defaultlink><?php echo pc_personal_domainname($pc["USER"]); ?></defaultlink>
		<contentsource>http://<?php echo $pcconfig["SITE"]; ?>/pc/rss.php?userid=<?php echo $pc["USER"]; ?></contentsource>
		<icon></icon>
		<banner></banner>
		<help>http://<?php echo $pcconfig["SITE"]; ?>/pc/help.html#NewsreaderKlipfolio</help>
		<kliplocation>http://<?php echo $pcconfig["SITE"]; ?>/pc/klip.php?id=<?php echo $pc["USER"]; ?></kliplocation>	
	</locations>
		
	<setup>
		<refresh>120</refresh>
		<referer></referer>
		<country>CN</country>
		<language>ZH</language>
		<codepage>936</codepage>
	</setup>
	
	<messages>
		<loading>Getting data...</loading>
		<nodata>No items to display.</nodata>
	</messages>
</klip>