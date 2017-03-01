<?php 
	$prefix = $argv[1];
	$number = $argv[2];
	$total = $argv[3];

	for($i=1;$i<=$total;$i++){
		$filename = $prefix."_".$number."_".$i.".txt";
		$f = fopen($filename, "r+");
		$text = fread($f,filesize($filename));

		echo $text."\n";
	}
?>