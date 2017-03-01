<?php 

$L = $argv[1];

$f[1] = fopen("oc300000_l".$L."_1_etapas.csv", "r");
$f[2] = fopen("oc300000_l".$L."_2_etapas.csv", "r");
$f[3] = fopen("oc300000_l".$L."_3_etapas.csv", "r");

$finish = fopen("oc300000_l".$L."_compmedia.csv", "w+");

$first =true;

fwrite($finish,"I;Media\n");
$x=0;
while (true) {
	for($i=1;$i<=3;$i++){
		$line[$i] = fgets($f[$i]);
	}
	
	$ok = false;
	for($i=1;$i<=3;$i++){
		if($line[$i] !== false){
			$ok = true;
		}
	}
	if(!$ok)
		break;


	if($first){
		$first = false;
		continue;
	}

	for($i=1;$i<=3;$i++){
		if(isset($line[$i]))
			$l[$i] = explode(";", $line[$i]);
	}

	$count = 0;
	$total = 0;
	for($i=1;$i<=3;$i++){
		if(isset($line[$i])){
			if(isset($l[$i][1])){
				$total++;
				$count+=$l[$i][1];
			}
		}
	}
	$media = $count/$total;

	fwrite($finish,++$x.";".$media."\n");
}

fclose($f[1]);
fclose($f[2]);
fclose($f[3]);
fclose($finish);

?>