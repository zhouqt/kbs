<?php
if (!defined('_BBS_WWW2_BMP_PHP_'))
{
define('_BBS_WWW2_BMP_PHP_', 1);

function compress_bmp(&$ofile, &$oname)
{
	if (defined("AUTO_BMP2PNG_THRESHOLD")) {
		$oname = basename($oname);
		if (strcasecmp(".bmp", substr($oname, -4)) == 0 && (filesize($ofile) > AUTO_BMP2PNG_THRESHOLD)) {
			$h = popen("identify -format \"%m\" ".$ofile, "r");
			if ($h) {
				$read = fread($h, 1024);
				pclose($h);
				if (strncasecmp("BMP", $read, 3) == 0) {
					$tp = tempnam("/tmp", "BMP2PNG");
					exec("convert -quality 75 $ofile png:$tp");
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