<?php
require('pcfuncs.php');

if (!$pcconfig["USERFILES"]) //不支持个人空间
    header('Location: /');
if (!defined('_USER_FILE_ROOT_'))
    header('Location: /');

function bbs_userfile_getfiledir($userid) {
    return _USER_FILE_ROOT_ . '/'. strtoupper($userid[0]) .'/'. $userid;
}

function bbs_userfile_getfilepath($userid, $filepath) {
    return bbs_userfile_getfiledir($userid) . '/' . $filepath;
}
    
function bbs_userfile_getrootfid($uid) {
    $query = 'SELECT fid FROM userfiles WHERE uid = '.intval($uid).' AND type = 1 AND pid = 0 LIMIT 1;';
    $result = mysql_query($query);
    if ($rows = mysql_fetch_array($result))
    {
        mysql_free_result($result);
        return $rows[fid];
    }
    if (!UserFile::add_file('',1,'','',0,'',0,$uid))
        return false;
    return bbs_userfile_getrootfid($uid);
}

$file_access = array('公开','好友','私有');

class UserFile {
var $fid;
var $uid;
var $pid;
var $filename;
var $filepath;
var $filesize;
var $filetype;
var $type;
var $hostname;
var $filetime;
var $access;
var $remark;
var $err;

function UserFile ($fid, $uid=0) {
    $fid = intval($fid);
    $uid = intval($uid);
    if ($uid)
        $query = 'SELECT * FROM userfiles WHERE fid ='.$fid.' AND uid = '.$uid.' LIMIT 1;';    
    else
        $query = 'SELECT * FROM userfiles WHERE fid ='.$fid.' LIMIT 1;';   
    $result = mysql_query($query);
    if (!($rows = mysql_fetch_array($result))) {
        $this->err = '文件不存在';
        return false;
    }
    $this->fid = $rows[fid];
    $this->uid = $rows[uid];
    $this->pid = $rows[pid];
    $this->type = $rows[type];
    $this->hostname = $rows[hostname];
    $this->filetime = $rows[filetime];
    $this->access = $rows[access];
    $this->remark = $rows[remark];
    $this->filename = $rows[filename];
    $this->filepath = $rows[filepath];
    $this->filesize = ($this->type==1)?'-':$rows[filesize];
    $this->filetype = ($this->type==1)?'-':$rows[filetype];
    mysql_free_result($result);
    
    return true;
}

function uf_edit($new_filename,$new_access,$new_remark) {
    return $this->edit_file($this->pid,$new_filename,$new_access,$new_remark);
}

function uf_mv($target_fid) {
    if (!($target_file = new UserFile($target_fid , $this->uid))) {
        $this->err = '目标文件夹不存在';
        return false;
    }
    if (!$target_file->fid) {
        $this->err = '目标文件夹不存在';
        return false;
    }
    if ($target_file->type != 1) {
        $this->err = '您只能将这个文件移动到另一个文件夹中';
        return false;
    }
    return $this->edit_file($target_file->fid,$this->filename,$this->access,$this->remark);
}

function uf_readdir(&$sub_files) {
    if ($this->type != 1) {
        $this->err = '这不是一个目录';
        return false;
    }
    $sub_files = $this->check_dir();
    return true;
}

function uf_mkdir($filename, $access, $remark) {
    return $this->add_file($filename,1,'','',$access,$remark,0);
}

function uf_add($filename,$tmp_filepath,$filetype,$access,$remark) {
    return $this->add_file($filename,0,$tmp_filepath,$filetype,$access,$remark,1);   
}

function uf_cp($target_fid) {
    if ($target_fid == $this->pid) {
        $this->err = '您只能将这个文件复制到另一个文件夹中';
        return false;
    }
    if ($this->type == 1) {
        $this->err = '仅有文件才能复制';
        return false;
    }
    if (!($target_file = new UserFile($target_fid , $this->uid))) {
        $this->err = '目标文件夹不存在';
        return false;
    }
    if (!$target_file->fid) {
        $this->err = '目标文件夹不存在';
        return false;
    }
    if ($target_file->type != 1) {
        $this->err = '您只能将这个文件复制到另一个文件夹中';
        return false;
    }
    $ret = $target_file->add_file($this->filename,0,$this->filepath,$this->filetype,$this->access,$this->remark,0);
    $this->err = $target_file->err;
    return $ret;
}

function uf_rm() {
    if ($this->type == 1) {
        if ($this->check_dir()) {
            $this->err = '请先删除该目录下的所有文件';
            return false;
        }
    }
    else
        @unlink($this->filepath);
    $query = 'DELETE FROM userfiles WHERE fid = '.$this->fid.' LIMIT 1;';
    mysql_query($query);
    return true;
}

function gen_filepath() {
    global $currentuser;
    $filepath  = 'F';
    $filepath .= '.'.$currentuser['index'];
    $filepath .= '.'.time();
    $filepath .= '.'.rand(1000,9999);
    return $filepath;
}

/**
 * add_file(string filename, int type , string tmp_filepath , string filetype , int access , string remark, int after_del, int auto_add_uid)
 * 添加文件/目录
 * filename: 文件/目录名
 * type    : 0:文件 / 1: 目录
 * tmp_filepath: 临时文件
 * filetype: 文件类型
 * access: 权限 0:公开/1:好友/2:私有
 × remark: 备注
 * upload_file: 是否为上传文件 0:不/1:是
 * auto_add_uid : 是否为自动添加 0:不/1:是
 */
function add_file($filename ,$type ,$tmp_filepath ,$filetype ,$access ,$remark, $upload_file, $auto_add_uid=0) {
    global $currentuser;
    
    $type = ($type==1)?1:0;
    $filename = trim(ltrim($filename));
    if ($auto_add_uid==0) {
        if (!$filename) {
            $this->err = ($type==1)?'请输入目录名':'请输入文件名';
            return false;
        }
    }
    if  ($type == 0) {
        if ($upload_file)  { //上传文件
             if (!is_uploaded_file($tmp_filepath)) {  
                 $this->err = '文件上传错误';
                 return false;
             }    
        }
        else {    
            if (!file_exists($tmp_filepath)) {
                $this->err = '源文件 '.$tmp_filepath.' 不存在';
                return false;
            }
        }
        $filedir = bbs_userfile_getfiledir($currentuser['userid']);
        if (!file_exists($filedir)) {
            if (!mkdir($filedir)) {
                $this->err = '创建文件目录失败';
                return false;
            }
        }
        $filepath = bbs_userfile_getfilepath($currentuser['userid'], $this->gen_filepath());
        if ($upload_file) {
            if (!move_uploaded_file($tmp_filepath,$filepath)) {
                $this->err = '移动文件失败';
                return false;
            }
        }
        else {
            if (!copy($tmp_filepath, $filepath)) {
                $this->err = '拷贝文件失败';
                return false;
            }
        }
        
        $filetype = trim(ltrim($filetype));
        $filesize = filesize($filepath);
    }
    else {
        $filepath = $filetype = '';
        $filesize = 0; 
    }
    
    if ($access != 1 && $access != 0)
        $access = 2;
        
    $remark = trim($remark);
    
    $auto_add_uid = intval($auto_add_uid);
    if ($auto_add_uid!=0)
    {
        $uid = $auto_add_uid;
        $pid = 0; 
    }
    else
    {
        $uid = $this->uid;
        $pid = $this->fid;
    }
    
    $query = 'INSERT INTO `userfiles` ( `fid` , `uid` , `pid` , `type` , `filename` , `filepath` , `hostname` , `filetime` , `filesize` , `filetype` , `access` , `remark` ) '.
             'VALUES ( \'\', \''.$uid.'\', \''.$pid.'\', \''.$type.'\', \''.addslashes($filename).'\', \''.addslashes($filepath).'\', \''.addslashes($_SERVER['REMOTE_ADDR']).'\', NOW( ) , \''.intval($filesize).'\', \''.addslashes($filetype).'\', \''.$access.'\', \''.addslashes($remark).'\' );';
    if (!mysql_query($query)) {
        if  ($type == 0 )
            @unlink($filepath);
        $this->err = '添加文件信息失败';
        return false;
    }
    if ($auto_add_uid==0)
        $this->err = '';
    return true;
}

/**
 * edit_file(int new_pid , string new_filename , int new_access , string new_remark)
 */
function edit_file($new_pid,$new_filename,$new_access,$new_remark) {
    if ($this->pid != $new_pid) {
        if (!($p_file = new UserFile($new_pid , $this->uid))) {
            $this->err = '目标文件夹不存在';
            return false;
        }
        if (!$p_file->fid) {
            $this->err = '目标文件夹不存在';
            return false;
        }
        if ($p_file->type != 1) {
            $this->err = '目标文件夹不存在';
            return false;
        }
    }
    $new_filename = trim(ltrim($new_filename));
    if (!$new_filename) {
        $this->err = '新文件名不能为空';
        return false;
    }
    if ($new_access != 0 && $new_access != 1)
        $new_access = 2;
    
    $query = 'UPDATE userfiles SET '.
             '  pid = '.$new_pid.' ,'.
             '  filename = \''.addslashes($new_filename).'\' ,'.
             '  access   =   '.$new_access.' ,'.
             '  remark = \''.addslashes($new_remark).'\' '.
             'WHERE fid = '.$this->fid.' LIMIT 1';
     mysql_query($query);
     return true;
}

function check_dir() {
    $query = 'SELECT `fid` FROM userfiles WHERE pid = '.$this->fid.' ORDER BY type DESC , filename ASC;';
    $result = mysql_query($query);
    $sub_files = array();
    while($rows = mysql_fetch_array($result))
        $sub_files[] = $rows[fid];
    if (mysql_num_rows($result))
        return $sub_files;
    else
        return false;    
}

    
}


?>