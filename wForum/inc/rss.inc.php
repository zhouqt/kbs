<?php

function generate_rss($channel, $items) {
	global $HTMLCharset;
	$str = "<?xml version=\"1.0\" encoding=\"$HTMLCharset\" ?>\n<rss version=\"2.0\">\n\t<channel>\n";
	foreach ($channel as $key => $value) {
		$str .= "\t\t<$key>$value</$key>\n";
	}
	foreach ($items as $item) {
		$str .= "\t\t<item>\n";
		foreach ($item as $key => $value) {
			$str .= "\t\t\t<$key>$value</$key>\n";
		}
		$str .= "\t\t</item>\n";
	}
	$str .= "\t</channel>\n</rss>\n";
	return $str;
}

?>