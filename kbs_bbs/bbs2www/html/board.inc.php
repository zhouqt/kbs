<?php
function bbs_boards_navigation_bar()
{
?>
<p align="center">
[<a href="/mainpage.html">首页导读</a>]
[<a href="/bbssec.php">分类讨论区</a>]
[<a href="/bbsnewbrd.php">新开讨论区</a>]
[<a href="/bbsrecbrd.php">推荐讨论区</a>]
[<a href="/bbsbrdran.php">讨论区人气排名</a>]
[<a href="/cgi-bin/bbs/bbs0an">精华公布栏</a>]
[<a href="javascript:history.go(-1)">快速返回</a>]
<br />
</p>
<?php	
}

function undo_html_format($str)
{
	$str = preg_replace("/&gt;/i", ">", $str);
	$str = preg_replace("/&lt;/i", "<", $str);
	$str = preg_replace("/&quot;/i", "\"", $str);
	$str = preg_replace("/&amp;/i", "&", $str);
	return $str;
}

# iterate through an array of nodes
# looking for a text node
# return its content
function get_content($parent)
{
    $nodes = $parent->child_nodes();
    while($node = array_shift($nodes))
        if ($node->node_type() == XML_TEXT_NODE)
            return $node->node_value();
    return "";
}

# get the content of a particular node
function find_content($parent,$name)
{
    $nodes = $parent->child_nodes();
    while($node = array_shift($nodes))
        if ($node->node_name() == $name)
            return undo_html_format(urldecode(get_content($node)));
    return "";
}

?>