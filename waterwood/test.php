<pre>
<?php

    /* this is an example of using kbs interface */

    $introot = "http://localhost:10080/interface/";
    $request = array();
    
    /* request 1 */
    $request[0]["thing"] = "board";
    $request[0]["bname"] = "Test";
    
    /* request 2 */
    $request[1]["thing"] = "topic";
    $request[1]["bname"] = "waterwood";
    $request[1]["start"] = "2";
    $request[1]["count"] = "3";
    
    /* make request */
    $requesttext = json_encode($request);
    $responsetext = http_post_data($introot . "read.php", $requesttext);
    
    /* resolve response */
    $pos = strpos($responsetext, "\r\n\r\n") + 4;
    $resulttext = substr($responsetext, $pos, strlen($responsetext) - $pos);
    $result = json_decode($resulttext);
    
    /* result is here */
    print_r($result);

?>
</pre>
