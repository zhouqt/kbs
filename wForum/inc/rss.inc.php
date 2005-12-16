<?php

function convert($str, $isUTF8) {
	if ($isUTF8) return (iconv("gb2312", "UTF-8", $str));
	return $str;
}

function generate_rss($channel, $items, $isUTF8 = false) {
	$charset = !$isUTF8 ? "gb2312" : "UTF-8";
	$str = "<?xml version=\"1.0\" encoding=\"$charset\" ?>\n<rss version=\"2.0\">\n\t<channel>\n";
	foreach ($channel as $key => $value) {
		$str .= "\t\t<$key>" . convert($value, $isUTF8) . "</$key>\n";
	}
	foreach ($items as $item) {
		$str .= "\t\t<item>\n";
		foreach ($item as $key => $value) {
			$str .= "\t\t\t<$key>" . convert($value, $isUTF8) . "</$key>\n";
		}
		$str .= "\t\t</item>\n";
	}
	$str .= "\t</channel>\n</rss>\n";
	return $str;
}

?>