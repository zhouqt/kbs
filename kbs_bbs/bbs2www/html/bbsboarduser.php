<?php
define('KEYWORD_THU','清华');
class BoardUser
{
var $link;
var $sql;
var $res;
var $resnum;
var $board;
var $infor;
var $bid;
var $brdarr;
var $err;
var $users;
var $usernum;
var $total;
var $telnetguest;
var $wwwguest;
var $cernet;
var $thu;
var $table;

function BoardUser($board) {
    $db['HOST']=bbs_sysconf_str('MYSQLHOST');
    $db['USER']=bbs_sysconf_str('MYSQLUSER');
    $db['PASS']=bbs_sysconf_str('MYSQLPASSWORD');
    $db['NAME']=bbs_sysconf_str('MYSQLSMSDATABASE');
    $this->link = mysql_connect($db['HOST'],$db['USER'],$db['PASS']) or exit ('无法连接到服务器!');
    mysql_select_db($db['NAME'],$this->link);
    $this->brdarr = array();
    $this->bid = bbs_getboard($board,$this->brdarr);
    if ($this->bid == 0) {
        $this->err = '讨论区不存在';
        return false;
    }
    $this->board = $this->brdarr['NAME'];
    $this->table = 'board_'.addslashes($this->board).'_tmp';
}

function reset_db() {
    $this->sql = 'DROP TABLE IF EXISTS `'.$this->table.'`;';
    $this->query();
    return true;
}

function query($res=false,$debug=0) {
    $result = mysql_query($this->sql,$this->link);
    if (!$result) {
        if ($debug)
            return false;
        else
            exit (mysql_error().'<br/'.$this->sql);
    }
    if ($res) {
        $this->res = array();
        while ($rows = mysql_fetch_array($result))
            $this->res[] = $rows;
        $this->resnum = mysql_num_rows($result);
        mysql_free_result($result);
    }
    return true;
}

function init_db() {
    $this->sql = 'CREATE TABLE `'.$this->table.'` ( `userid` VARCHAR( 12 ) NOT NULL ,'.
               ' `hostname` INT( 12 ) NOT NULL ,'.
               ' `location` VARCHAR( 50 ) NOT NULL ,'.
               ' `infor` VARCHAR( 100 ) NOT NULL ,'.
               ' `cernet` INT( 1 ) UNSIGNED NOT NULL );';
    $this->query();
}

function get_users() {
    $this->users = array();
    $this->usernum = bbs_useronboard($this->board,$this->users);
    if ($this->usernum < 0) {
        $this->err = '系统错误';
        return false;
    }
    return true;
}

function read_sql($sql) {
    $this->sql = $sql;
    return $this->make_array();
}

function analyse() {
    $this->total = $this->telnetguest = $this->wwwguest = $this->thu = 0;
    for ($i = 0 ; $i < $this->usernum ; $i ++) {
        if (!strcmp($this->users[$i]['USERID'],'guest'))
            $this->telnetguest ++;
        if (!strcmp($this->users[$i]['USERID'],'_wwwguest'))
            $this->wwwguest ++;
        $this->total ++;
        $ip = ip2long($this->users[$i]['HOST']);
        $this->sql = 'SELECT * FROM address WHERE ips <= '.$ip.' AND ipe >= '.$ip.' ORDER BY (ipe - ips) ASC LIMIT 1;';
        $this->query(1);
        if ($this->resnum > 0) {
            $location = $this->res[0]['location'];
            $infor = $this->res[0]['infor'];
            $cernet = $this->res[0]['cernet'];
        }
        else {
            $location = $infor = '';
            $cernet = 0;
        }
        if ($cernet) $this->cernet ++ ;
        if (stristr($location.$infor,KEYWORD_THU)) $this->thu ++;
        $this->users[$i]['LOCATION'] = $location;
        $this->users[$i]['INFOR'] = $infor;
        $this->users[$i]['CERNET'] = intval($cernet);
        $this->sql = 'INSERT INTO `'.$this->table.'` VALUES(\''.addslashes($this->users[$i]['USERID']).'\','.$ip.',\''.addslashes($location).'\',\''.addslashes($infor).'\','.intval($cernet).'); ';
        $this->query();
    }
    return true;
}

function read_res($order=0) {
    switch ($order) {
        case 1: //hostname,userid,cernet
            $this->sql = 'SELECT * FROM `'.$this->table.'` ORDER BY hostname , userid, cernet DESC;';
            break;
        case 2: //hostname,cernet,userid
            $this->sql = 'SELECT * FROM `'.$this->table.'` ORDER BY hostname , cernet DESC, userid;';
            break;
        case 3: //cernet, userid , hostname
            $this->sql = 'SELECT * FROM `'.$this->table.'` ORDER BY cernet DESC,userid,hostname;';
            break;
        case 4: //cernet,hostname, userid
            $this->sql = 'SELECT * FROM `'.$this->table.'` ORDER BY cernet DESC,hostname,userid;';
            break;
        default:
           $this->sql = 'SELECT * FROM `'.$this->table.'` ORDER BY userid;';
    }
    return $this->make_array();
}


function make_array() {
    if (!$this->query(1,1))
        return false;
    $this->usernum = $this->resnum;
    $this->total = $this->telnetguest = $this->wwwguest = $this->thu = 0;
    $this->users = array();
    for ($i = 0 ; $i < $this->resnum ; $i ++) {
        $this->users[] = array(
                            'USERID' => $this->res[$i]['userid'],
                            'HOST'   => long2ip($this->res[$i]['hostname']),
                            'LOCATION' => $this->res[$i]['location'],
                            'INFOR' => $this->res[$i]['infor'],
                            'CERNET' => $this->res[$i]['cernet']
                            );    
        if (!strcmp($this->res[$i]['userid'],'guest'))
            $this->telnetguest ++;
        if (!strcmp($this->res[$i]['userid'],'_wwwguest'))
            $this->wwwguest ++;
        if ($this->res[$i]['cernet'])
            $this->cernet ++ ;
        if (stristr($this->res[$i]['location'].$this->res[$i]['infor'],KEYWORD_THU))
            $this->thu ++ ;
    }
    return true;            
}

function show() {
?>
<center>
<table cellspacing="0" cellpadding="3" border="0" class="t1">
<tbody><tr><td width="40" class="t2">序号</td><td width="120" class="t2">用户名</td><td width="120" class="t2">地址</td>
<td class="t2">来源</td><td width="40" class="t2">Cernet</td></tr></tbody>
<?php
    for ($i = 0 ; $i < $this->usernum ; $i ++ ) {
        echo '<tbody><tr><td class="t3">'.($i+1).'</td><td class="t4"><a href="/bbsqry.php?userid='.$this->users[$i]['USERID'].'">'.$this->users[$i]['USERID'].'</a></td>'.
             '<td class="t7">'.$this->users[$i]['HOST'].'</td><td class="t8">'.
             htmlspecialchars($this->users[$i]['LOCATION'].' '.$this->users[$i]['INFOR']).
             '</td><td class="t3">'.($this->users[$i]['CERNET']?'Y':'&nbsp;').'</td></tr></tbody>';
    }
?>
</table>
</center>
<?php    
}

function show_board_infor() {
?>
<p align="center"><a href="/bbsdoc.php?board=<?php echo $this->board; ?>"><?php echo $this->board.'('.$this->brdarr['DESC'].')'; ?></a>
讨论区共有在线用户 <font class="b3"><?php echo $this->usernum; ?></font> 人，Telnet-guest <font class="b3"><?php echo $this->telnetguest; ?></font> 人，
WWW-guest <font class="b3"><?php echo $this->wwwguest; ?></font> 人，中国教育网用户 <font class="b3"><?php echo $this->cernet; ?></font> 人，
<?php echo KEYWORD_THU; ?>用户 <font class="b3"><?php echo $this->thu; ?></font> 人。
</p>
<?php    
}

function quit() {
    mysql_close($this->link);
}
}

