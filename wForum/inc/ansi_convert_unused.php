<?php
//funcs.php 里面似乎是没用的函数，暂时移出来 - atppp

function ansi_getfontcode($fgcolor,$bgcolor,$defaultfg,$defaultbg,$highlight,$blink,$underlink, &$head,&$tail)
{
	$modify="";
	if ($fgcolor==-1) 
	  $modify=sprintf(" color=%s",$defaultfg);
	else
	if ($fgcolor==-2)
	  $modify=sprintf(" color=%s",$defaultbg);
	else
	if ($highlight)
	   $fgcolor+=8;
	if ($fgcolor<0) $fgcolor=0;
	if ($bgcolor==-1)
	  $modify .= sprintf(" style='background-color:%s'",$defaultbg);
	else
	if ($bgcolor==-2)
	  $modify .= sprintf(" style='background-color:%s'",$defaultfg);
	if ($bgcolor<0) $bgcolor=0;
	$head = sprintf("<font class=f%d%02d%s>",$bgcolor,$fgcolor,$modify);
	if ($underlink) {
	   $head .= "<u>";
	   $tail = "</u>";
	}
	$tail .= "</font>";
}

function ansi_convert( $buf , $defaultfg, $defaultbg)
{
	$keyword = preg_split("/\x1b\[([^a-zA-Z]*)([a-zA-Z])/",$buf,-1,PREG_SPLIT_DELIM_CAPTURE);
	$fgcolor=-1;
	$bgcolor=-1;
	$blink=false;
	$underlink=false;
	$highlight=false;
	for ($i=1;$i<count($keyword);$i+=3) {
		if ($keyword[$i+2]=="")
			continue;
		if ($keyword[$i+1]=='m') {
			$head="";
			$tail="";
			if ($keyword[$i]=="") {
					// *[;m
				$fgcolor=-1;
				$bgcolor=-1;
				$blink=false;
				$underlink=false;
				$highlight=false;
			} else {
				$good=true;
				$colorcodes=split(';',$keyword[$i]);
				foreach ( $colorcodes as $code ) {
					if (preg_match("/[\D]/",$code)) {
						$good=false;
						break;
					}
					if ($code=="") 
						$value=0;
					else
						$value=intval($code);
					if ($value<=8 && $value>=1) {
						switch ($value) {
						case 0:
							$fgcolor=-1;
							$bgcolor=-1;
							$blink=false;
							$underlink=false;
							break;
						case 1:
							$highlight=1;
							break;
						case 4:
							$underlink=1;
							break;
						case 5:
							$blink=1;
						case 7:
							$savebg=$bgcolor;
							if ($fgcolor==-1)
								$bgcolor=-2;
							else
								$bgcolor=$fgcolor;
							if ($bgcolor==-1)
								$fgcolor=-2;
							else
								$fgcolor=$savebg;
						}
					} else
					if ($value<=37 && $value>=30)
						$fgcolor=$value-30;
					else
					if ($value<=47 && $value>=40)
						$bgcolor=$value-40;
					else {
						// unsupport code
						$good=false;
						break;
					}
				}
				if ($good)
					ansi_getfontcode($fgcolor,$bgcolor,$defaultfg,$defaultbg,$highlight,$blink,$underlink, $head,$tail);
			}
			$final .= $head . htmlspecialchars($keyword[$i+2]) . $tail;
		} else $final .= htmlspecialchars($keyword[$i+2]);
	}
	return $final;
}

?>