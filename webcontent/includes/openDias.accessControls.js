$(document).ready(function() {
  var role = getCookie("role");

  $("#newrealname").val( getCookie("realname") );
  $("#currentrole").html( get_priv_from_role( role, 'name' ) + " (" + role + ")" );
  if ( get_priv_from_role( role, 'update_access' ) ) {
    $('.onlyadmin').css({ display: 'block' });
  }

  $("#tabs").tabs();

  $('#updateThisUser').click( function(){
    if( $('#newpassword').val() != $('#newpassword2').val() ) {
      alert( LOCAL_new_password_do_not_match );
      return 0;
    }
    $.ajax({ url: "/opendias/dynamic",
             dataType: "xml",
             timeout: 10000,
             data: { action: "updateUser",
                     username: "[current]",
                     realname: $("#newrealname").val(),
                     password: $("#newpassword").val(),
                     },
             cache: false,
             type: "POST",
             error: function( x, t, m ) {
               if(t=="timeout") {
                 alert("[s001] " + LOCAL_timeout_talking_to_server);
               } else {
                 alert("[s001] " + LOCAL_error_talking_to_server+": "+t+"\n"+m);
               }
             },
             success: function(data){
               if( $(data).find('error').text() ) {
                 alert( $(data).find('error').text() );
               } else {
                 setCookie("realname", $('#newrealname').val() );
                 setLoginOutArea();
                 alert( LOCAL_details_updated );
               }
             }
           });
    });

  $('#createNewUser').click( function(){
    if( $('#password1').val() != $('#password2').val() ) {
      alert( LOCAL_new_password_do_not_match );
      return 0;
    }
    $.ajax({ url: "/opendias/dynamic",
             dataType: "xml",
             timeout: 10000,
             data: { action: "createUser",
                     username: $("#newuserid").val(),
                     realname: $("#newrealname").val(),
                     password: $("#password").val(),
                     role: $("#newuser_role").val(),
                     },
             cache: false,
             type: "POST",
             error: function( x, t, m ) {
               if(t=="timeout") {
                 alert("[s001] " + LOCAL_timeout_talking_to_server);
               } else {
                 alert("[s001] " + LOCAL_error_talking_to_server+": "+t+"\n"+m);
               }
             },
             success: function(data){
               if( $(data).find('error').text() ) {
                 alert( $(data).find('error').text() );
               }
             }
           });
    });

});
