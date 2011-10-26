var filteredDocs;
var master;

$(document).ready(function(){

  $('#filterTab').click( function() { 
	$('#filterOptions').slideToggle('slow');
  });

  $('#startDate').datepicker( {dateFormat:'yy-mm-dd'} );
  $('#endDate').datepicker( {dateFormat:'yy-mm-dd'} );

  $('#doFilter').click(function(){
    document.getElementById('docList_table').getElementsByTagName('tbody')[0].innerHTML = "";
    loadListData();
//    $.ajax({ url: "/opendias/dynamic",
//             dataType: "xml",
//             data: {action: "docFilter",
//                    subaction: "fullList",
//                    textSearch: $('#textSearch').val(),
//                    startDate: $('#startDate').val(),
//                    endDate: $('#endDate').val(),
//                    tags: $('#tags').val()
//                   },
//             cache: false,
//             type: "POST",
//             success: function(data){
//               if( $(data).find('error').text() ) {
//                 alert("Unable to get a filtered list:."+$(data).find('error').text());
//               } else {
//                 $('#docList_table').fadeOut("fast");
//                 filteredDocs = new Array();
//                 $(data).find('DocFilter').find('Results').find('docid').each( function() {
//                   filteredDocs.push($(this).text());
//                 });
//                 master = 0;
//                 applyMoveRow();
//               }
//             }
//           });

  });

  $('#textSearch').keypress( function() { getRecordCount() } );
  $('#textSearch').change( function() { getRecordCount() } );
  $('#startDate').change( function() { getRecordCount() } );
  $('#endDate').change( function() { getRecordCount() } );
  //$('#tags').change( function() { getRecordCount() } );

});

function getRecordCount() {
    $.ajax({ url: "/opendias/dynamic",
             dataType: "xml",
             data: {action: "docFilter",
                    subaction: "count",
                    textSearch: $('#textSearch').val(),
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

function applyMoveRow() {
  if(master >= allDocs.length) {
     $("#filterProgress").progressbar( "destroy" );
     $("#docList_table").trigger("rebuild");
     $('#docList_table').fadeIn();
  } else {
     $("#filterProgress").progressbar({ value: parseInt(100*(master/allDocs.length)) });
     if( $.inArray(allDocs[master], filteredDocs) > -1 ) {
       // we need to show this doc
       if ( $.inArray(allDocs[master], showingDocs) == -1 ) {
         showingDocs.push(allDocs[master]);
         // move from hidden to the lime lite
         tr = document.getElementById('docid_'+allDocs[master]);
//         tr = safeKeeping['docid_'+allDocs[master]];
         $("#docList_table").trigger("addRow",tr.cloneNode(true));
         document.getElementById('safeKeeping').getElementsByTagName('tbody')[0].removeChild(tr);
       }
     } else {
       // we need to hide this doc
       if ( $.inArray(allDocs[master], showingDocs) > -1 ) {
         showingDocs.splice($.inArray(allDocs[master], showingDocs), 1);
         // shuffle away for safe keeping
         $("#docList_table").trigger("removeRow",'docid_'+allDocs[master]);
       }
     }
     master++;
     setTimeout("applyMoveRow()", 1);
  }
}
