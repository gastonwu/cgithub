<?php
ini_set ('memory_limit', '2048M');

$million = 1000000;
$million10 = 100 * $million;
$billion = 1000 * $million;
$large_number =  4 * $billion;

$pos = 0;
$data = array();
for($i=0;$i<$large_number;$i++){
	if($i % $million == 0){
		echo ".";
	}
	if($i % $million10 == 0){
		echo "\n".date("Y-m-d H:i:s")."\n";
	}
	$data[$i] =1 ;
}
