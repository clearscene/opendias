$(document).ready(function() {

  $("#tabs").tabs();

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
           foreach device ($(data).find('devices').foreach()) {
             
           }
         }
  });

});

