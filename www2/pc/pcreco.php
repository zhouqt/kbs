<?php
require("pcfuncs.php");
require("pcstat.php");
require("pcmainfuncs.php");
$colors = array("#adbe00","#dfd581","#ff00cc","#ffb600","#00b6ef","#f75151","#00b6ef");

if(pc_update_cache_header(30))
	return;
	
function display_recommend_topic($link,$topic) {
    global $pcconfig,$users,$colors;
    $nodes = array();
    if (!getRecommendNodesByTopic($link,$topic,$nodes))
        return; 
?>
<tbody>
	<tr>
	    <td bgcolor="#999999" height="3"> </td>
	</tr>
	<tr>
	    <td>
	        <table class="table2" width="100%" cellspacing="0" cellpadding="3" >
				<tr>
					<td class="td3" bgcolor="<?php echo $colors[rand(0,sizeof($colors)-1)]; ?>" align="left"><b><a href="pcsec.php?sec=<?php echo $topic; ?>"><font color="white"><?php echo $pcconfig["SECTION"][$topic]; ?></font></a></b></td>			
				</tr>
				<tr>
					<td align="left" class="td3">
					<table width="100%">
<?php
    foreach($nodes as $node) {
        if (!$users[$node[uid]])
            $users[$node[uid]] = pc_load_infor($link,'',$node[uid]);
        echo '<tr><td align="left">[<a href="index.php?id='.$users[$node['uid']]['USER'].'"><font class="low2">'.$users[$node[uid]]['NAME'].'</font></a>]&nbsp;'.
             '<a href="pccon.php?id='.$node[uid].'&nid='.$node[nid].'&s=all">'.html_format($node[subject]).'</a>&nbsp;(<a href="/bbsqry.php?userid='.$users[$node[uid]]['USER'].'">'.$users[$node[uid]]['USER'].'</a>)</td>'.
             '<td align="right"><a href="/bbsqry.php?userid='.$node[recuser].'"><font class="low">'.$node[recuser].'</a> 推荐</font></td></tr>';
    }
    
?>					
					</table>
					<table width="100%"><tr><td align="right"><a href="pcreclist.php?topic=<?php echo $topic; ?>">更多推荐文章</a></td></tr></table>
					</td>			
				</tr>
			</table>
			    	</td>
	</tr>
</tbody>	
<?php    
}


$link = pc_db_connect();
pcmain_html_init();
$users = array();

?>
<tbody>
<form action="pcsearch.php" method="get" onsubmit="if(this.keyword.value==''){alert('请输入关键字');return false;}">
<tr>
	<td bgcolor="#F6F6F6">
	<table width="100%"><tr><td align="left">
	BLOG搜索
	<input name="keyword" type="text" class="textinput" size="20"> 
	<input type="hidden" name="exact" value="0">
	<select name="key">
	<option value="u" selected>用户名</option>
	<option value="c">Blog名</option>
	<option value="d">Blog描述</option>
	</select>
	<input type="submit" class="textinput" value="开始搜">
	</td><td align="right">
	<a href="/pc/pc.php">注册用户</a> 
	<a href="/pc/pcnew.php">最新日志</a> 
	<a href="/pc/pcnew.php?t=c">最新评论</a>
	</td></tr></table>
	</td>
</tr></form>
</tbody>
<?php
    $topics = array_keys($pcconfig["SECTION"]);
    foreach($topics as $topic) {
        display_recommend_topic($link,$topic);
    }
?>    
<tr>
	<td height="5"> </td>
</tr>
<?php
pc_db_close($link);
pcmain_html_quit()
?>