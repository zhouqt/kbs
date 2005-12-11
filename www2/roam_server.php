<?php
define ('SMTH_ROAM_FILE'  , '/home/bbs/0Announce/roam');
define ('SMTH_ROAM_FROM'  , '166.111.8.238');
define ('SMTH_ROAM_PORT'  , 1080);

function bbs_roam_query ($query_string,$query_keys,$type,&$data,&$err) {
    switch ($query_string) {
        case 'queryuser':
            $query_string = '/service/queryuser.php';
            $encoded = 'userid='.rawurlencode ($query_keys[0]);
            break;
        case 'queryboard':
            $query_string = '/service/queryboard.php';
            $encoded = 'board='.rawurlencode ($query_keys[0]);
            break;
        case 'checkreadperm';
            $query_string = '/service/checkreadperm.php';
            $encoded= 'usernum='.intval ($query_keys[0]).'&bid='.intval ($query_keys[1]);
            break;
        case 'normalboard':
            $query_string = '/service/normalboard.php';
            $encoded = 'board='.rawurlencode ($query_keys[0]);
            break;
        case 'ann_traverse_check':
            $query_string = '/service/ann_traverse_check.php';
            $encoded = 'filename='.rawurlencode ($query_keys[0]).'&userid='.rawurlencode ($query_keys[1]);
            break;
        default:
            return -1;    
    }
    
    $query_file = 'http://'.SMTH_ROAM_FROM.':'.SMTH_ROAM_PORT.$query_string;
    
    $url = parse_url($query_file);
    if (!$url) return -1;
	if (!isset($url['port'])) { $url['port'] = ''; }
	if (!isset($url['query'])) { $url['query'] = ''; }

    $fp = fsockopen($url['host'], $url['port'] ? $url['port'] : 80 , $errno , $errstr , 30);
    if (!$fp) 
        return -2;
    
    fputs($fp, sprintf("POST %s%s%s HTTP/1.0\n", $url['path'], $url['query'] ? "?" : "", $url['query']));
	fputs($fp, "Host: $url[host]\n");
	fputs($fp, "Content-type: application/x-www-form-urlencoded\n");
	fputs($fp, "Content-length: " . strlen($encoded) . "\n");
	fputs($fp, "Connection: close\n\n");
	
	fputs($fp, "$encoded\n");
	
	$line = fgets($fp,1024);
	if (!eregi("^HTTP/1\.. 200", $line)) {
	    return -3;
	}
	
	$data_rcv = '';
	$inheader = 1;
	while(!feof($fp)) {
		$line = fgets($fp,1024);
		if ($inheader && ($line == "\n" || $line == "\r\n")) 
			$inheader = 0;
		elseif (!$inheader) 
			$data_rcv .= $line;
	}
	fclose($fp);
    
    if (!$data_rcv)
        return -4;
        
    $parser = xml_parser_create();
    xml_parser_set_option($parser,XML_OPTION_CASE_FOLDING,0);
    xml_parser_set_option($parser,XML_OPTION_SKIP_WHITE,1);
    xml_parse_into_struct($parser,$data_rcv,$values,$tags);
    xml_parser_free($parser);
    
    if (strcasecmp($values [1]['tag'],'error')==0) 
        $err = $values [1]['value'];
    else
        return -5;
    
    if ($err==1) { //err
        if (strcasecmp($values [2]['tag'],'message')==0) 
            $err = rawurldecode ($values [2]['value']);
        else
            $err = 'Unknown Error';
        return 0;
    }
    
    if ($type==1) {
        $data = array ();
        $i = 2;
        $err = 1;
        while ($err==1) {
             if (strcasecmp ($values [$i]['tag'],'data') == 0 && strcasecmp ($values [$i]['type'],'open') == 0)
                $err = 0;
             if (!$values [$i])
                break;
             $i ++;
        }
        if ($err==1) {
            $err = 'Can\'t Get Data!';
            return 0;
        }
        
        $err = 0;
        while (!(strcasecmp ($values [$i]['tag'],'data') == 0 && strcasecmp ($values [$i]['type'],'close') == 0)) {
            $data[$values [$i]['tag']] = rawurldecode ($values [$i]['value']);
            $i ++;
            if (!$values [$i]) {
                $err = 1;
                break;
            }
        }
        
        if ($err==1) {
            $err = 'Received Data Error!';
            return 0;
        }
    }
    else {
        if (strcasecmp($values [2]['tag'],'message')==0) 
            $err = rawurldecode ($values [2]['value']);
        else
            $err = 'No Message';
    }
    
    return 1;
}

