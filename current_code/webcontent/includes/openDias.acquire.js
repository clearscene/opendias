var baron = 0;
var canon = 0;
var PROGRESS_REFRESH_TIME = 400;
var doneAtLeastOnePage = 0;

function showStatus(dev, canv, prog) {
  if(canv == 1 && canon == 0) {
    $("#progressbar_"+dev).canvasLoader({'radius':20, 'dotRadius':2});
    canon=1;
  } else if(canv == undefined && canon == 1) {
    $("#progressbar_"+dev).canvasLoaderHalt();
    canon=0;
  }
  if(prog == undefined && baron == 1) {
    $("#progressbar_"+dev).progressbar( "destroy" );
    baron=0;
  } else if( prog != undefined ) {
    baron=1;
    $("#progressbar_"+dev).progressbar({
      value: prog,
    });
  }
}


function getScanningProgress (progressId, device) {

  var action = 'refresh';

  $.ajax({ url: "/opendias/dynamic",
	 dataType: "xml",
   timeout: 10000,
	 data: {action: "getScanningProgress", 
          scanprogressid: progressId,
	       },
	 cache: false,
	 type: "POST",
	 success: function(dta) {
           if( $(dta).find('error').text() ){
             alert(LOCAL_error_getting_scan_progress+": "+$(dta).find('error').text());
             return 1;
           }

           status = parseInt( $(dta).find('ScanningProgress').find('status').text() );
           vvalue = parseInt( $(dta).find('ScanningProgress').find('value').text() );

           if( status == 0 ) { // SCAN_IDLE,
             $('#status_'+device).text( LOCAL_setting_up );
             showStatus(device, undefined, undefined);
             // Give us a chance ....

           } else if( status == 1 ) { // SCAN_INTERNAL_ERROR,
             $('#status_'+device).text( LOCAL_internal_error );
             showStatus(device, undefined, undefined);
             alert( LOCAL_internal_error + ": " + vvalue);
             action='finish';

           } else if( status == 2 ) { // SCAN_DB_WORKING,
             $('#status_'+device).text( LOCAL_waiting_on_the_database );
             showStatus(device, undefined, undefined);
             // Give us a chance ....

           } else if( status == 5 ) { // SCAN_DB_ERROR // DB error code
             showStatus(device, undefined, undefined);
             $('#status_'+device).text( LOCAL_error_while_scanning );
             alert( LOCAL_error_while_scanning + ": " + vvalue);
             action='finish';

           } else if( status == 4 ) { // SCAN_WAITING_ON_SCANNER,
             showStatus(device, undefined, undefined);
             $('#status_'+device).text( LOCAL_setting_up_the_scanner );
             // Give us a chance ....

           } else if( status == 5 ) { // SCAN_ERRO_FROM_SCANNER,// SANE error code
             showStatus(device, undefined, undefined);
             $('#status_'+device).text( LOCAL_error_while_scanning );
             alert( LOCAL_error_while_scanning + ": " + vvalue);
             action='finish';

           } else if( status == 6 ) { // SCAN_SCANNING,// Current progress
             $('#status_'+device).text( LOCAL_scanning_in_progress );
             showStatus(device, undefined, vvalue);
             $("#progressbar_"+device).progressbar({
               value: vvalue,
             });

           } else if( status == 7 ) { // SCAN_WAITING_ON_NEW_PAGE,// Waiting for page [x]
             showStatus(device, undefined, undefined);
             $('#status_'+device).text( LOCAL_please_insert_page + ": "+vvalue+".");
             if(confirm( LOCAL_please_insert_page + ": "+vvalue+".")) {
               action='postnewpage';
             } else {
               action='finish';
             }

           } else if( status == 8 ) { // SCAN_TIMEOUT_WAITING_ON_NEW_PAGE,
             showStatus(device, undefined, undefined);
             $('#status_'+device).text( LOCAL_timeout_waiting_for_next_page );
             alert( LOCAL_timeout_waiting_for_next_page );
             action='finish';

           } else if( status == 9 ) { // SCAN_CONVERTING_FORMAT,
             $('#status_'+device).text( LOCAL_converting_image_format );
             showStatus(device, 1, undefined);

           } else if( status == 10 ) { // SCAN_ERROR_CONVERTING_FORMAT,// FreeImage error code
             showStatus(device, undefined, undefined);
             $('#status_'+device).text( LOCAL_error_converting_image_format );
             alert( LOCAL_error_converting_image_format + ": " + vvalue);
             action='finish';

           } else if( status == 11 ) { // SCAN_PERFORMING_OCR,
             showStatus(device, 1, undefined);
             $('#status_'+device).text( LOCAL_performing_ocr_on_image );

           } else if( status == 12 ) { // SCAN_ERROR_PERFORMING_OCR,// xxxxxx error code
             showStatus(device, undefined, undefined);
             $('#status_'+device).text( LOCAL_error_while_performing_ocr );
             alert( LOCAL_error_while_performing_ocr + ": " + vvalue);
             action='finish';

           } else if( status == 13 ) { // SCAN_SANE_BUSY
             // Put everything back like it was, ready for a new attempt.
             showStatus(device, undefined, undefined);
             $("#format_"+device).removeAttr('disabled');
             $("#pagesSlider_"+device).slider({ disabled: false });
             $("#resolutionSlider_"+device).slider({ disabled: false });
             $("#ocr_"+device).removeAttr('disabled');
             $("#lengthSlider_"+device).slider({ disabled: false });
             $("#scanButton_"+device).removeAttr('disabled');
             $("#resolutionGood_"+device).parent().removeClass("greyResolution");
             $("#resolutionGood_"+device).removeClass("greySweetResolution");
             $("#resolutionGood_"+device).addClass("sweetResolution");
             $('#status_'+device).text( LOCAL_try_again_in_a_minute );
             alert( LOCAL_sane_is_busy );
             action='finish';
             if( doneAtLeastOnePage == 1 ) {
               action=='postnewpage';
             }

           } else if( status == 14 ) { // SCAN_RESERVED_1,
           } else if( status == 15 ) { // SCAN_RESERVED_2,
           } else if( status == 16 ) { // SCAN_FINISHED
             showStatus(device, undefined, undefined);
             $('#status_'+device).text( LOCAL_scan_complete );
             document.location.href = "/opendias/docDetail.html?docid="+vvalue;
             action='finish';

           }
         },
         error: function( x, t, m ) {
           if(t=="timeout") {
             alert("[a001] " + LOCAL_timeout_talking_to_server);
           } else {
             alert("[a002] " + LOCAL_error_talking_to_server + ": "+t);
           }
         },
         complete: function() {
           if( action=='postnewpage' ) {
               doneAtLeastOnePage = 1;
               $.ajax({ url: "/opendias/dynamic",
                      timeout: 10000,
                      dataType: "xml",
                      data: {action: "nextPageReady",
                             scanprogressid: progressId,
                             },
                      cache: false,
                      type: "POST",
                      success: function(dta2) {
                        if( $(dta2).find('error').text() ){
                          alert( LOCAL_error_starting_next_page + ": "+$(dta2).find('error').text());
                          return 1;
                        }
                      },
                      error: function( x, t, m ) {
                        if(t=="timeout") {
                          alert("[a003] " + LOCAL_timeout_talking_to_server );
                        } else {
                          alert("[a004] " + LOCAL_error_talking_to_server + ": "+t);
                        }
                      },
                    });
                getScanningProgress(progressId,device);

           } else if( action=='refresh' ) {
             setTimeout("getScanningProgress('"+progressId+"','"+device+"')", PROGRESS_REFRESH_TIME);
           } else {
             // do nothinog - just drop off and finish
           }
         },
       });
}


