
$(document).ready(function(){

  $('#filterTab').click( function() { 
	$('#filterOptions').slideToggle('slow');
  });

  $('#startDate').datepicker( {dateFormat:'yy-mm-dd'} );
  $('#endDate').datepicker( {dateFormat:'yy-mm-dd'} );
  $('#doFilter').click(function(){
    $.ajax({ url: "dynamic",
             dataType: "xml",
             data: {action: "filter",
                    textSearch: $('#textSearch').val(),
                    startDate: $('#startDate').val(),
                    endDate: $('#endDate').val(),
                   },
             cache: false,
             type: "POST",
             success: function(data){
               if( $(data).find('error').text() ) {
                 alert("An error occured while updating the information.");
               } else {
                 var filteredDocs = new Array();
                 $(data).find('docid').each( function() {
                   filteredDocs.push($(this).text());
                 });
                 $('#docList_table').tablesorterPager({size: allDocs.length, container: $("#pager"), positionFixed: false});
                 for(master in allDocs) {
                   if( $.inArray(allDocs[master], filteredDocs) > -1 ) {
                     // we need to show this doc
                     if ( $.inArray(allDocs[master], showingDocs) == -1 ) {
                       showingDocs.push(allDocs[master]);
                       // move from hidden to the lime lite
                       tr = document.getElementById('docid_'+allDocs[master]);
                       document.getElementById('safeKeeping').getElementsByTagName('tbody')[0].removeChild(tr);
                       document.getElementById('docList_table').getElementsByTagName('tbody')[0].appendChild(tr);
                     }
                   } else {
                     // we need to hide this doc
                     if ( $.inArray(allDocs[master], showingDocs) > -1 ) {
                       showingDocs.splice($.inArray(allDocs[master], showingDocs), 1);
                       // shuffle away for safe keeping
                       tr = document.getElementById('docid_'+allDocs[master]);
                       document.getElementById('docList_table').getElementsByTagName('tbody')[0].removeChild(tr);
                       document.getElementById('safeKeeping').getElementsByTagName('tbody')[0].appendChild(tr);
                     }
                   }
                 }
                 $("#docList_table")
                   .trigger("update")
                   .trigger("appendCache")
              //     .trigger("sorton",[sorting])
                   .tablesorterPager({size: 5, container: $("#pager"), positionFixed: false});
               }
             }
           });

  });
});
