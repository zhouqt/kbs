<?php
/**
 * WWW favorite code, written by PHP script, only for test
 * @author: windinsn@smth.org
 * 
 * +------------------------------------------------------+
 * | KBS BBS Version 1.2                                  |
 * +------------------------------------------------------+
 * | Copyright (c) KBS BBS                                |
 * | http://dev.kcn.cn                                    |
 * +------------------------------------------------------+
 *
 * class Favorite
 *
 * usage:
 * $foo = new Favorite ($userid);
 *
 * Properties:
 *   $foo->userId;
 *   $foo->userInfo;
 *   $foo->file;
 *   $foo->err;
 *   $foo->allFav;
 *   $foo->allFavCnt;
 *   $foo->info;
 *   $foo->fav;
 *   $foo->favCnt;
 *   $foo->currentPid;
 *   $foo->maxOrder;
 *   $foo->allDirs;
 *
 * Methods:
 *   $foo->load ($pid);
 *   $foo->add ($pid, $type, $order, $title, $url);
 *   $foo->del ($id);
 *   $foo->move ($id, $newOrder);
 *   $foo->is_dir ($id);
 *   $foo->id_exist ($id);
 *   $foo->load_all_dirs ();
 *
 */

if (!defined ('FAVORITE_ROOT_DIR'))
	define ('FAVORITE_ROOT_DIR', BBS_HOME . '/home');
if (!defined ('FAVORITE_FILE_NAME'))
	define ('FAVORITE_FILE_NAME', 'WWWFAVORITE');
if (!defined ('FAVORITE_MAX_ITEM_LENGTH'))
	define ('FAVORITE_MAX_ITEM_LENGTH', 1024);
if (!defined ('FAVORITE_MAX_ITEM_NUMBER'))
	define ('FAVORITE_MAX_ITEM_NUMBER', 1024);
if (!defined ('FAVORITE_DEFAULT_FILE'))
	define ('FAVORITE_DEFAULT_FILE', BBS_HOME . '/etc/WWWFAVRITEDEFAULT');
if (!defined ('FAVORITE_NAME'))
	define ('FAVORITE_NAME', 'ÎÒµÄ°Ù±¦Ïä');

class Favorite {
	/**
	 *
	 * user id
	 *
	 * @var    string
	 * @access public
	 *
	 */
	var $userId;
	
	/**
	 *
	 * user information array, get by function bbs_getuser ();
	 *
	 * @var    array
	 * @access public
	 *
	 */
	var $userInfo;
	
	/**
	 *
	 * the file which we store information
	 *
	 * @var    string
	 * @access public
	 *
	 */
	var $file;
	
	/**
	 *
	 * error message
	 *
	 * @var    string
	 * @access public
	 *
	 */
	var $err;
	
	/**
	 *
	 * all the favorite items
	 *
	 * @var    array
	 *         index is the item ID;
	 *         ['ID']: int, the item ID;
	 *         ['TYPE']: int, whether it is a directory
	 *         ['PID']:int, ID of the parent's directory
	 *         ['ORDER']:int, order in the directory
	 *         ['TITLE']:string, favorite subject
	 *         ['URL']:string, favorite url
	 * @access public
	 *
	 */
	var $allFav;
	
	/**
	 *
	 * number of all favorite items
	 *
	 * @var    int
	 * @access public
	 *
	 */
	var $allFavCnt;
	
	/**
	 *
	 * current item information
	 * must use $foo->load ($pid) function before, where $pid is current directory ID
	 *
	 * @var    array
	 *         have the same structure with $foo->allFav item
	 * @access public
	 *
	 */
	var $info;
	
	/**
	 *
	 * favorite items in current directory
	 * must use $foo->load ($pid) function before, where $pid is current directory ID
	 *
	 * @var    array
	 *         have the same structure with $foo->allFav
	 * @access public
	 *
	 */
	var $fav;
	
	/**
	 *
	 * number of favorite items in current directory
	 * must use $foo->load ($pid) function before, where $pid is current directory ID
	 *
	 * @var    int
	 * @access public
	 *
	 */
	var $favCnt;
	
	/**
	 *
	 * current directory ID
	 * must use $foo->load ($pid) function before, where $pid is current directory ID
	 *
	 * @var    int
	 * @access public
	 *
	 */
	var $currentPid;
	
