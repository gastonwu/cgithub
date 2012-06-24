<?php
require_once "bitmap.php";
ini_set ('memory_limit', '1024M');

$million = 1000000;
$million10 = 100 * $million;
$billion = 1000 * $million;
$large_number =  4 * $billion;

$bitmap = new BitMap();
$pos = 0;
for($i=0;$i<$large_number;$i++){
	if($i % $million == 0){
		echo ".";
	}
	if($i % $million10 == 0){
		echo "\n".date("Y-m-d H:i:s")."\n";
	}
	$bitmap->set($i);
	
}