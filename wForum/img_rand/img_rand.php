<?php
// numric auth. hehe.. by hightman@hightman.net
// $Id$

$radix = "0123456789";
$maxlen = 4;

mt_srand();

$fpath = "img_rand/" . mt_rand(0, 6) . ".jpg";
$font =  $_SERVER['DOCUMENT_ROOT']."/img_rand/font_rand/" . mt_rand(0, 18) . ".ttf";
$im = ImageCreateFromJPEG($fpath); // width=200 height=10
$color = ImageColorAllocate($im, 0, 0, 0);

$len = $maxlen;
$bwidth = 200 / $maxlen;
$rand_str = "";

while ($len) {
    $len --;
    $i = mt_rand(0, 9);
    $j = mt_rand(20, 30);
    $x = 170 - $len * $bwidth;
    $y = 15 + $j;
    $angle = $i - 15;
    Imagettftext($im, $j, $i - 15, $x, $y, $color, $font, $radix[$i]);
    $rand_str .= $radix[$i];
}

session_start();
$_SESSION['num_auth'] = str_replace(" ", "", $rand_str);

header("Content-Type: image/jpeg");
ImageJpeg($im);
ImageDestroy($im);
exit();
?>

