<?php
define("USERFACE_IMG_NUMS",60);
define("USERFACE_IMG_CUSTOM",-1);
define("USERFACE_IMG_URL",-2);
$groups=array("无门无派");
$shengxiao=array("未知","鼠","牛","虎","兔","龙","蛇","马","羊","猴","鸡","狗","猪");
$bloodtype=array("未知","A","B","O","AB","其他");
$religion=array("未知","佛教", "道教", "基督教","天主教", "回教" , "无神论者", "共产主义者", "其他");
$profession=array("未知" ,"学生", "财会/金融", "工程师", "顾问", "计算机相关行业", "家庭主妇", "教育/培训",
"客户服务/支持","零售商/手工工人", "退休","无职业","销售/市场/广告","研究和开发","一般管理/监督",
"政府/军队","执行官/高级管理","制造/生产/操作","专业人员","自雇/业主","其他");
$married=array("未知","未婚","已婚","离异","丧偶");
$graduate=array("未知","小学","初中","高中","大学","研究生","博士");
$character=array("未知","多重性格","乐天达观", "成熟稳重","幼稚调皮","温柔体贴","活泼可爱","普普通通","内向害羞",
"外向开朗", "心地善良", "聪明伶俐", "善解人意", "风趣幽默", "思想开放","积极进取", "小心谨慎","郁郁寡欢",
"正义正直","悲观失意", "好吃懒做", "处事洒脱" ,"疑神疑鬼","患得患失","异想天开", "多愁善感", "淡泊名利",
"见利忘义", "瞻前顾后", "循规蹈矩", "热心助人", "快言快语", "少言寡语","爱管闲事","追求刺激","豪放不羁",
"狡猾多变", "贪小便宜", "见异思迁", "情绪多变", "水性扬花", "重色轻友", "胆小怕事", "积极负责","勇敢正义",
"聪明好学", "实事求是", "务实实际", "老实巴交", "圆滑老练", "脾气暴躁", "慢条斯理", "诚实坦白","婆婆妈妈","急性子");

function get_astro($birthmonth, $birthday)
{
	if (($birthmonth==0) || ($birthday==0) ){
		return "<font color=gray>未知</font>";
	}
	$birth=$birthmonth.'月'.$birthday.'日';
	switch ($birthmonth){
	case 1:
		if ($birthday>=21)	{
			$function_ret="<img src=star/z11.gif alt=水瓶座".$birth."> 水瓶座";
		}else{
			$function_ret="<img src=star/z10.gif alt=魔羯座".$birth."> 魔羯座";
		} 
		break;
	case 2:
		if ($birthday>=20)	{
			$function_ret="<img src=star/z12.gif alt=双鱼座".$birth."> 双鱼座";
		}else{
			$function_ret="<img src=star/z11.gif alt=水瓶座".$birth."> 水瓶座";
		} 
		break;
	case 3:
		if ($birthday>=21)	{
			$function_ret="<img src=star/z1.gif alt=白羊座".$birth."> 白羊座";
		}else{
			$function_ret="<img src=star/z12.gif alt=双鱼座".$birth."> 双鱼座";
		} 
		break;
	case 4:
		if ($birthday>=21)	{
			$function_ret="<img src=star/z2.gif alt=金牛座".$birth."> 金牛座";
		}else{
			$function_ret="<img src=star/z1.gif alt=白羊座".$birth."> 白羊座";
		} 
		break;
	case 5:
		if ($birthday>=22)	{
			$function_ret="<img src=star/z3.gif alt=双子座".$birth."> 双子座";
		}else{
			$function_ret="<img src=star/z2.gif alt=金牛座".$birth."> 金牛座";
		} 
		break;
	case 6:
		if ($birthday>=22)	{
			$function_ret="<img src=star/z4.gif alt=巨蟹座".$birth."> 巨蟹座";
		}else{
			$function_ret="<img src=star/z3.gif alt=双子座".$birth."> 双子座";
		} 
		break;
	case 7:
		if ($birthday>=23)	{
			$function_ret="<img src=star/z5.gif alt=狮子座".$birth."> 狮子座";
		}else{
			$function_ret="<img src=star/z4.gif alt=巨蟹座".$birth."> 巨蟹座";
		} 
		break;
	case 8:
		if ($birthday>=24){
			$function_ret="<img src=star/z6.gif alt=处女座".$birth."> 处女座";
		}else{
			$function_ret="<img src=star/z5.gif alt=狮子座".$birth."> 狮子座";
		} 
		break;
	case 9:
		if ($birthday>=24)	{
			$function_ret="<img src=star/z7.gif alt=天秤座".$birth."> 天秤座";
		}else{
			$function_ret="<img src=star/z6.gif alt=处女座".$birth."> 处女座";
		} 
		break;
	case 10:
		if ($birthday>=24){
			$function_ret="<img src=star/z8.gif alt=天蝎座".$birth."> 天蝎座";
		}else{
			$function_ret="<img src=star/z7.gif alt=天秤座".$birth."> 天秤座";
		} 
		break;
	case 11:
		if ($birthday>=23)	{
			$function_ret="<img src=star/z9.gif alt=射手座".$birth."> 射手座";
		}else{
			$function_ret="<img src=star/z8.gif alt=天蝎座".$birth."> 天蝎座";
		} 
		break;
	case 12:
		if ($birthday>=22)	{
			$function_ret="<img src=star/z10.gif alt=魔羯座".$birth."> 魔羯座";
		}else{
			$function_ret="<img src=star/z9.gif alt=射手座".$birth."> 射手座";
		} 
		break;
	default:
		$function_ret="";
		break;
	} 
	return $function_ret;
}
 ?>