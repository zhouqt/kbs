<?php
        /**
         * This file lists fav boards to user.  @author caltary
         */
        require("funcs.php");
        if ($loginok !=1 )
                html_nologin();
        else
        {
                html_init("gb2312");
                if (isset($_GET["select"]))
                        $select = $_GET["select"];
                else
                        $select = 0;
                settype($select, "integer");
                if (isset($_GET["up"]))
                        $list_father=$_GET["up"];
                else
                        $list_father=-1;
                settype($list_father,"integer");
                
                if ($select < -1)// || $group > sizeof($section_nums))
                        html_error_quit("错误的参数");
                if(bbs_load_favboard($select)==-1)
                        html_error_quit("错误的参数");
                if (isset($_GET["delete"]))
                {
                        $delete_s=$_GET["delete"];
                        settype($delete_s,"integer");
                        bbs_del_favboard($delete_s);
                }
                if (isset($_GET["dname"]))
                {
                        $add_dname=$_GET["dname"];
                        bbs_add_favboarddir($add_dname);
                }
                if (isset($_GET["bname"]))
                {
                        $add_bname=$_GET["bname"];
                        $sssss=bbs_add_favboard($add_bname);
                }
                $boards = bbs_fav_boards($select, 1);
                if ($boards == FALSE)
                        html_error_quit("读取版列表失败");
?>
<style type="text/css">A {color: #0000f0}</style>
<body>
<center><?php echo BBS_FULL_NAME; ?> -- 个人定制区
<?php
 if( $select===-1 ) echo "[根目录]";
?>
<a href="bbssec.php">分类讨论区</a>
<hr class="default"/>
<table width="610">
<tr>
<td>序号</td>
<td>未</td>
<td>讨论区名称</td>
<td>类别</td>
<td>中文描述</td>
<td>版主</td>
<td>文章数</td>
</tr>
<?php
                $brd_name = $boards["NAME"]; // 英文名
                $brd_desc = $boards["DESC"]; // 中文描述
                $brd_class = $boards["CLASS"]; // 版分类名
                $brd_bm = $boards["BM"]; // 版主
                $brd_artcnt = $boards["ARTCNT"]; // 文章数
                $brd_unread = $boards["UNREAD"]; // 未读标记
                $brd_zapped = $boards["ZAPPED"]; // 是否被 z 掉
                $brd_position= $boards["POSITION"];//位置
                $brd_flag= $boards["FLAG"];//目录标识
                $brd_bid= $boards["BID"];//目录标识
                $rows = sizeof($brd_name);
                if($select != -1)
                {
?>
                <tr>
<td></td>
<td><img src=/images/folder.gif></td>
<td><a href="bbsfav.php?select=<?php echo $list_father; ?>">回到上一级</a></td>
<td colspan=4></td>
</tr>
<?php
                }
                for ($i = 0; $i < $rows; $i++)  
                {
                if( $brd_unread[$i] ==-1 && $brd_artcnt[$i] ==-1)
                        continue;
?>
<tr>
<td><?php echo $i+1; ?></td>
<td>
<?php
                        if ($brd_flag[$i] ==-1 )
                        {
?>
        <img src=/images/folder.gif></td>
        <td>
        <a href="bbsfav.php?select=<?php echo $brd_bid[$i];?>&up=<?php echo $select; ?>">
        <?php echo $brd_desc[$i];?>
        </a></td>
        <td colspan=4></td>
        <td><a href="bbsfav.php?select=<?php echo $select;?>&delete=<?php echo $brd_bid[$i];?>">删除</a></td>
        </tr>   
<?php
                                continue;
                        }
                        if ($brd_unread[$i] == 1)
                                echo "◆";
                        else
                                echo "◇";
?>
</td>
<td>
<?php
                        if ($brd_zapped[$i] == 1)
                                echo "*";
                        else
                                echo "&nbsp;";
?><a href="/bbsdoc.php?board=<?php echo urlencode($brd_name[$i]); ?>"><?php echo $brd_name[$i]; ?></a>
</td>
<td><?php echo $brd_class[$i]; ?></td>
<td>
<a href="/bbsdoc.php?board=<?php echo urlencode($brd_name[$i]); ?>"><?php echo $brd_desc[$i]; ?></a>
</td>
<td>
<?php
                        $bms = explode(" ", trim($brd_bm[$i]));
                        if (strlen($bms[0]) == 0 || $bms[0][0] <= chr(32))
                                echo "诚征版主中";
                        else
                        {
                                if (!ctype_print($bms[0][0]))
                                        echo $bms[0];
                                else
                                {
?>
<a href="/cgi-bin/bbs/bbsqry?userid=<?php echo $bms[0]; ?>"><?php echo $bms[0]; ?></a>
<?php
                                }
                        }
?>
</td>
<td><?php echo $brd_artcnt[$i]; ?></td>
<td>
<a href="bbsfav.php?select=<?php echo $select;?>&delete=<?php echo bbs_is_favboard($brd_position[$i])-1;?>">
删除</a>
</td>
</tr>
<?php
                }
?>
</table>
<br>
<form action=bbsfav.php>增加目录<input name=dname size=24 maxlength=20 type=text value=""><input type=submit value=确定><input type=hidden name=select value=<?php echo $select;?>></form>
<form action=bbsfav.php>增加版面<input name=bname size=24 maxlength=20 type=text value=""><input type=submit value=确定><input type=hidden name=select value=<?php echo $select;?>></from>
<hr class="default"/>
</center>

<?php
                bbs_release_favboard();
                html_normal_quit();
        }
?>