	/**
	 *
	 * max order number in current directory
	 * must use $foo->load ($pid) function before, where $pid is current directory ID
	 *
	 * @var    int
	 * @access public
	 *
	 */
	var $maxOrder;
	
	/**
	 *
	 * all the directories
	 * must use $foo->load_all_dirs () function before
	 *
	 * @var    array
	 *         have the same structure with $foo->allFav
	 * @access public
	 */
	var $allDirs;
	
	/**
	 *
	 * construct function, initialize all information
	 * usage: $foo = class Favorite (string userId);
	 *
	 * @parm    string $userId: whose favorite information
	 * @return  true on success
	 *          false if some error occur
	 * @access  public
	 *
	 */
	function Favorite ($userId) {
		$this->userInfo = array ();
		if (!bbs_getuser ($userId, $this->userInfo)) {
			$this->err = sprintf ('User %s is NOT exist!', $userId);    
			return false;
		}
		$this->userId = $this->userInfo['userid'];
		$this->file = $this->_get_file ();
		return $this->_init ();
	}
	
	/**
	 *
	 * load favorite items in a given directory
	 * usage: $foo->load ($pid);
	 *
	 * @parm    int $pid: the directory ID
	 * @return  true on success
	 *          false if some error occur
	 * @access  public
	 *
	 */
	function load ($pid) {
		if (!$this->id_exist ($pid)) {
			$this->err = sprintf ('Favorite folder ID %d is NOT exist!', $pid);
			return false;    
		}
		$this->fav = array ();
		$this->maxOrder = 0;
		reset ($this->allFav);
		while (list ($id, $val)=each ($this->allFav)) {
			if ($val['PID']==$pid) {
				$this->fav[] = $val;
				if ($val['ORDER'] > $this->maxOrder)
					$this->maxOrder = $val['ORDER'];
			}    
		}
		usort ($this->fav, 'favorite_order_cmp');
		$this->favCnt = sizeof ($this->fav);
		$this->currentPid = $pid;
		if ($this->currentPid)
			$this->info = $this->allFav[$this->currentPid];
		return true;
	}
	
	/**
	 *
	 * load all the directories to variable $foo->allDirs
	 * usage: $foo->load_all_dirs ()
	 *
	 * @return true on success
	 *         false if some error occur
	 * @access public
	 */
	function load_all_dirs () {
		$this->allDirs = array ();
		reset ($this->allFav);
		while (list ($id, $val)=each ($this->allFav)) {
			if ($this->is_dir ($id))
				$this->allDirs[$id]=$val;
		}
		return true;
	}
	
	/**
	 * 
	 * add an item
	 * usage: $foo->add ($pid, $type, $order, $title, $url);
	 *
	 * @parm    int $pid: parent directory ID
	 * @parm    int $type: item type ($type=1 mean it is a directory, $type=0 it is common)
	 * @parm    int $order: item order ID in the directory
	 * @parm    string $title: item title
	 * @parm    string $url: item url
	 * @return  true on success
	 *          false if some error occur
	 * @access  public
	 *
	 */
	function add ($pid, $type, $order, $title, $url) {
		$title = trim ($title);
		$url = trim ($url);
		if (!$title) {
			$this->err ='Title can\'t be blank!';
			return false;    
		}
		if (!$type && !$url) {
			$this->err ='Url can\'t be blank!';
			return false;    
		}
		
		if ($this->allFavCnt + 1 > FAVORITE_MAX_ITEM_NUMBER) {
			$this->err = 'You have added maximum items!';    
			return false;
		}
		if (strlen ($title . $url) > FAVORITE_MAX_ITEM_LENGTH) {
			$this->err = 'Title or Url is too long!';    
			return false;
		}
		
		if (!$this->load ($pid))
			return false;
		
		$type = ($type==1)?1:0;
		$order = intval ($order);
		$id = $this->_gen_new_id ();
		$this->favCnt ++;
		$this->maxOrder ++;
		$this->allFav[$id] = array ('ID'=>$id, 'ORDER'=>$this->maxOrder, 'PID'=>$pid,'TYPE'=>$type,  'TITLE'=>$title, 'URL'=>$url);
		$this->fav[] = $this->allFav[$id];
		$this->_move ($this->allFav[$id], $order);
		return $this->_save ();
	}
	
