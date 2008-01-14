<?php
	require_once("pcfuncs.php");
	
	function pc_rss_init()
	{
	echo '<?xml version="1.0" encoding="gb2312"?>';
?>
<rdf:RDF 
	xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
	xmlns:dc="http://purl.org/dc/elements/1.1/" 
	xmlns:sy="http://purl.org/rss/1.0/modules/syndication/"
	xmlns:co="http://purl.org/rss/1.0/modules/company/"
	xmlns:ti="http://purl.org/rss/1.0/modules/textinput/"
	xmlns="http://purl.org/rss/1.0/"
>
<?php	
	}
	
	function pc_rss_end()
	{
?>
</rdf:RDF>
<?php
	}
	
	function pc_rss_channel($rss, $rssetems)
	{
?>
	<channel rdf:about="<?php echo $rss[channel][siteaddr]; ?>">
		<title><?php echo $rss[channel][title]; ?></title>
		<link><?php echo $rss[channel][pcaddr]; ?></link>
		<description><?php echo $rss[channel][desc]; ?></description>
		<dc:language>zh-cn</dc:language> 
<?php
		if($rss[channel][publisher])
		{
?>
		<dc:publisher><?php echo $rss[channel][publisher]; ?></dc:publisher> 
<?php
		}
		if($rss[channel][email])
		{
?>
		<dc:creator><?php echo $rss[channel][email]; ?></dc:creator> 
<?php
		}
		if($rss[channel][rights])
		{
?>
		<dc:rights>Copyright <?php echo $rss[channel][rights]; ?></dc:rights> 
<?php
		}
		if($rss[channel][date])
		{
?>
		<dc:date><?php echo $rss[channel][date]; ?></dc:date>
<?php
		}
	 	if($rss[channel][updatePeriod])
	 	{
?>
		<sy:updatePeriod><?php echo $rss[channel][updatePeriod]; ?></sy:updatePeriod>
<?php
		}
		if($rss[channel][updateFrequency])
		{
?>
		<sy:updateFrequency><?php echo $rss[channel][updateFrequency]; ?></sy:updateFrequency>
<?php
		}
		if($rss[channel][updateBase])
		{
?>
		<sy:updateBase><?php echo $rss[channel][updateBase]; ?></sy:updateBase> 
<?php
		}
		if($rss[channel][logoimg])
		{
?>
		<image rdf:resource="<?php echo $rss[channel][logoimg]; ?>" />
<?php
		}
?>
		<items>
			<rdf:Seq>
<?php
		foreach($rssetems as $etem) {
?>
			<rdf:li resource="<?php echo $etem[addr]; ?>" /> 
<?php
        }
?>
			</rdf:Seq>
		</items>
	</channel>
<?php
	}
	
	function pc_rss_etem($etem)
	{
		$etembody = $etem[etemdesc]."\n<br><hr size=1>\n".
				"(<a href=\"".$etem[addr]."\">阅读全文</a>)\n".
				"(<a href=\"".$etem[etemcomaddr]."\">发表评论</a>)";
?>
	<item rdf:about="<?php echo $etem[addr]; ?>">
		<title><?php echo $etem[etemtitle]; ?></title> 
		<link><?php echo $etem[addr]; ?></link>
		<dc:creator><?php echo $etem[etemauth]; ?></dc:creator> 
		<dc:date><?php echo $etem[etemtime]; ?></dc:date> 
		<description>
			<![CDATA[
				<?php echo $etembody; ?>
			]]> 
		</description>
	</item>
<?php
	}

	function pc_rss_user_etem($useretem)
	{
?>	
		<item rdf:about="<?php echo $useretem[addr]; ?>">
		<title><?php echo $useretem[subject]; ?></title> 
		<link><?php echo $useretem[addr]; ?></link> 
		<description>
		<![CDATA[
			<?php echo $useretem[desc]; ?>
		]]> 
		</description>
		<dc:publisher><?php echo $useretem[publisher]; ?></dc:publisher> 
		<dc:creator><?php echo $useretem[creator]; ?></dc:creator> 
		<dc:rights><?php echo $useretem[rights]; ?></dc:rights> 
		<dc:subject><?php echo $useretem[subject]; ?></dc:subject> 
		</item>	
<?php		
	}
	
	function pc_rss2_init($rss)
	{
	echo '<?xml version="1.0" encoding="gb2312" ?>';
?>
<rss version="2.0">
	<channel>
		<title><?php echo $rss[channel][title]; ?></title>
		<link><?php echo $rss[channel][pcaddr]; ?></link>
		<description><?php echo $rss[channel][desc]; ?></description>
		<language>zh-cn</language>
		<managingEditor><?php echo $rss[channel][email]; ?></managingEditor>
<?php
	}
	
	function pc_rss2_etem($etem)
	{
		$etembody = $etem[etemdesc];
?>
		<item>
			<title><?php echo $etem[etemtitle]; ?></title> 
			<link><?php echo $etem[addr]; ?></link>
			<author><?php echo $etem[etemauth]; ?></author>
			<guid><?php echo $etem[addr]; ?></guid>
			<pubDate><?php echo $etem[gmttime]; ?></pubDate>
			<description>
				<![CDATA[
					<?php echo $etembody; ?>
				]]> 
			</description>			
		</item>
<?php
	}
	
	function pc_rss2_end()
	{
?>
	</channel>
</rss>
<?php
	}
	
	function pc_rss_user($rss)
	{
		pc_rss_init();
		pc_rss_channel($rss, $rss[useretems]);
		foreach($rss[useretems] as $useretem)
			pc_rss_user_etem($useretem);
		pc_rss_end();
	}
	
	function pc_rss_output($rss)
	{
		pc_rss_init();
		pc_rss_channel($rss, $rss[etems]);
		foreach($rss[etems] as $etem)
			pc_rss_etem($etem);
		pc_rss_end();
	}
	
	function pc_rss2_output($rss)
	{
		pc_rss2_init($rss);
		foreach($rss[etems] as $etem)
			pc_rss2_etem($etem);
		pc_rss2_end();
	}
?>
