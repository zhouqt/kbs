<?php
        /**
         * This file searches x boards to user.  @author bad
         */
        require("funcs.php");
login_init();
        if ($loginok !=1 )
                html_nologin();
        else
        {
                html_init("gb2312", "", "", 1);
                if (isset($_GET["q"]))
                    $select = $_GET["q"];
                else
                    $select = "";
                if (isset($_GET["b"]))
                    $board = $_GET["b"];
                else
                    $board = "";
?>
<style>
FONT.f001 {
    color: #e87874;
}
a:visited,a:link {
    text-decoration: underline;
}
</style>
<body onload="document.qf0.q.focus();">
<form action="bbsxsearch.php" name="qf0">
<table border="0" cellpadding="0" cellspacing="5">
<tr> 
<td><a href="/bbsxsearch.php"><img src="images/xsearch.jpg" border="0"></a></td>
<td align="left" valign="middle" style="font-size:10pt;">
  <h3><?php echo BBS_FULL_NAME; ?> -- 令狐冲精华区全文检索</h3>
  <input type="text" name="q" value="<?php echo $select; ?>" size="41"><br>
  版面名称：<input type="text" name="b" value="<?php echo $board; ?>" style="width: 100"> <input type="submit" style="width: 80px" value="<?php if ($select=="") echo "开始搜索"; else echo "重新搜索"; ?>">
</td>
</tr>
</table>
</form>

<?php

    function reg($num, $per) {
        return (floor($num / $per) * $per);
    }
    
                if ($select != "") {
                    $query = $select;
                    if ($board != "") $query = "=$board $select";
                    if (isset($_GET["s"])) $start = $_GET["s"]; else $start = 0;
                    if (!is_numeric($start)) $start = 0; if ($start < 0) $start = 0;
                    $per_page = 10;
                    $per_query = 100;
                    
                    $re = bbs_x_search($query, (int)($start/$per_query), $total_records);
                    $len = count($re);

                    $min = reg($start - $per_query, $per_page);
                    if ($min < 0) $min = 0;
                    $prev = reg($start - $per_page, $per_page);
                    if ($prev < 0) $prev = 0;
                    $max = $min + 2 * $per_query;
                    if ($max > $total_records - 1) $max = reg($total_records - 1, $per_page);
                    $next = ($start + $per_page) / $per_page * $per_page;
                    if ($next > $total_records - 1) $next = reg($total_records - 1, $per_page);
                    
                    $ss1 = $start + 1;
                    $ss2 = $ss1 + $per_page - 1;
                    if ($ss2 > $total_records) $ss2 = $total_records;
                    if ($ss2 >= $ss1) $str = "显示第 $ss1 条到 第 $ss2 条记录。";
                    else $str = "";

                    echo "<p><b>查到 $total_records 条记录。</b> $str</p>";
                    $stop_index = $start % $per_query + $per_page;
                    if ($stop_index > $len) $stop_index = $len;
                    for ($i = $start % $per_query; $i < $stop_index; $i++) {
                        echo "<p><a href=\"bbsanc.php?path=".urlencode(substr($re[$i]["filename"],9))."\">".htmlspecialchars(trim($re[$i]["title"]))."</a> 位置：".htmlspecialchars($re[$i]["path"])."<br /><span style=\"font-size: 10pt;\">".$re[$i]["content"]."</span></p>\n";
                    }
                    $links = array();
                    if ($prev < $start) $links[] = "<a href=\"bbsxsearch.php?q=".urlencode($select)."&b=".urlencode($board)."&s=$prev\">上一页</a>";
                    for ($i = $min; $i <= $max; $i += $per_page) {
                        if ($start >= $i && $start < $i + $per_page) {
                            $links[] = $i/$per_page+1;
                        } else {
                            $links[] = "<a href=\"bbsxsearch.php?q=".urlencode($select)."&b=".urlencode($board)."&s=$i\">".(int)($i/$per_page+1)."</a>";
                        }
                    }
                    if ($next > $start) $links[] = "<a href=\"bbsxsearch.php?q=".urlencode($select)."&b=".urlencode($board)."&s=$next\">下一页</a>";
                    $len = count($links);
                    if ($len > 1) {
                        echo "<p align=\"center\">";
                        for ($i = 0; $i < $len - 1; $i++) {
                            echo $links[$i]." ";
                        }
                        echo $links[$len - 1];
                        echo "</p>";
                    }
                }
                html_normal_quit();
        }
?>

