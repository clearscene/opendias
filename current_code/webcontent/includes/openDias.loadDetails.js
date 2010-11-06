var sorting = [[1,0]]; // sort by date desc

// Read a page's GET URL variables and return them as an associative array.
function getUrlVars() {
    var vars = [], hash;
    var hashes = window.location.href.slice(window.location.href.indexOf('?') + 1).split('&');
    for(var i = 0; i < hashes.length; i++) {
        hash = hashes[i].split('=');
        vars.push(hash[0]);
        vars[hash[0]] = hash[1];
    }
    return vars;
}

function applyNewRow(tagid, tag, selected) {

  var tr = document.createElement("tr");
  var id = document.createAttribute('id');
  tr.setAttribute('id','tagid_'+tagid);
  var e_tag = document.createElement("td");
  e_tag.appendChild(document.createTextNode(tag));
  tr.appendChild(e_tag);

  if(selected=="NULL") {
    document.getElementById('available').getElementsByTagName('tbody')[0].appendChild(tr);
    $('#tagid_'+tagid).one('dblclick', function() {
      moveTag( $(this).attr('id'), 'add' );
    });
  } else {
    document.getElementById('selected').getElementsByTagName('tbody')[0].appendChild(tr);
    $('#tagid_'+tagid).one('dblclick', function() {
      moveTag( $(this).attr('id'), 'remove' );
    });
  }

}

$(document).ready(function() {

  $('#playAudioLink').click(function(){
                               $.ajax({ url: "dynamic",
                                        dataType: "xml",
                                        data: {action: "getAudio", docid: getUrlVars()['docid']},
                                        cache: false,
                                        type: "POST",
                                        success: function(data){
                                                  if($(data).find('error').text()) {
                                                    alert("Unable to get the audio: "+$(data).find('error').text());
                                                    return 1;
                                                  }
                                                 id = $(data).find('filename').text();
                                                 $('#audio').attr('src','/audio/'+id);
                                                 }
                                        });
                               $('#playAudio').toggle();
                               });

  $.ajax({ url: "dynamic",
         dataType: "xml",
         data: {action: "getDocDetail", docid: getUrlVars()['docid']},
         cache: false,
         type: "POST",
         success: function(data){
           if( $(data).find('error').text() ){
             alert("Unable to get document details: "+$(data).find('error').text());
             return 1;
           }
           officialDocId = $(data).find('docid').text();
           $("#docid").text( officialDocId );
           $("#title").val( $(data).find('title').text() );
           $("#ocrtext").val( $(data).find('extractedText').text() );
           $("#docDate").val( $(data).find('docDate').text() );
           $("#scanDate").append(document.createTextNode( $(data).find('scanDate').text() ));
           $("#type").append(document.createTextNode( $(data).find('type').text() ));

           // Set images and default width
           for( x=1 ; x<=parseInt($(data).find('pages').text()) ; x++ ) {
             $("#slider ul").append("<li><div class='scanImageContainer zoom'><img id='scanImage"+x+"' alt='' src='/scans/"+officialDocId+"_"+x+".jpg' /></div></li>");
             $("#scanImage"+x).css("width", "300px");
           }

           // setup the slider
           $("#slider li").css("height", 30+($(data).find('y').text() * ( 300 / $(data).find('x').text() ))+"px" );
           if($(data).find('pages').text() != "1") {
             $("#slider").easySlider({prevText:'', nextText:''});
           }

           // make eachimage zoomable
           for( x=1 ; x<=parseInt($(data).find('pages').text()) ; x++ ) {
             $("#scanImage"+x).parent().gzoom({
                                 sW: 300,
                                 sH: $(data).find('y').text() * ( 300 / $(data).find('x').text() ),
                                 lW: $(data).find('x').text(),
                                 lH: $(data).find('y').text(), 
                                 lighbox: false
                                 });
           }

           $("#docDate").datepicker( {dateFormat:"yy/mm/dd"} );

           $(data).find('tags').find('tag').each( function() {
               applyNewRow( $(this).find("tagid").text(),
                            $(this).find("tagname").text(),
                            $(this).find("selected").text()
                            );
           });

           $("#available")
                 .trigger("update")
                 .trigger("appendCache")
                 .trigger("sorton",[sorting]);
           $("#selected")
                 .trigger("update")
                 .trigger("appendCache")
                 .trigger("sorton",[sorting]);

         }
  });

  $('#title').autocomplete({
      source: function( request, response ) {
        $.ajax({
          url: "/dynamic",
          dataType: "json",
          type: "POST",
          data: {
            action: "titleAutoComplete",
            startsWith: request.term
          },
          success: function( data ) {
            response( $.map( data.results, function( item ) {
              return {
                label: item.title,
                value: item.title
              }
            }));
          }
        });
      },
      minLength: 2,
      select: function( event, ui ) {
      //  log( ui.item ?
      //    "Selected: " + ui.item.label :
      //    "Nothing selected, input was " + this.value);
      },
      open: function() {
        $( this ).removeClass( "ui-corner-all" ).addClass( "ui-corner-top" );
      },
      close: function() {
        $( this ).removeClass( "ui-corner-top" ).addClass( "ui-corner-all" );
      }
    });

});

