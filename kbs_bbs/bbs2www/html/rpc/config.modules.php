<?php
// $Id$

define ('MODULE_ACCESS_NONE',            0x00000000);
define ('MODULE_ACCESS_PUBLIC',          0x00000001);
define ('MODULE_ACCESS_KERNEL',          0x00000002);
define ('MODULE_ACCESS_HIDDEN',          0x00000004);
define ('MODULE_ACCESS_DISABLE',         0x00000008);
define ('MODULE_ACCESS_ANONYMOUS',       0x00000010);
    
$i = 0;
$smth_modules = array ();

/**
 * Module Configure Start
 */
$smth_modules[$i]['mid']        =  1;                     // Module ID
$smth_modules[$i]['name']       =  'Smth Alumni';         // Module Name
$smth_modules[$i]['access']     =  0;                     // Module Access
$smth_modules[$i]['version']    =  '0.9alpha';            // Module Version
$smth_modules[$i]['innerhost']  =  '10.0.3.2';         // Module Host (Inner Address)
$smth_modules[$i]['innerport']  =  8080;                    // Port (Inner Address)
$smth_modules[$i]['outerhost']  =  '166.111.8.235';         // Module Host (Outer Address)
$smth_modules[$i]['outerport']  =  8080;                    // Port (Outer Address)
$smth_modules[$i]['xmlrpcuser'] =  'alumni';              // XMLRPC User ID
$smth_modules[$i]['xmlrpcpass'] =  'alumni';              // XMLRPC Password
$smth_modules[$i]['xmlrpctype'] =  0;                     // HTTPS?
$smth_modules[$i]['xmlrpcsuser']=  'alumni';              // Module XMLRPC Service User ID
$smth_modules[$i]['xmlrpcspass']=  'alumni';              // Module XMLRPC Service Password
$smth_modules[$i]['xmlrpcstype']=  0;                     // HTTPS?
$smth_modules[$i]['xmlrpcspath']=  '/rpc/rpc.php';        // Module XMLRPC Service Path
$smth_modules[$i]['path']       =  '/';            // Module Path
$smth_modules[$i]['logo']       =  'modules/alumni.gif';  // Module Logo Image
$smth_modules[$i]['type']       =  2;                     // Module Type
$smth_modules[$i]['desc']       =  'Mtech Alumni System'; // Description
$i ++;
/**
 * Module Configure End
 */
?>