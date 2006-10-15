<?php
	/*
	** @id:windinsn dec 18,2003
	*/
	function pc_tbp_check_url($url)
	{
		$arr = array();
		if(!eregi("^http://([a-zA-Z0-9_\.\-]{1,}):{0,1}([0-9]{0,})\/([a-zA-Z0-9_\.\-\/]{0,})\?{0,1}(.+){0,}$",trim(ltrim($url)),$arr))
			return FALSE;
		$url = array(
				"URL" => $arr[0],
				"HOST" => $arr[1],
				"PORT" => $arr[2],
				"FILE" => $arr[3],
				"VARS" => $arr[4]
				);
		return $url;
	}
	
	/*
	** use php fsockopen function to do a http post action
	** return 0 : success;
	**        -1: url wrong;
	**        -2: can not connect to host
	**        -3: page is not exist
	*/
	function pc_tbp_http_post($url, $data)
	{
		$url = parse_url($url);
		if (!$url) return -1;
		if (!isset($url['port'])) { $url['port'] = ""; }
		if (!isset($url['query'])) { $url['query'] = ""; }
		
		$encoded = "";
		
		while (list($k,$v) = each($data)) 
		{
			$encoded .= ($encoded ? "&" : "");
			$encoded .= rawurlencode($k)."=".rawurlencode($v);
		}
		
		$fp = fsockopen($url['host'], $url['port'] ? $url['port'] : 80 , $errno , $errstr , 30);
		if (!$fp) return -2;
		
		fputs($fp, sprintf("POST %s%s%s HTTP/1.0\n", $url['path'], $url['query'] ? "?" : "", $url['query']));
		fputs($fp, "Host: $url[host]\n");
		fputs($fp, "Content-type: application/x-www-form-urlencoded\n");
		fputs($fp, "Content-length: " . strlen($encoded) . "\n");
		fputs($fp, "Connection: close\n\n");
		
		fputs($fp, "$encoded\n");
		
		$line = fgets($fp,1024);
		if (!eregi("^HTTP/1\.. 200", $line)) return -3;
		
		$results = ""; $inheader = 1;
		while(!feof($fp)) 
		{
			$line = fgets($fp,1024);
			if ($inheader && ($line == "\n" || $line == "\r\n")) 
			{
				$inheader = 0;
			}
			elseif (!$inheader) 
			{
				$results .= $line;
			}
		}
		fclose($fp);
		
		return 0;
	}
	
	/*
	** return 0 : success;
	**        -1: url wrong;
	**        -2: can not connect to host
	**        -3: page is not exist
	*/
	function pc_tbp_trackback_ping($url,$tbarr)
	{
		$url = pc_tbp_check_url($url);
		if(!$url)
			return -1;
		$data = array(
			"title" => $tbarr[title],
			"url" => $tbarr[url],
			"excerpt" => substr($tbarr[excerpt],0,255),
			"blog_name" => $tbarr[blogname]
			);
		$r = pc_tbp_http_post($url[URL],$data);
		return $r;
	}
?>