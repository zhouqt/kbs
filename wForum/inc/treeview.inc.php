<?php

/*
 * 树形回复结构入口函数 showTree(boardName, groupID, articles, displayFN, maxThread=100, startNum=0)
 *
 * @param boardName    版面英文名称
 * @param groupID      本主题的 groupID
 * @param articles     从 bbs_get_threads_from_gid() 或类似函数返回的 $articles 数组
 * @param displayFN    用于显示每个帖子的回调函数
 *        displayFN(boardName,groupID,article,startNum,level,lastflag)
 *        @param boardName,groupID    同上
 *        @param article              这篇文章。如果 == null 用于显示指示还有更多的省略号
 *        @param startNum             这篇文章在这个主题中的序号
 *        @param level                深入第几层。原贴 level = 0
 *        @param lastflag             $lastflag[$l] 表示第 $l 层是否还有更多回复
 * @param maxThread    最多显示多少个帖子，最好是奇数。默认 101。
 * @param startNum     如果本主题帖子数目超过maxThread，就平级显示startNum附近的maxThread个主题以及本主题第一个帖子
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

function showTree($boardName, $groupID, $articles, $displayFN, $maxthread = 101, $startNum = 0) {
	$threadNum = count($articles);
	$more = ($threadNum > $maxthread);
	$lastflag = array();
	if ($more) {
		$start = $startNum - (int)($maxthread / 2);
		if ($start < 0) $start = 1;
		$end = $start + $maxthread;
		if ($end > $threadNum) {
			$end = $threadNum;
			$start = $threadNum - $maxthread;
		}

		$displayFN($boardName, $groupID, $articles[0], 0, 0, $lastflag);
		$lastflag[0] = true;
		if ($start > 1) $displayFN($boardName, $groupID, null, $start - 1, 1, $lastflag);
		for($i = $start; $i < $end; $i++) {
			$lastflag[0] = ($i != $threadNum - 1);
			$displayFN($boardName, $groupID, $articles[$i], $i, 1, $lastflag);
		}
		if ($lastflag[0]) { //嘿嘿，这个有点诡异 :p
			$lastflag[0] = false;
			$displayFN($boardName, $groupID, null, $end, 1, $lastflag);
		}
	} else {
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
		
		showTreeRecursively($boardName, $groupID, $treenodes, 0, 0, $lastflag, $displayFN);
		for($i=0; $i < $threadNum; $i++) { // 可怜的孩子，没有爹的帖子
			if (!$treenodes[$i]->showed) {
				//$displayFN($boardName, $groupID, $treenodes[$i]->data, $i, 0, $lastflag);
				showTreeRecursively($boardName, $groupID, $treenodes, $i, 0, $lastflag, $displayFN);
			}
		}
	}
}

function showTreeRecursively($boardName, $groupID, &$treenodes, $index, $level, &$lastflag, $displayFN) {
	/*
	 * 这里其实有个问题，如果回复结构有环的话会无穷递归，可以在这里判断一下 $treenodes[$index]->showed 是不是 false.
	 * 不过 smthbbs 系统应该不会产生这种环，就暂时不管了。
	 */
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
