<?php
function showUserManageMenu(){
?>
<table cellpadding=3 cellspacing=1 align=center class=tableborder1>
<tr>
<th width=14% height=25 id=tabletitlelink><a href=usermanager.php>控制面板首页</a></th>
<th width=14%  id=tabletitlelink><a href=mymodify.php>基本资料修改</a></th>
<th width=14%  id=tabletitlelink><a href=modifypsw.php>用户密码修改</a></th>
<th width=14%  id=tabletitlelink><a href=modifyadd.php>联系资料修改</a></th>
<th width=14%  id=tabletitlelink><a href=usersms.php>用户短信服务</a></th>
<th width=14%  id=tabletitlelink><a href=friendlist.php>编辑好友列表</a></th>
<th width=14%  id=tabletitlelink><a href=favlist.php>用户收藏管理</a></th>
</tr>
</table>
<?php
}

?>