<?php

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
            return get_content($node);
    return "";
}

# get an attribute from a particular node
function find_attr($parent,$name,$attr)
{
    $nodes = $parent->child_nodes();
    while($node = array_shift($nodes))
        if ($node->node_name() == $name)
            return $node->get_attribute($attr);
    return "";
}

# load xml doc
$doc = domxml_open_file("board.xml") or die("What boards?");


$root = $doc->document_element();
$boards = $root->child_nodes();


?>
<html>
<head>
<title>ˮľ�廪bbsվ</title>
<meta HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=gb2312"> 
<link rel="stylesheet" type="text/css" href="/bbs.css">
<link rel="stylesheet" type="text/css" href="/ansi.css">
</head>
<body>
<table align="center" class="BODY">
<tr>
<td class="default">English Name</td>
<td class="default">Chinese Name</td>
<td class="default">Visit Times</td>
<td class="default">Stay Time</td>
</tr>
<?php
# shift through the array
while($board = array_shift($boards))
{
    if ($board->node_type() == XML_TEXT_NODE)
        continue;

    $ename = find_content($board, "EnglishName");
    $cname = find_content($board, "ChineseName");
    $visittimes = find_content($board, "VisitTimes");
    $staytime = find_content($board, "StayTime");
    $hours = $staytime / 3600;
    settype($hours, "integer");
    $minutes = ($staytime - $hours * 3600) / 60;
    settype($minutes, "integer");
    $seconds = $staytime - $hours * 3600 - $minutes * 60;
?>
<tr>
<td class="default"><?php echo $ename; ?></td>
<td class="default"><?php echo iconv("UTF-8", "GBK", $cname); ?></td>
<td class="default"><?php echo $visittimes; ?></td>
<td class="default"><?php echo $hours; ?>:<?php echo $minutes; ?>:<?php echo $seconds; ?></td>
</tr>
<?php
}
?>
</table>
</body>
</html>
<?php
?>