$(document).ready(function() {

  $("#tabs").tabs();

  $('#loading').canvasLoader({'radius':20, 'dotRadius':2});

  $.ajax({ url: "/opendias/dynamic",
         dataType: "xml",
         timeout: 10000,
         data: {action: "getScannerList"},
         cache: false,
         type: "POST",
         success: function(data){
           if( $(data).find('error').text() ){
             $('#loading').canvasLoaderHalt();
             $('#scanning').text( LOCAL_failed_to_get_list_of_scanners + ": "+$(data).find('error').text());
             return 1;
           }
           var deviceid=0;
           if( $(data).find('ScannerList').attr("cached") ) {
             $('#cached').show();
           }

           $(data).find('ScannerList').find('Devices').find('Device').each( function() {

             deviceid++;
             var device=deviceid;

             // Create scan doc
             var div = document.createElement("div");
             div.setAttribute('id','deviceTab_'+device);
             var newTabHtml = document.getElementById('scannerTemplate').innerHTML;
             idchange = new Array('title', 'deviceid', 'format', 'pages', 'pagesSlider', 'resolution', 
                        'resolutionSlider', 'ocr', 'progressbar', 'resolutionDisplay', 'pagesDisplay', 
                        'scanButton', 'status', 'resolutionGood', 'length', 'lengthDisplay', 'lengthSlider');
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
             var host = "";
             if( $(this).find("host").text() != "" ) {
               host = " (on host '" + $(this).find("host").text() + "')";
             }
             $('#title_'+device).text( $(this).find("type").text() + ": " +
                                      $(this).find("vendor").text() + " - " +
                                      $(this).find("model").text() + host);
             $('#deviceid_'+device).val( $(this).find("name").text() );
             //$('#format_'+device).append('<option>'+$(this).find("format").text()+'</option>');
             $("#resolutionSlider_"+device).slider({
               range: "min",
               value: parseInt($(this).find("default").text()),
               min: parseInt($(this).find("min").text()),
               max: parseInt($(this).find("max").text()),
               step: 50,
               slide: function(event, ui) {
                 $("#resolution_"+device).val( ui.value );
                 $("#resolutionDisplay_"+device).text( ui.value + " dpi" );
                 if(ui.value >= bestLow && ui.value <= bestHigh) {
                   $("#resolutionGood_"+device).addClass("sweetResolution");
                   $("#resolutionGood_"+device).parent().removeClass("poorResolution");
                   $("#ocr_"+device).removeAttr('disabled');
                 } else {
                   $("#resolutionGood_"+device).parent().addClass("poorResolution");
                   $("#resolutionGood_"+device).removeClass("sweetResolution");
                   $("#ocr_"+device).attr('disabled', 'disabled');
                   $("#ocr_"+device).removeAttr('checked');
                 }
               }
             });
             var bestLow = 300;
             var bestHigh = 400;
             var resFactor = 215 / (parseInt($(this).find("max").text()) - parseInt($(this).find("min").text()) );
             $("#resolutionGood_"+device).css( { 'left': resFactor * (bestLow - parseInt($(this).find("min").text()) ),
                                                 'width': (bestHigh - bestLow) * resFactor*1.05 } );
             $("#resolution_"+device).val( $(this).find("default").text() );
             $("#resolutionDisplay_"+device).text( $(this).find("default").text() + " dpi" );
             if(parseInt($(this).find("default").text()) >= bestLow && parseInt($(this).find("default").text()) <= bestHigh) {
               $("#resolutionGood_"+device).addClass("sweetResolution");
               $("#ocr_"+device).removeAttr('disabled');
             } else {
               $("#resolutionGood_"+device).parent().addClass("poorResolution");
               $("#ocr_"+device).attr('disabled', 'disabled');
             }

             $("#lengthSlider_"+device).slider({
               range: "min",
               value: 100,
               min: 20,
               max: 100,
               slide: function(event, ui) {
                 $("#length_"+device).val( ui.value );
                 $("#lengthDisplay_"+device).text( ui.value + " %" );
               }
             });
             $("#pagesSlider_"+device).slider({
               range: "min",
               value: 1,
               min: 1,
               max: 10,
               slide: function(event, ui) {
                 $("#pages_"+device).val( ui.value );
                 $("#pagesDisplay_"+device).text( ui.value + " pages" );
               }
             });
             $("#scanButton_"+device).click( function() {
               // Stop the form from being changed after submittion
               $("#format_"+device).attr('disabled', 'disabled');
               $("#pagesSlider_"+device).slider('disable');
               $("#resolutionSlider_"+device).slider('disable');
               $("#ocr_"+device).attr('disabled', 'disabled');
               $("#lengthSlider_"+device).slider('disable');
               $("#scanButton_"+device).attr('disabled', 'disabled');
               $("#resolutionGood_"+device).parent().addClass("greyResolution");
               $("#resolutionGood_"+device).removeClass("sweetResolution");
               $("#resolutionGood_"+device).addClass("greySweetResolution");
               // Added something above - then also add it to recovery on SCAN_SANE_BUSY code

               $.ajax({ url: "/opendias/dynamic",
                        dataType: "xml",
                        timeout: 10000,
                        data: {action: "doScan", 
                               deviceid: $("#deviceid_"+device).val(),
                               format: $("#format_"+device).val(),
                               pages: $("#pages_"+device).val(),
                               resolution: $("#resolution_"+device).val(),
                               ocr: $("#ocr_"+device).val(),
                               pagelength: $("#length_"+device).val(),
                              },
                        cache: false,
                        type: "POST",
                        success: function(data){
                          if( $(data).find('error').text() ){
                            alert( LOCAL_unable_to_start_scanning + ": "+$(data).find('error').text());
                            return 1;
                          }
                          scanuuid = $(data).find('DoScan').find('scanuuid').text();
                          getScanningProgress(scanuuid, device);
                        },
                        error: function( x, t, m ) {
                          if(t=="timeout") {
                            alert("[a005] " + LOCAL_timeout_talking_to_server );
                          } else {
                            alert("[a006] " + LOCAL_error_talking_to_server + ": "+t);
                          }
                        },
                      });
             });
           });
         $('#loading').canvasLoaderHalt();
         $('#scanning').hide();
         },
         error: function( x, t, m ) {
           if(t=="timeout") {
             alert("[a007] " + LOCAL_timeout_talking_to_server );
           } else {
             alert("[a008] " + LOCAL_error_talking_to_server + ": "+t);
           }
         },
  });

});

