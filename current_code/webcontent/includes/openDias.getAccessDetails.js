
function applyLocationRow(location, role) {
  applyNewRow('location', "Anyone accessing from ", location, role);
}

function applyUserRow(user, role) {
  applyNewRow('user', "User with logon of ", user, role);
}

function applyNewRow(type, human, prop, role) {

  var tr = document.createElement("tr");
  var id = document.createAttribute('id');
  //tr.setAttribute('id','tagid_'+tagid);

  var strong1 = document.createElement("strong");
  strong1.appendChild(document.createTextNode(prop));
  var prop_td = document.createElement("td");
  prop_td.appendChild(document.createTextNode(human));
  prop_td.appendChild(strong1);

  var strong2 = document.createElement("strong");
  strong2.appendChild(document.createTextNode(role));
  var role_td = document.createElement("td");
  role_td.appendChild(document.createTextNode( LOCAL_will_get_the ));
  role_td.appendChild(strong2);
  role_td.appendChild(document.createTextNode( LOCAL_role ));

  var button = document.createElement("button");
  button.appendChild(document.createTextNode( LOCAL_delete ));
  var dele_td = document.createElement("td");
  dele_td.appendChild(button);

  tr.appendChild(prop_td);
  tr.appendChild(role_td);
  tr.appendChild(dele_td);

  document.getElementById(type+'Table').getElementsByTagName('tbody')[0].appendChild(tr);
//  $('#tagid_'+tagid).one('dblclick', function() {
//    moveTag( $(this).attr('id'), 'add' );
//  });

}

$(document).ready(function() {

  $.ajax({ url: "/opendias/dynamic",
         dataType: "xml",
         timeout: 10000,
         data: {action: "getAccessDetails"},
         cache: false,
         type: "POST",
         success: function(data){
           if( $(data).find('error').text() ){
             alert( LOCAL_unable_to_get_access_info + ": " + $(data).find('error').text());
             return 1;
           }
           $(data).find('AccessDetails').find('LocationAccess').find('Access').each( function() {
               applyLocationRow($(this).find("location").text(),
                                $(this).find("role").text()
                                );
           });
           $(data).find('AccessDetails').find('UserAccess').find('Access').each( function() {
               applyUserRow(    $(this).find("user").text(),
                                $(this).find("role").text()
                                );
           });
         },
         error: function( x, t, m ) {
           if(t=="timeout") {
             alert("[c001] " + LOCAL_timeout_talking_to_server);
           } else {
             alert("[c001] " + LOCAL_error_talking_to_server + ": " + t+"\n"+m);
           }
         },
  });

});

