<?php
/**
 * web service for smthbbs
 * @author: windinsn May 8,2004
 */
require ('funcs.php');

$trust_hosts = array (
                    '10.0.3.2',
                    '166.111.8.235'
                    );

function web_service_check_host () {
    global $trust_hosts;
    $host = $_SERVER['REMOTE_ADDR'];
    if (!$host) return 0;
    foreach ($trust_hosts as $trust_host) {
        if (strcmp ($trust_host,$host) == 0)
            return 1;    
    }
    return 0;
}

function web_service_error ($errNo=1,$errMsg='') {
    //header("Content-Type: text/xml");
	//header("Content-Disposition: inline;filename=webservice.xml");
?>
<?xml version="1.0" encoding="UTF-8"?>
<response>
<error><?php echo $errNo; ?></error>
<?php
    if ($errMsg) {
?>
<message><?php echo rawurlencode ($errMsg); ?></message>
<?php
    }
?>
</response>    
<?php
    exit ();
}

function web_service_success () {
    //header("Content-Type: text/xml");
	//header("Content-Disposition: inline;filename=webservice.xml");
?>    
<?xml version="1.0" encoding="UTF-8"?>
<response>
<error>0</error>
</response>
<?php   
    exit (); 
}

function web_service_etems($etems) {
    //header("Content-Type: text/xml");
	//header("Content-Disposition: inline;filename=webservice.xml");
?>
<?xml version="1.0" encoding="UTF-8"?>
<response>
<error>0</error>
<data>
<?php
    $keys = array_keys ($etems);
    foreach ($keys as $key) {
        echo '<'.$key.'>'.rawurlencode ($etems[$key]).'</'.$key.'>';        
    }
?>
</data>
</response>
<?php   
    exit (); 
}

?>