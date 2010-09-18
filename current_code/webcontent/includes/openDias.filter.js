
$('#filterTab').click( function() { 
	$('#filterOptions').slideToggle('slow');
} );

$('#startDate').datepicker( {dateFormat:"yy/mm/dd"} );
$('#endDate').datepicker( {dateFormat:"yy/mm/dd"} );
$('#doFilter').click(function(){
    $.ajax({ url: "dynamic",
             dataType: "xml",
             data: {action: "filter",
                    textSearch: $('#textSearch').text(),
                    startDate: $('#startDate').text(),
                    endDate: $('#endDate').text(),
                   },
             cache: false,
             type: "POST",
             success: function(data){
               if( $(data).find('error').text() ) {
                 alert("An error occured while updating the information.");
               } else {
                 var filteredDocs = new Array
                 $(data).find('docid').each( function(dte) {
                   filterDocs.push(dte.text());
                 });
                 for(master in allDocs) {
                   
                 }
                 $("#docList_table")
                   .trigger("update")
                   .trigger("appendCache")
                   .trigger("sorton",[sorting])
                   .tablesorterPager({size: 5, container: $("#pager"), positionFixed: false});
               }
             }
           });

});
