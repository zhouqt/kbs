<?php

/*
 * 树形回复结构入口函数 showTree(boardName, groupID, articles, displayFN)
 *
 * @param boardName    版面英文名称
 * @param groupID      本主题的 groupID
 * @param articles     从 bbs_get_threads_from_gid() 或类似函数返回的 $articles 数组
 * @param displayFN    用于显示每个帖子的回调函数
 *        displayFN(boardName,groupID,article,startNum,level,lastflag)
 *        @param boardName,groupID    同上
 *        @param article              这篇文章
 *        @param startNum             这篇文章在这个主题中的序号
 *        @param level                深入第几层。原贴 level = 0
 *        @param lastflag             $lastflag[$l] 表示第 $l 层是否还有更多回复
 *
 * @author atppp
 */

class TreeNode {
	var $data;
	var $index;
	var $showed;
	var $first_child;
	var $last_child;
	var $next_sibling;
	
	function TreeNode($data, $index) {
		$this->data = &$data;
		$this->index = $index;
		$this->showed = false;
		$this->first_child = $this->last_child = $this->next_sibling = null;
	}
	
	function addChild(&$node) { /* here it's very important to assign by reference */
		if ($this->first_child == null) $this->first_child = &$node;
		if ($this->last_child != null) {
			$this->last_child->next_sibling = &$node;
		}
		$this->last_child = &$node;
	}
}

function showTree($boardName,$groupID,$articles,$displayFN) {
	$threadNum = count($articles);
	$more = ($threadNum > 100);
	if ($more) $threadNum = 100; //最多显示100个，先这样吧。
	
	/* 产生回复树结构 */
	$treenodes = array();
	for($i=0; $i < $threadNum; $i++) {
		$treenodes[$i] = new TreeNode($articles[$i], $i);
	}
	for($i=1; $i < $threadNum; $i++) {
		for ($j=0; $j < $i; $j++) {
			if ($articles[$i]['REID'] == $articles[$j]['ID']) {
				$treenodes[$j]->addChild($treenodes[$i]);
				break;
			}
		}
	}
	
	$lastflag = array();
	showTreeRecursively($boardName, $groupID, $treenodes, 0, 0, $lastflag, $displayFN);
	for($i=0; $i < $threadNum; $i++) { // 可怜的孩子，没有爹的帖子
		if (!$treenodes[$i]->showed)
			$displayFN($boardName, $groupID, $treenodes[$i]->data, $i, 0, $lastflag);
	}	
	if ($more) echo '<tr><td class=TableBody1 width="100%" height=25 style="color:red"> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;... 还有更多 ...</td></tr>';
}

function showTreeRecursively($boardName, $groupID, &$treenodes, $index, $level, &$lastflag, $displayFN) {
	$displayFN($boardName, $groupID, $treenodes[$index]->data, $index, $level, $lastflag);
	$treenodes[$index]->showed = true;
	$cur = &$treenodes[$index]->first_child;
	while($cur != null) {
		$temp = &$cur->next_sibling;
		$lastflag[$level] = ($temp != null);
		showTreeRecursively($boardName, $groupID, $treenodes, $cur->index, $level+1, $lastflag, $displayFN);
		$cur = &$temp;
	}
}

?>
