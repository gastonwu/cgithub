<?php

header('Content-type:text/html;charset=utf-8');

//这里我使用专门用来测试sphinx的中文数据

$link = mysql_connect('localhost','root','');
mysql_query('set names utf8');
mysql_select_db('test',$link);

$sphinxLink = mysql_connect('127.0.0.1:9306');

//测试数据

//获取real-time当前最大id
//*
$maxRs = mysql_query('select * from testrt order by id desc limit 1 ',$sphinxLink);
$maxVal = @mysql_result($maxRs,0,'id');
$maxVal = empty($maxVal) ? 1 :$maxVal;
//$sets = mysql_query('select co_id,co_name,co_do from sph_comp limit 1,10000',$link);
$i = $maxVal; ##此处必须使用最大值，否则会出现数据插入失败
$time = time();

##注意原先可以使用 insert into values (value),(value)的逗号分隔的值方式，但这里防止内存不足，每次都直接插入索引

//while ($row=mysql_fetch_array($sets)) {
for($j=0;$j<5;$j++){
//$title = str_repeat($j,65538);
$title = '中文';
$sphinxSql = "insert into testrt(id,gid,title,content) values ($i,$time,'$title','content-{$time}')";
$res = mysql_query($sphinxSql,$sphinxLink);
//print_r($res);
$i++;
}
//*/

//搜索测试，针对中文

$key = '中文';

$sql = "select * from testrt where match('{$key}') limit 1";
echo $sql."\n";
$rs = mysql_query($sql);
print_r($rs);

while ($row = mysql_fetch_array($rs)) {
print_r($row);
}


?>