function bbs_analyse_choose($board) {
?>
<form action="/bbsboarduser.php" method="get">
<center>
请输入欲进行分析的版面
<input type="text" name="board" class="b1" value="<?php echo $board; ?>" />
<input type="submit" value="开始分析" class="a" />
</center>
</form>
<?php    
}

function bbs_analyse_checksql($sql) {
    if (!$sql) return false;
    if (stristr($sql,'INSERT')) return false;
    if (stristr($sql,'DROP'))   return false;
    if (stristr($sql,'DELETE')) return false;
    if (stristr($sql,'ALERT'))  return false;
    if (stristr($sql,'CREATE')) return false;
    if (stristr($sql,'OPTIMIZE')) return false;
    if (stristr($sql,'REPLACE')) return false;
    if (stristr($sql,'INFILE')) return false;
    if (stristr($sql,'UPDATE')) return false;
    if (stristr($sql,'FLUSH')) return false;
    if (stristr($sql,'KILL')) return false;
    if (stristr($sql,'LOCK')) return false;
    if (stristr($sql,'SET')) return false;
    if (stristr($sql,'GRANT')) return false;
    if (stristr($sql,'REVOKE')) return false;
    if (stristr($sql,'USE')) return false;
    if (!stristr($sql,'SELECT')) return false;
    return true;
}

