<?php
    require("funcs.php");
    if ($loginok != 1)
		html_nologin();
    else
    {
        html_init("gb2312");
        if ($currentuser["userid"]=="guest")
            html_error_quit("匆匆过客不能设定好友");

		if( isset( $_GET["start"] ) ){
			$start = $_GET["start"];
			settype($start, "integer");
		}else
			$start = 0;

		$total = bbs_countfriends($currentuser["userid"]);

		if( $total < 0 ){
			html_error_quit("系统错误");
		}
?>
<center>
水母清华 -- 好友名单 [使用者: <?php echo $currentuser["userid"];?>]
<hr color="green"><br>
<?php
		if( $total != 0 ){
			if($start >= $total){
				$start = $total - 20;
				if($start < 0) $start = 0;
			}else if($start < 0){
				$start = $total - 20;
				if($start < 0) $start = 0;
			}

			$friends = bbs_getfriends($currentuser["userid"], $start);
			if ($friends == FALSE){
				html_error_quit("系统错误1");
			}
?>
您共设定了 <?php echo $total;?> 位好友<br>
<table border=1>
<tr><td>序号</td><td>好友代号</td><td>好友说明</td><td>删除好友</td></tr>
<?php
			$i=0;
			
			foreach ($friends as $friend){
				$i++;
?>
<tr><td><?php echo $start+$i;?></td>
<td><a href="/bbsqry.php?userid=<?php echo $friend["ID"];?>"><?php echo $friend["ID"];?></a></td>
<td><?php echo $friend["EXP"];?></td>
<td>[<a onclick="return confirm('确实删除吗?')" href="/bbsfdel.php?userid=<?php echo $friend["ID"];?>">删除</a>]</td>
</tr>
<?php
			}
?>
</table>
<?php
		}
?>
<hr color="green">
[<a href="/cgi-bin/bbs/bbsfadd">添加新的好友</a>]
<?php
		if( $start > 0 ){
?>
[<a href="/bbsfall.php?start=0">第一页</a>]
[<a href="/bbsfall.php?start=<?php if($start > 20) echo $start - 20; else echo "0";?>">上一页</a>]
<?php
		}
		if( $start < $total - 20 ){
?>
[<a href="/bbsfall.php?start=<?php echo $start + 20; ?>">下一页</a>]
[<a href="/bbsfall.php?start=-1">最后一页</a>]
</center>
<?php
		}

		html_normal_quit();
    }
?>
