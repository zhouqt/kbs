<?php
/**
 * This source file is part of SMTH BBS PHP Library Project.
 * Copyright (c) 2002-2003 SMTH BBS
 * 
 * This file contains PHP library functions for operating boards,
 * such as checking read-only board, checking anonymouse, etc.
 * 
 * $Id$
 */
if (!defined('_BBS_BOARDS_PHP_'))
{
define('_BBS_BOARDS_PHP_', 1);

require("funcs.php");

/**
 * Checking whether a board is set with some specific flags or not.
 * 
 * @param $board the board object to be checked
 * @param $flag the flags to check
 * @return TRUE  the board is set with the flags
 *         FALSE the board is not set with the flags
 * @author flyriver
 */
function bbs_check_board_flag($board,$flag)
{
	if ($board["FLAG"] & $flag)
		return TRUE;
	else
		return FALSE;
}

/**
 * Checking whether a board is an anonymous board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is an anonymous board
 *         FALSE the board is not an anonymous board
 * @author flyriver
 */
function bbs_is_anony_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["ANONY"]);
}

/**
 * Checking whether a board is an outgo board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is an outgo board
 *         FALSE the board is not an outgo board
 * @author flyriver
 */
function bbs_is_outgo_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["OUTGO"]);
}

/**
 * Checking whether a board is a junk board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is a junk board
 *         FALSE the board is not a junk board
 * @author flyriver
 */
function bbs_is_junk_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["JUNK"]);
}

/**
 * Checking whether a board is an attachment board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is an attachment board
 *         FALSE the board is not an attachment board
 * @author flyriver
 */
function bbs_is_attach_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["ATTACH"]);
}

/**
 * Checking whether a board is a readonly board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is a readnoly board
 *         FALSE the board is not a readonly board
 * @author flyriver
 */
function bbs_is_readonly_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["READONLY"]);
}

function bbs_is_noreply_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["NOREPLY"]);
}

//if (defined('BBS_NEWPOSTSTAT')) {
    
if (defined('BBS_STAT_HOT')) {
include ('db.php');  // include the database class
if (!($db = new BbsDb)) {
    html_error_quit($db->err);    
}
/**
 * get hot threads of a borad
 * bbs_get_hot_threads(string board)
 * @author: windinsn
 */
function bbs_get_hot_threads($board,$num,&$threads,&$err) 
{
    global $db;
    $brdarr = array();
    $bid = bbs_getboard($board,$brdarr);
    if (!$bid) {
        $err = '°æÃæ '.$board.' ²»´æÔÚ';
        return false;
    }
    $board = $brdarr['NAME'];
    $now = date('YmdHis');
    $sql = 'SELECT threadid,userid,title,time AS created,MAX(time) AS changed,count(DISTINCT userid) AS count FROM postlog WHERE YEAR(time)=YEAR('.$now.') AND MONTH(time)=MONTH('.$now.') AND DAYOFMONTH(time)=DAYOFMONTH('.$now.') AND bname = \''.addslashes($board).'\' GROUP BY threadid ORDER BY count DESC , id DESC LIMIT 0 , '.intval($num).';';
    if (!$db->query($sql,1)) {
        $err = $db->err;
        return false;    
    }
    $threads = array();
    for ($i = 0 ; $i < $db->nums ; $i ++ ) {
        $threads[] = array(
                'gid' => $db->arrays[$i]['threadid'],
                'userid' => $db->arrays[$i]['userid'],
                'created' => $db->arrays[$i]['created'],
                'changed' => $db->arrays[$i]['changed'],
                'count'  => $db->arrays[$i]['count'],
                'title' => $db->arrays[$i]['title']
            );    
    }
    return true;
}
} // defined('BBS_STAT_HOT')
//} // defined('BBS_NEWPOSTSTAT')
} // !define ('_BBS_BOARDS_PHP_')
?>
