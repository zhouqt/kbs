<?php
if (!defined('_BBS_WWW2_BMP_PHP_'))
{
define('_BBS_WWW2_BMP_PHP_', 1);

/* F**K PHP5 STUPID PEOPLE! */
function my_basename($filename) {
	$pos1 = strrpos($filename, "/");
	$pos2 = strrpos($filename, "\\");
	if ($pos1 === false && $pos2 === false) return $filename;
	if ($pos1 === false) $pos1 = $pos2;
	else $pos1 = ($pos1 > $pos2) ? $pos1 : $pos2;
	return substr($filename, $pos1 + 1);
}

function compress_bmp(&$ofile, &$oname)
{
	if (defined("AUTO_BMP2PNG_THRESHOLD")) {
		$oname = my_basename(addslashes($oname));
		if (strcasecmp(".bmp", substr($oname, -4)) == 0 && (filesize($ofile) > AUTO_BMP2PNG_THRESHOLD)) {
			$h = @popen("identify -format \"%m\" ".escapeshellarg($ofile), "r");
			if ($h) {
				$read = fread($h, 1024);
				pclose($h);
				if (strncasecmp("BMP", $read, 3) == 0) {
					$tp = $ofile.".BMP2PNG";
					@exec("convert -quality 75 ".escapeshellarg($ofile)." png:".escapeshellarg($tp));
					if (file_exists($tp)) {
						unlink($ofile);
						$ofile = $tp;
						$oname = substr($oname, 0, -4) . ".png";
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

} // !define ('_BBS_WWW2_BMP_PHP_')
?>
