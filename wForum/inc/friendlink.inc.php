<?php
	function gen_friend_links($infile, $outfile) {
		if (($fout = @fopen($outfile, "w")) === false) return false;
		if (($fin = fopen($infile, "r")) === false) {
			fclose($fout);
			return false;
		}
		$total = 0;
		while (!feof($fin)) {
			$ll = trim(fgets($fin, 4096));
			if (strlen($ll) == 0 || $ll{0} == "#") continue;
			$urltitle = preg_split("/\s[\s]+/", $ll);
			if (count($urltitle) == 2) {
				if ($total == 0) {
					fwrite($fout, "<table cellspacing=1 cellpadding=3 align=center class=TableBorder1>\n");
					fwrite($fout, "<tr><th align=left id=TableTitleLink height=25 style=\"font-weight:normal\" colspan=\"5\"><b>-=&gt; ”—«È¡¥Ω”</b></th></tr>\n");
				}
				$total++;
				if ($total % 5 == 1) {
					fwrite($fout, "<tr>");
				}
				fwrite($fout, "<td class=TableBody1 height=25 width=\"20%%\" align=\"center\"><a target=\"_blank\" href=\"".$urltitle[1]."\">".$urltitle[0]."</a></td>\n");
				if ($total % 5 == 0) {
					fwrite($fout, "</tr>");
				}
			}
		}
		if ($total % 5 != 0) {
			fwrite($fout, "<td class=TableBody1 colspan=\"".(5-$total%5)."\">&nbsp;</td></tr>");
		}
		if ($total > 0) {
			fwrite($fout, "</table><br>");
		}
		fclose($fin);
		fclose($fout);
		return true;
	}
?>