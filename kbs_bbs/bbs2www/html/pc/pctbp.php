<?php
	/*
	** @id:windinsn dec 18,2003
	*/
	function pc_tbp_check_url($url)
	{
		$arr = array();
		if(!eregi("^http://([a-zA-Z0-9_\.\-]{1,}):{0,1}([0-9]{0,})\/([a-zA-Z0-9_\.\-\/]{0,})\?{0,1}(.+){0,}$",$url,$arr))
			return FALSE;
		$url = array(
				"URL" => $arr[0],
				"SERVER" => $arr[1],
				"PORT" => $arr[2],
				"FILE" => $arr[3],
				"VARS" => $arr[4]
				);
		return $url;
	}
	
	function pc_tbp_connect($url)
	{
		$host = gethostbyname($url[SERVER]);
		$port = $url[PORT]?$url[PORT]:80;
		$timeout = 30;
		
		$fp = fsockopen($host,$port,&$err_num,&$err,$timeout);
		if(!$fp)
			return FALSE;
		else
		{
			set_socket_blocking($fp,FALSE);
			return $fp;
		}
	}
	
	function pc_tbp_close($fp)
	{
		fclose($fp);	
	}
	
	function pc_tbp_send_cmd($fp,$cmd)
	{
		fputs($fp,$cmd);	
	}
	
	function pc_tbp_get_infor($fp)
	{
		fgets($fp,4096);
	}
	
	function pc_tbp_trackback_ping($url,$tbarr)
	{
		$url = pc_tbp_check_url($url);
		if(!$url)
			return -1;
		else
		{
			$fp = pc_tbp_connect($url);
			if(!$fp)
				return -2;
			else
			{
				$cmd = "POST ".$url[URL]."\n".
					"Content-Type: application/x-www-form-urlencoded\n".
					"title=".urlencode($tbarr[title])."&url=".urlencode($tbarr[url])."&excerpt=".urlencode(substr($tbarr[excerpt],0,255))."&blog_name=".urlencode($tbarr[blogname]);
				
				pc_tbp_send_cmd($fp,$cmd);
				$infor = pc_tbp_get_infor($fp);
				pc_tbp_close($fp);
				return $infor;
			}
		}
	}
	
?>