<?php
require("wapfuncs.php");
if(loginok())
{
  //copy from mainpage.php
  $hotsubject_file = BBS_HOME . "/xml/day.xml";
  $doc = domxml_open_file($hotsubject_file);
  if (!$doc)
    return;
  $root = $doc->document_element();
  $boards = $root->child_nodes();
  $brdarr = array();

  waphead(0);
  echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
  echo "<p>";
  while($board = array_shift($boards))
    {
      if ($board->node_type() == XML_TEXT_NODE)
        continue;

      $hot_title = find_content($board, "title");
      $title = smarticonv($hot_title);
      $hot_author = find_content($board, "author");
      $hot_board = find_content($board, "board");
      //$hot_time = find_content($board, "time");
      //$hot_number = find_content($board, "number");
      $hot_groupid = find_content($board, "groupid");

      $brdnum = bbs_getboard($hot_board, $brdarr);
      if ($brdnum == 0)
        continue;
      echo "$hot_author [$hot_board]";
      showlink(urlstr("readpost",array(),encarg("readpost",array("board"=>$brdnum,"id" => $hot_groupid,"way" => 3))),"$title");
      echo "<br/>";

    }
  showlink(urlstr("menu"),Ö÷²Ëµ¥);
  echo "</p>";
}
?>
</card>
</wml>

<?php
//functions for xml access
function get_content($parent)
{
  $nodes = $parent->child_nodes();
  while($node = array_shift($nodes))
    if ($node->node_type() == XML_TEXT_NODE)
      return $node->node_value();
  return "";
}

function find_content($parent,$name)
{
  $nodes = $parent->child_nodes();
  while($node = array_shift($nodes))
    if ($node->node_name() == $name)
      return get_content($node);
  return "";
}

function find_attr($parent,$name,$attr)
{
  $nodes = $parent->child_nodes();
  while($node = array_shift($nodes))
    if ($node->node_name() == $name)
      return $node->get_attribute($attr);
  return "";
}
//end for xml access functions

?>