function bbs_analyse_sql($board) {
?>
<form action="/bbsboarduser.php" method="get">
<center>
请输入SQL语句
<input type="hidden" name="board" value="<?php echo $board; ?>" />
<input type="hidden" name="act" value="sql" />
<input type="text" name="sql" class="b1" size="60" value="<?php if (isset($_GET['sql'])) echo htmlspecialchars($_GET['sql']); ?>" />
<input type="submit" value="提交" class="a" />
</center>
</form>
<?php    
}

function bbs_analyse_action($board) {
?>
<p align="center">
<form action="/bbsboarduser.php" method="get">
<input type="hidden" name="board" value="<?php echo $board; ?>" />
<input type="hidden" name="act" value="show" />
请选择要进行的操作
<select name="order">
<option value="1">以来源、用户名、教育网顺序显示分析结果</option>
<option value="2">以来源、教育网、用户名顺序显示分析结果</option>
<option value="3">以教育网、用户名、来源顺序显示分析结果</option>
<option value="4">以教育网、来源、用户名顺序显示分析结果</option>
<option value="0">以用户名顺序显示版面分析结果</option>
</select>
<input type="submit" value="提交" class="a" />
</form>
<br /><br />
<input type="submit" value="重新分析版面用户" class="a" onclick="window.location.href='bbsboarduser.php?board=<?php echo $board; ?>&act=reset'" />
<input type="submit" value="清空版面分析结果" class="a" onclick="window.location.href='bbsboarduser.php?board=<?php echo $board; ?>&act=clear'" />
<input type="submit" value="选择版面" class="a" onclick="window.location.href='bbsboarduser.php'" />
</p>
<?php    
}

function bbs_analyse_nodata($board) {
?>
<p align="center">
<input type="submit" value="暂无版面信息，点击进行分析" class="a" onclick="window.location.href='bbsboarduser.php?board=<?php echo $board; ?>&act=reset'" />
<input type="submit" value="返回" class="a" onclick="history.go(-1)" />
</p>
<?php   
exit (); 
}

require('funcs.php');
login_init();
html_init('GB2312','版面在线用户分析','',1);

if ($loginok != 1)
    html_nologin();
if (!strcmp($currentuser["userid"],"guest"))
    html_error_quit("请先登录");
if (!($currentuser["userlevel"]&BBS_PERM_SYSOP))
    html_error_quit("您无权访问本页面");
if (isset($_GET['board']))
    $board = $_GET['board'];
else {
    bbs_analyse_choose('');
    exit ();
}

if (!($bu = new BoardUser($board)))
    html_error_quit($bu->err);
if (!$bu->bid)
    html_error_quit($bu->err);

switch ($_GET['act']) {
    case 'reset':
        $bu->reset_db();
        $bu->init_db();
        $bu->get_users();
        $bu->analyse();
        $bu->show_board_infor();
        $bu->show();
        break;
    case 'show':
        if (!$bu->read_res($_GET['order']))
            bbs_analyse_nodata($bu->board);
        $bu->show_board_infor();
        $bu->show();
        break;
    case 'sql':
        $sql = $_GET['sql'];
        if (!bbs_analyse_checksql($sql))
            html_error_quit('SQL语句为空或者含有非法字符');
        if (!$bu->read_sql($sql))
            bbs_analyse_nodata($bu->board);
        $bu->show_board_infor();
        $bu->show();
        break;
    case 'clear':
        $bu->reset_db();
        break;
    default:
        if (!$bu->read_res())
            bbs_analyse_nodata($bu->board);
        $bu->show_board_infor();
        $bu->show();
}


bbs_analyse_action($bu->board);
bbs_analyse_sql($bu->board);
bbs_analyse_choose($bu->board);
$bu->quit();
html_normal_quit();
?>