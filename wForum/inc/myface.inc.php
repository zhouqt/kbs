<?php
	/* 自定义头像相关函数 */

	/*
	 * 获得文件系统上的文件全路径，这里假设 PHP 页面都是在 wForum 根目录，以后改成更好的办法吧。
	 */
	$topdir = dirname($_SERVER['SCRIPT_FILENAME']) . "/";
	function get_myface_fs_filename($filename) {
		global $topdir;
		return $topdir.$filename;
	}
	
	function get_myface_dir($userid) {
		return MYFACEDIR . strtoupper(substr($userid,0,1));
	}
	
	/*
	 * 这里用随机数为的是浏览器能立刻刷新上传的头像。
	 * 这里还用到了当前用户名为的是确认修改的时候能够把上传的无用文件都删掉，缺点是新用户注册的时候就不能上传头像了 - atppp
	 */
	function get_myface_filename($userid, $ext) {
		mt_srand();
		$d = get_myface_dir($userid);
		$fd = get_myface_fs_filename($d);
		if (is_file($fd)) die("这不可能，这不可能！");
		if (!is_dir($fd)) @mkdir($fd);
		return $d."/".$userid.".".mt_rand(0,10000).$ext;
	}
	
	function get_myface($user, $extra_tag = false) {
		if ($user['userface_img'] == -1) {
			$user_pic = htmlEncode($user['userface_url']);
			$has_size = true;
		} else {
			$user_pic = 'userface/image'.$user['userface_img'].'.gif';
			$has_size = false;
		}
		$str = "<img src=\"".$user_pic."\"";
		if ($has_size) $str .= " width=\"".$user['userface_width']."\" height=\"".$user['userface_height']."\"";
		if ($extra_tag !== false) $str .= " ".$extra_tag;
		return $str."/>";
	}
?>