declare option exist:serialize "method=xhtml media-type=text/html";

let $host := "http://kitwallace.co.uk/rt/"
let $field := "sw"
let $stream := "streamid"
let $pk := "streampk"

let $baseurl :=concat($host,"swcontrol.xq")
let $seturl := concat($host,"home.xq?_action=store&amp;_id=",$stream,"&amp;_pk=",$pk,"&amp;",$field,"=")
let $geturl := concat($host,"home.xq?_action=value&amp;_id=",$stream,"&amp;_pk=",$pk,"&amp;_field=",$field)
let $command := request:get-parameter("command",())

let $update := if (exists($command)) 
               then
                  let $url := concat($seturl,$command)
                  let $doit := httpclient:get(xs:anyURI($url),false(),())
                  return response:redirect-to(xs:anyURI($baseurl)) 
               else ()
               
let $current := httpclient:get(xs:anyURI($geturl),false(),())
return             
<html>
  <head>      
     <meta name="viewport" content="width=device-width, initial-scale=1"/>
     <link href="http://kitwallace.co.uk/KWMC/images/icon-normal.png" rel="icon" sizes="128x128" />
     <link rel="shortcut icon" type="image/png" href="http://kitwallace.co.uk/KWMC/images/favicon.png"/>   
  </head>
  <body>
    <div> 
      <button onClick="javascript:document.location.href='?command=1'">
       {if (number($current)=0) then attribute style {"background-color: red"} else ()}
      Turn On</button> 
     
      <button onClick="javascript:document.location.href='?command=0'">
      {if (number($current)=1) then attribute style {"background-color: red"} else ()}
       Turn Off</button> 
    </div>
 </body>
</html>
