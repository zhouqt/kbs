<?php
        /**
         * This file searches x boards to user.  @author bad
         */
        require("funcs.php");
        if ($loginok !=1 )
                html_nologin();
        else
        {
                html_init("gb2312");
                if (isset($_GET["q"]))
                        $select = $_GET["q"];
                else
                        $select = "";
?>
<style type="text/css">A {color: #0000f0}</style>
<body>
<center><?php echo BBS_FULL_NAME; ?> -- 令狐冲精华区全文检索
<form action="bbsxsearch.php" name="qf0">
<table width="640" border="0" align="center" cellpadding="0" cellspacing="0">
<tr> 
<td width="74"><a href="/bbsxsearch.php"><img src="images/xsearch.jpg" border="0"></a></td>
<td align="center" valign="middle">
  <input type="text" name="q" value="<?php echo $select; ?>" style="width: 500"><br>
  <select name="area" size="1" style="width: 160" onChange="changeBoard0();">
  <option value="" selected>--- 全部的分类 ---</option>
  <option value="1">BBS系统  -- [系统]</option>
  </select>
  <select name="board" style="width: 245px" size="1">
  <option value="" selected>--- 全部版面 ---</option>
  </select>
  <input type="submit" style="width: 80px" value="<?php if ($select=="") echo "开始搜索"; else echo "重新搜索"; ?>">
</td>
</tr>
</table>
</form>

</center>

<?php
                html_normal_quit();
        }
?>