function bbs_roam_readinfor($userid) {
    if (strstr($userid, '.'))
        return 0;
    $file = SMTH_ROAM_FILE . '/' . strtoupper ($userid[0]) . '/' . $userid;
    $lines = file ($file);
    if (!$lines) return 0;
    $ret = array();
    foreach ($lines as $line) {
        $line = ltrim(trim($line));
        $exp = explode (':',$line);
        $k = ltrim (trim ($exp[0]));
        $v = ltrim (trim ($exp[1]));
        if (!$k || !$v)
            return 0;
        $ret[$k]=$v;
    }
    return $ret;
}

function bbs_roam_init_guest () {
    global $currentuser;
    $currentuser = array (
                    'userid' => 'guest'
                        );
    $_SESSION['currentuser'] = $currentuser;
}

session_start();
session_name();

if (!session_is_registered('currentuser')) {
    bbs_roam_init_guest ();
}
else {
    $currentuser = $_SESSION['currentuser'];
}

if (isset ($_GET['roam_user'])) {
    if ($currentuser['userid'] != $_GET['roam_user']) {
        if ($_GET['pass_infor']==1) {
            $user_infor = bbs_roam_readinfor($_GET['roam_user']);
            if (!$user_infor)
                bbs_roam_init_guest ();
            else if (!$ret['ROAM_FROM'] || $ret['ROAM_FROM'] != $_SERVER['REMOTE_ADDR']) {
                bbs_roam_init_guest ();
            }
            else {
                $currentuser = $ret;
                $_SESSION['currentuser'] = $currentuser;    
            }
        }
        else {
            bbs_roam_init_guest ();
        }
    }
}

function roam_login_init() {
    return;
}

function bbs_roam_getboard ($board, &$brdarr) {
    $brdarr = array ();
    $board = ltrim (trim ($board));
    if (!$board) return -1;
    $ret =  bbs_roam_query ('queryboard',array ($board),1,$brdarr,$err);
    if ($ret < 0) return -2;
    if ($ret == 0) return 0;
    if (!sizeof ($brdarr)) return -2;
    return $brdarr['BID'];
}

function bbs_roam_checkreadperm($usernum, $bid) {
    $usernum = intval ($usernum);
    $bid = intval ($bid);
    if ($usernum == 0 || $bid == 0)
        return 0;
    $ret = bbs_roam_query('checkreadperm',array ($usernum,$bid),0,$data,$err);
    if ($ret < 0) return -1;
    if ($ret == 1 && strcasecmp ($err,'OK') == 0)
        return 1;
    return 0;
}

function bbs_roam_normalboard($board) {
    $ret = bbs_roam_query('normalboard',array ($board),0,$data,$err);
    if ($ret < 0) return -1;
    if ($ret == 1 && strcasecmp ($err,'OK') == 0) return 1;
    return 0;
}

function bbs_roam_ann_traverse_check($filename,$userid) {
    if (!$userid) return -1;
    $ret = bbs_roam_query('ann_traverse_check',array ($filename,$userid),0,$data,$err);
    if ($ret < 0) return -1;
    if ($ret == 1 && strcasecmp ($err,'OK') == 0) return 1;
    return 0;
}

?>