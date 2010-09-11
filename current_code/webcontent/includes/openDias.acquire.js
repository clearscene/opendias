function updateProgressBar (progressId, device) {

  $.ajax({ url: "dynamic",
	 dataType: "xml",
	 data: {action: "getScanningProgress", 
		scanprogressid: progressId,
	       },
	 cache: false,
	 type: "POST",
	 success: function(dta){
           prog = parseInt( $(dta).find('progress').text() );
	   $("#progressbar_"+device).progressbar({
	     value: prog,
	   });
           if(prog != 100) {
             setTimeout("updateProgressBar('"+progressId+"','"+device+"')", 400);
           } else {
             alert("doc done.");
           }
	 }
       });
}


$(document).ready(function() {

  $("#tabs").tabs();

  $('#loading').canvasLoader({'radius':20, 'dotRadius':2});

  $.ajax({ url: "dynamic",
         dataType: "xml",
         data: {action: "getScannerList"},
         cache: false,
         type: "POST",
         success: function(data){
           if( $(data).find('error').text() ){
             alert("Error in fetching details.");
             return 1;
           }
           var deviceid=0;
           $(data).find('device').each( function() {

             deviceid++;
             var device=deviceid;

             // Create scan doc
             var div = document.createElement("div");
             div.setAttribute('id','deviceTab_'+device);
             var newTabHtml = document.getElementById('scannerTemplate').innerHTML;
             idchange = new Array('title', 'deviceid', 'format', 'pages', 'pagesSlider', 'resolution', 
                        'resolutionSlider', 'ocr', 'progressbar', 'resolutionDisplay', 'pagesDisplay', 
                        'scanButton', 'hlp', 'q');
             for (change in idchange) {
               //alert("replace: '" + idchange[change]+"_DEVICE'   with    '" + idchange[change]+"_"+device + "'.");
               newTabHtml = newTabHtml.replace(new RegExp(idchange[change]+"_DEVICE","g"), idchange[change]+"_"+device);
             }
             div.innerHTML = newTabHtml;
             $('#tabs').append(div);

             // Create new tab
             $('#tabs').tabs("add",'#deviceTab_'+device, 
                                      $(this).find("type").text() + ": " + 
                                      $(this).find("vendor").text() + " - " + 
                                      $(this).find("model").text() );

             // Bring the tab contents up-2-date
             $('#title_'+device).text( $(this).find("type").text() + ": " +
                                      $(this).find("vendor").text() + " - " +
                                      $(this).find("model").text() );
             $('#deviceid_'+device).val( $(this).find("name").text() );
             $('#hlp_'+device).val( $(this).find("hlp").text() );
             $('#q_'+device).val( $(this).find("q").text() );
             $('#format_'+device).append('<option>'+$(this).find("format").text()+'</option>');
             $("#resolutionSlider_"+device).slider({
               value: parseInt($(this).find("default").text()),
               min: parseInt($(this).find("min").text()),
               max: parseInt($(this).find("max").text()),
               step: 50,
               slide: function(event, ui) {
                 $("#resolution_"+device).val( ui.value );
                 $("#resolutionDisplay_"+device).text( ui.value + " dpi" );
               }
             });
             $("#resolution_"+device).val( $(this).find("default").text() );
             $("#resolutionDisplay_"+device).text( $(this).find("default").text() + " dpi" );
             $("#pagesSlider_"+device).slider({
               value: 1,
               min: 1,
               max: 10,
               step: 1,
               slide: function(event, ui) {
                 $("#pages_"+device).val( ui.value );
                 $("#pagesDisplay_"+device).text( ui.value + " pages" );
               }
             });
             $("#scanButton_"+device).click( function() {
               $.ajax({ url: "dynamic",
                        dataType: "xml",
                        data: {action: "doScan", 
                               deviceid: $("#deviceid_"+device).val(),
                               format: $("#format_"+device).val(),
                               pages: $("#pages_"+device).val(),
                               resolution: $("#resolution_"+device).val(),
                               ocr: $("#ocr_"+device).val(),
                               hlp: $("#hlp_"+device).val(),
                               q: $("#q_"+device).val(),
                              },
                        cache: false,
                        type: "POST",
                        success: function(data){
                          scanuuid = $(data).find('scanuuid').text();
                          updateProgressBar(scanuuid, device);
                        }
                      });
             });
           });
         $('#loading').canvasLoaderHalt();
         $('#scanning').hide();
         }
  });

});

