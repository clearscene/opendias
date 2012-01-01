var filteredDocs;
var master;

$(document).ready(function(){

  $('#filterTab').click( function() { 
	$('#filterOptions').slideToggle('slow');
  });

  $('#startDate').datepicker( {dateFormat:'yy-mm-dd'} );
  $('#endDate').datepicker( {dateFormat:'yy-mm-dd'} );

  $('#doFilter').click(function(){
    reloadResults();
  });

  $('#textSearch').keypress( function() { getRecordCount() } );
  $('#textSearch').change( function() { getRecordCount() } );
  $('#isactionrequired').change( function() { getRecordCount() } );
  $('#startDate').change( function() { getRecordCount() } );
  $('#endDate').change( function() { getRecordCount() } );

});

function getRecordCount() {
    $.ajax({ url: "/opendias/dynamic",
             dataType: "xml",
             data: {action: "docFilter",
                    subaction: "count",
                    textSearch: $('#textSearch').val(),
                    actionrequired: $('#isactionrequired').val(),
                    startDate: $('#startDate').val(),
                    endDate: $('#endDate').val(),
                    tags: $('#tags').val()
                   },
             cache: false,
             type: "POST",
             success: function(data){
               if( $(data).find('error').text() ) {
                 alert("Unable to get a filtered list:."+$(data).find('error').text());
               } else {
                 $('#filterProgress').text( "Will return an estimated "+$(data).find('DocFilter').find('count').text()+" docs" );
               }
             }
           });
}

