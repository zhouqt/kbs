<?php
	$query = $_SERVER["QUERY_STRING"];
	settype($query, "string");
	$av = explode(".", $query);
	$ac = count($av);
	if ($ac == 4 || $ac == 5) {
		$ftype = 0;
		$num = 0;
		$ap = intval($av[3]);
	} else if ($ac == 6 || $ac == 7) {
		$ftype = intval($av[3]);
		$num = intval($av[4]);
		$ap = intval($av[5]);
	} else {
		die;
	}
	if ($ap <= 0) die;
	$bid = intval($av[1]);
	$id = intval($av[2]);
	header("Location: bbscon.php?bid=$bid&id=$id&ap=$ap");
?>
