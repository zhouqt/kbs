<?php
    require("userfile.php");
    
    pc_check_referer();
    if(isset($_GET['fid']))
        $fid = intval($_GET['fid']);
    else
        html_error_quit("错误的参数");
    
    $link = pc_db_connect();
    if (!($f = new UserFile($fid)))
        html_error_quit("文件不存在");
    
    if (!$f->fid || $f->type != 0)
        html_error_quit("文件不存在");
    
    if ($pcconfig["USERFILEPERM"])
        if ($f->access != 0) {
            $pc = pc_load_infor($link,'',$f->uid);    
            if (!$pc)
                html_error_quit("文件不存在");
            if ($f->access == 2) {
                if (!pc_is_admin($currentuser,$pc))
                    html_error_quit("文件不存在");
            }
            if ($f->access == 1) {
               if  (!pc_is_friend($currentuser["userid"],$pc["USER"]))
                    html_error_quit("文件不存在");
            }
        }
        
    pc_db_close($link);
	
    if (!$pcconfig["USERFILEPERM"] || $f->access == 0)
        if (cache_header("public",$f->filetime,600))
            return;
        
    if (!($file = fopen($f->filepath, "rb")))
        html_error_quit("文件不存在");
	
	Header("Content-type: " . $f->filetype);
	Header("Accept-Ranges: bytes");
	Header("Content-Length: " . filesize($f->filepath));
	Header("Content-Disposition: inline;filename=" . $f->filename);
	echo fread($file,filesize($f->filepath));
	fclose($file);
	exit;
?>