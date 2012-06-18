<?php
class BitMap{
	private $bitmap;
	public function __construct(){
		$this->bitmap = array();
	}
	public function set($number){
		$pos = floor($number / 8);
		$charPos = $number % 8;

		$char = $this->bitmap[$pos];
		$this->bitmap[$pos] = $char | (1 << $charPos);
	}

	public function get($number){
		$pos = floor($number / 8);
		$charPos = $number % 8;

		$char = $this->bitmap[$pos];
		$flag = ($char & (1<<$charPos)) >> $charPos;

		return $flag;
	}
}
/**
$bitmap = new BitMap();
$bitmap->set(123);
$flag1 = $bitmap->get(123);
$flag2 = $bitmap->get(124);

printf("flag1:%d,flag2:%d\n",$flag1,$flag2);;
*/