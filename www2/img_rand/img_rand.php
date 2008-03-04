<?php
// numric auth. hehe.. by hightman@hightman.net

$radix = "ABCEFGHIJKLMNPQRSTWXYZ123456789";
$radix_max = strlen($radix) - 1;
$maxlen = 4;

mt_srand();

$fpath = "img_rand/" . mt_rand(0, 6) . ".jpg";
$font = $_SERVER["DOCUMENT_ROOT"] . "/img_rand/font_rand/" . mt_rand(0, 9) . ".ttf";

$im = ImageCreateFromJPEG($fpath); // width=200 height=10
$color = ImageColorAllocate($im, 0, 0, 0);

$len = $maxlen;
$bwidth = 200 / $maxlen;
$rand_str = "";

while ($len) {
    $len --;
    $i = mt_rand(0, $radix_max);
    $j = mt_rand(20, 30);
    $x = 170 - $len * $bwidth;
    $y = 15 + $j;
    $angle = $i - 15;
    Imagettftext($im, $j, $i - 15, $x, $y, $color, $font, $radix[$i]);
    $rand_str .= $radix[$i];
}

session_set_cookie_params(3600);
session_start();
$_SESSION['num_auth'] = str_replace(" ", "", $rand_str);

header("Content-Type: image/png");
ImagePng($im);
ImageDestroy($im);
exit();
?>

