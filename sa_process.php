<?php 

$it = $argv[1];
$a = $argv[2];
$better = $argv[3];

$f[1] = fopen("t5_it".$it."_a".$a."_1_etapas.csv", "r");
$f[2] = fopen("t5_it".$it."_a".$a."_2_etapas.csv", "r");
$f[3] = fopen("t5_it".$it."_a".$a."_3_etapas.csv", "r");

$finish = fopen("t5_it".$it."_a".$a."_compmedia.csv", "w+");

$first =true;

fwrite($finish,"I;T;Melhor;Media\n");
while ( (($line[1] = fgets($f[1])) !== false) && (($line[2] = fgets($f[2])) !== false) && (($line[3] = fgets($f[3])) !== false)) {
	if($first){
		$first = false;
		continue;
	}

	for($i=1;$i<=3;$i++){
		$l[$i] = explode(";", $line[$i]);
	}


	fwrite($finish,$l[1][0].";".$l[1][1].";".$l[$better][2].";".(($l[1][2]+$l[2][2]+$l[3][2])/3)."\n");
}

fclose($f[1]);
fclose($f[2]);
fclose($f[3]);
fclose($finish);

?>