	/**
	 *
	 * del an item
	 * usage: $foo->del ($id)
	 *
	 * @parm    int $id: item ID
	 * @return  true on success
	 *          false if some error occur
	 * @access  public
	 *
	 */
	function del ($id) {
		if (!$this->id_exist ($id)) {
			$this->err = 'Target item is NOT exist!';
			return false;    
		}
		if (!$this->_del_item ($this->allFav[$id]))
			return false;
		return $this->_save ();
	}
	
	/**
	 *
	 * whether the given item is directory
	 * usage: $foo->is_dir ($id);
	 *
	 * @parm    int $id: item ID
	 * @return  true if it is a directory
	 *          false if it is a common item
	 * @access  public
	 *
	 */
	function is_dir ($id) {
		return ($id==0)?true:($this->allFav[$id]['TYPE'] != 0);    
	}
	
	/**
	 *
	 * whether the given ID exist
	 * usage: $foo->id_exist ($id)
	 *
	 * @parm    int $id: query ID
	 * @return  true if exists
	 *          false doesn't exist
	 * @access  public
	 *
	 */
	function id_exist ($id) {
		return ($id==0)?true:array_key_exists ($id, $this->allFav);
	}
	
	/**
	 *
	 * change item order in a directory
	 * usage: $foo->move ($id, $newOrder)
	 *
	 * @parm    int $id: item ID
	 * @Parm    int $newOrder: item new order number
	 * @return  true on success
	 *          false if some error occur
	 * @access  public
	 * 
	 */
	function move ($id, $newOrder) {
		if (!($itemArr = $this->allFav[$id])) {
			$this->err = 'Item is NOT exist!';
			return false;        
		}
		if (!$this->load ($itemArr['PID']))
			return false;
		if (!$this->_move ($itemArr, $newOrder))
		   return false;
		return $this->_save ();
	}
	
	/**
	 *
	 * change item order in a directory
	 * usage: $this->move ($itemArr, $newOrder)
	 *
	 * @parm    array $itemArr: item array, which has the same structure with $this->allFav item
	 * @Parm    int $newOrder: item new order number
	 * @return  true on success
	 *          false if some error occur
	 * @access  private
	 * 
	 */
	function _move ($itemArr, $newOrder) {
		if ($itemArr['PID'] != $this->currentPid)
			return false;
		if ($newOrder < 1 || $newOrder > $this->maxOrder)
			$newOrder = $this->maxOrder;
		if ($itemArr['ORDER']==$newOrder)
			return true;
		$forward = ($itemArr['ORDER'] > $newOrder)?true:false;
		for ($i=0; $i < $this->favCnt; $i ++) {
			if ($this->fav[$i]['ORDER'] < $itemArr['ORDER'] && $this->fav[$i]['ORDER'] < $newOrder)
				continue;
			if ($this->fav[$i]['ORDER'] > $itemArr['ORDER'] && $this->fav[$i]['ORDER'] > $newOrder)
				break;
			$forward?($this->allFav[$this->fav[$i]['ID']]['ORDER'] ++):($this->allFav[$this->fav[$i]['ID']]['ORDER'] --);
		}
		$this->allFav[$itemArr['ID']]['ORDER'] = $newOrder;  
		return $this->load ($itemArr['PID']);
	}
	
	/**
	 *
	 * initialize $foo->allFav array, load all favorite items
	 *
	 * @return  ture on success
	 *          false if some error occur
	 * @access  private
	 *
	 */
	function _init () {
		if (!($fh = $this->_open ('r')))
			return false;
		$this->allFav = array ();
		while (!feof ($fh)) {
			$line = trim (ltrim (fgets ($fh)));
			if (!$line) continue;
			list ($id, $pid, $type, $order, $title, $url) = explode (':', $line);
			$id = intval ($id);
			if (!$id) continue;
			$this->allFav[$id] = array ('ID'=>$id, 'ORDER'=>intval ($order), 'PID'=>intval ($pid),'TYPE'=>intval ($type),  'TITLE'=>rawurldecode ($title), 'URL'=>rawurldecode ($url));
		}
		$this->_close ($fh);
		$this->allFavCnt = sizeof ($this->allFav);
		return true;
	}
	
