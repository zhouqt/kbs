<?php
require("pcadmin_inc.php");
pc_admin_check_permission();
$link = pc_db_connect();
$fid = intval($_GET["fid"]);
$query = 'SELECT * FROM filter WHERE fid = '.$fid.  ' LIMIT 1;';
$result = mysql_query($query);
$node = mysql_fetch_array($result);
if (!$node)
    html_error_quit("文章不存在");
if (strtolower($_GET['filter'])=='n') {
    if ($node[state]==0 || $node[state]==2) {
        $query = 'UPDATE filter SET state = 1 WHERE fid = '.$fid.' LIMIT 1;';
        mysql_query($query);
        $pc = pc_load_infor($link,"",$node[uid]);
        
        if ($node[nid])//过滤的是评论
            $ret = pc_add_comment($link,$pc,$node[nid],$node[emote],$node[username],$node[subject],$node[body],$node[htmltag],true,$node[hostname]);
        else
            $ret = pc_add_node($link,$pc,$node[pid],$node[tid],$node[emote],$node[comment],$node[access],$node[htmltag],$node[trackback],$node[subject],$node[body],$node[nodetype],$node[auto_tbp],$node[tbp_url],$node[tbp_art],$node[tbpencoding],true,$node[hostname]);
                switch($ret)
				{
					case -1:
						html_error_quit("缺少日志主题");
						exit();
						break;
					case -2:
						html_error_quit("目录不存在");
						exit();
						break;
					case -3:
						html_error_quit("该目录的日志数已达上限");
						exit();
						break;
					case -4:
						html_error_quit("分类不存在");
						exit();
						break;
					case -5:
						html_error_quit("由于系统原因日志添加失败,请联系管理员");
						exit();
						break;
					case -6:
						$error_alert = "由于系统错误,引用通告发送失败!";
						break;
					case -7:
						$error_alert = "TrackBack Ping URL 错误,引用通告发送失败!";
						break;
					case -8:
						$error_alert = "对方服务器无响应,引用通告发送失败!";
						break;
				    case -9:
				        $error_alert = "您的文章可能含有不当词汇，请等待管理员审核。";
				        break;
					default:
				}
    }    
}

if (strtolower($_GET['filter'])=='y') {
    if ($node[state]==0 || $node[state]==1) {
        $query = 'UPDATE filter SET state = 2 WHERE fid = '.$fid.' LIMIT 1;';
        mysql_query($query);
    }
}

pc_db_close($link);
pc_return("pcdoc.php?userid=_filter&tag=".$node[state]);
?>
