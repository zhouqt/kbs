function loadThreadFollow(t_id,b_name){
	var targetImg =eval("document.all.followImg" + t_id);
	var targetDiv =eval("document.all.follow" + t_id);
	alert(t_id);
	if ("object"==typeof(targetImg)){
		if (targetDiv.style.display!='block'){
			targetDiv.style.display="block";
			targetImg.src="pic/nofollow.gif";
			if (targetImg.loaded=="no"){
				document.frames["hiddenframe"].location.replace("loadtree.php?bname="+b_name+"&ID="+t_id);
			}
		}else{
			targetDiv.style.display="none";
			targetImg.src="pic/plus.gif";
		}
	}
}