	/**
	 *
	 * del directory
	 * usage: $this->_del_dir ($itemArr)
	 *
	 * @parm    array $itemArr, has the same structure with $this->allFav item
	 * @return  ture on success
	 *          false if some error occur
	 * @access  private
	 *
	 */
	function _del_dir ($itemArr) {
		if (!$this->load ($itemArr['ID']))
			return false;
		while ($this->favCnt) {
			if (!$this->_del_item ($this->fav[0]))
				return false;
			$this->load ($itemArr['ID']);
		}
		unset ($this->allFav[$itemArr['ID']]);
		if (!$this->load ($itemArr['PID']))
			return false;
		for ($i = 0; $i < $this->favCnt; $i ++) {
			if ($this->fav[$i]['ORDER'] > $itemArr['ORDER'])
				$this->allFav[$this->fav[$i]['ID']]['ORDER']--;    
		}
		return true;
	}
	
	/**
	 *
	 * del an item
	 * usage: $this->_del_dir ($itemArr)
	 *
	 * @parm    array $itemArr, has the same structure with $this->allFav item
	 * @return  ture on success
	 *          false if some error occur
	 * @access  private
	 *
	 */
	function _del_item ($itemArr) {
		if ($this->is_dir ($itemArr['ID'])) {
			if (!$this->_del_dir ($itemArr))
				return false;
		}
		else {
			unset ($this->allFav[$itemArr['ID']]);
		}
		if (!$this->load ($itemArr['PID']))
			return false;
		for ($i = 0; $i < $this->favCnt; $i ++) {
			if ($this->fav[$i]['ORDER'] > $itemArr['ORDER'])
				$this->allFav[$this->fav[$i]['ID']]['ORDER']--;    
		}
		return true;    
	}
	
	/**
	 *
	 * save current favorite information to file
	 * usage: $this->_save ()
	 *
	 * @return  ture on success
	 *          false if some error occur
	 * @access  private
	 *
	 */
	function _save () {
		if (!($fh = $this->_open ('w')))
			return false;
		
		reset ($this->allFav);
		while (list ($id, $val)=each ($this->allFav)) {
			$buf = sprintf ("%d:%d:%d:%d:%s:%s\n",$id,$val['PID'],$val['TYPE'],$val['ORDER'],rawurlencode ($val['TITLE']), rawurlencode ($val['URL']));    
			fputs ($fh, $buf, strlen ($buf));
		}
		$this->_close ($fh);
		return true;
	}
	
	/**
	 *
	 * generate a new item ID
	 * usage: $this->_gen_new_id ()
	 *
	 * @return  int: new ID
	 * @access  private
	 *
	 */
	function _gen_new_id () {
		$ids = array ();    
		reset ($this->allFav);
		while (list ($id, $val)=each ($this->allFav)) {
			$ids[$id]=1;
		}
		$id = 1;
		while (isset ($ids[$id]))
			$id ++;
		return $id;    
	}
	
	/**
	 *
	 * the file path where we store information
	 * usage: $this->_get_file ()
	 *
	 * @return  string: the file path
	 * @access  private
	 *
	 */
	function _get_file () {
		return FAVORITE_ROOT_DIR . '/' . strtoupper ($this->userId[0]) . '/' . $this->userId . '/' . FAVORITE_FILE_NAME ;
	}
	
	/**
	 *
	 * open file
	 * usage: $this->_open ($mode)
	 *
	 * @parm    string mode
	 * @return  the file handle on success
	 *          false if some error occur
	 * @access  private
	 *
	 */
	function _open ($mode) {
		if (!file_exists ($this->file)) {
			if (file_exists(FAVORITE_DEFAULT_FILE)) {
				if (!copy (FAVORITE_DEFAULT_FILE, $this->file))
					exit ('System Error!');
			} else {
				touch($this->file);
			}
		}
		
		if (!($fh=fopen ($this->file, $mode))) {
			$this->err = sprintf ('Can\'t open favorite file!');
			return false;
		}
		
		if (!flock ($fh, (strcmp ($mode, 'r')==0 || strcmp ($mode, 'rb')==0)?LOCK_SH:LOCK_EX)) {
			$this->err = sprintf ('Can\'t lock favorite file!');
			fclose ($fh);
			return false;
		}
		
		return $fh;
	}
	
	/**
	 *
	 * close file
	 * usage: $this->_close ($fh)
	 *
	 * @parm    resource $fh: file handle
	 * @access  private
	 *
	 */
	function _close ($fh) {
		flock ($fh, LOCK_UN);
		fclose ($fh);    
	}
	
	
}

function favorite_order_cmp ($array1, $array2) {
	if ($array1['ORDER'] == $array2['ORDER'])
		return 0;    
	return ($array1['ORDER']>$array2['ORDER'])?1:-1;
}

?>