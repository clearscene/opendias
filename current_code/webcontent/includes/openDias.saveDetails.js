formFields = new Array( 'title', 'actionrequired', 'docDate', 'ocrtext', 'hardcopyKept' );

function sendUpdate(kkey, vvalue) {

    lockForm();
    $.ajax({ url: "/opendias/dynamic",
             dataType: "xml",
             data: {action: "updateDocDetails", 
                    docid: $('#docid').text(),
                    kkey: kkey,
                    vvalue: vvalue,
                   },
             cache: false,
             type: "POST",
             success: function(data){
               if( $(data).find('error').text() ) {
                 alert("An error occured while updating the information.");
                 unlockForm(0);
               } else {
                 unlockForm(1);
               }
             }
           });

}

function moveTag(tag, docid, action) {

    lockForm();
    $.ajax({ url: "/opendias/dynamic",
             dataType: "xml",
             data: {action: "moveTag",
                    subaction: action, 
                    docid: docid,
                    tag: tag,
                   },
             cache: false,
             type: "POST",
             success: function(data){
               if( $(data).find('error').text() ) {
                 alert("An error occured while updating the information.");
                 unlockForm(0);
               } else {
                 unlockForm(1);
               }
             }
           });
}

function lockForm() {
  changeFormState(true);
}
function unlockForm(success) {
  changeFormState(false);
  if(success) {
    $('#saveAlert').show();
    $('#saveAlert').fadeOut(4000);
  }
}
function changeFormState(state) {
  for ( var key in formFields ) {
    if(state) {
      $('#'+formFields[key]).attr('disabled', 'disabled');
    } else {
      $('#'+formFields[key]).removeAttr('disabled');
    }
  }
}
function oncloseEvent() {
  var notComplete = 0;
  var msg = "";
  if(document.getElementById('title').value == "New (untitled) document.") {
    notComplete = 1;
    msg += "Document Title is still the default. ";
  }
  if(document.getElementById('docDate').value == " - No date set -") {
    notComplete = 1;
    msg += "Document Date is still the default. ";
  }
  if(document.getElementById('selected').getElementsByTagName('tr').length == 1) {
    notComplete = 1;
    msg += "Document has not assigned tags. ";
  }

  if(notComplete == 1) {
    return "Document details are incomplete: "+msg;
  }
}

$(document).ready(function() {

  for ( var key in formFields ) {
    $('#'+formFields[key]).change(function(){
      if($(this).is(':checkbox')) {
        sendUpdate( $(this).attr('id'), $(this).is(':checked') );
      } else {
        sendUpdate( $(this).attr('id'), $(this).val() );
      }
    });
  }

  $('#delete').click( function() {
    var answer = confirm("Delete this document. Are you sure?");
    if (answer){
      $.ajax({ url: "/opendias/dynamic",
             dataType: "xml",
             data: {action: "deleteDoc", 
                    docid: $('#docid').text(),
                   },
             cache: false,
             type: "POST",
             success: function(data){
               if( $(data).find('error').text() ) {
                 alert("An error occured deleting the document.");
                 unlockForm(0);
               } else {
                 document.location.href = "/opendias/";
               }
             },
      });
    }
  });

});

window.onbeforeunload = oncloseEvent;
