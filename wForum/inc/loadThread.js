function loadThreadFollow(t_id,b_name){
	var targetImg=getRawObject("followImg"+t_id);
	var targetDiv =getRawObject("follow" + t_id);

	if ("object"==typeof(targetImg)){
		if (targetDiv.style.display=='none'){
//			targetDiv.style.display="block";
			targetDiv.style.display="";

			targetImg.src="pic/nofollow.gif";
			window.frames["hiddenframe"].document.location.href="loadtree.php?bname="+b_name+"&ID="+t_id;
		}else{
			targetDiv.style.display="none";
			targetImg.src="pic/plus.gif";
		}
	}
}
