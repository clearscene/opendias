$(document).ready(function() {

  // Set the language dropdown to the current value
  var currentLang = getCookie("requested_lang");
  if( currentLang != null ) {
    $('#language').val(currentLang);
  }

  // Handle an update to the language setting
  $('#language').change( function() {
    var setting = $('#language').val();
    if( setting == '--' ) {
      deleteCookie("requested_lang", null);
    }
    else {
      setCookie("requested_lang", setting);
    }
    window.location.reload(true);
  });

  setLoginOutArea();

  $('#loginbutton').click( function(){ 
    $('#loginbutton').attr("disabled", true);
    $.ajax({ url: "/opendias/dynamic",
             dataType: "xml",
             timeout: 10000,
             data: {action: "checkLogin",
                    username: $('#username').val(),
                    password: $('#password').val(),
                   },
             cache: false,
             type: "POST",
             error: function( x, t, m ) {
               $('#password').val('');
               if(t=="timeout") {
                 alert("[s001] " + LOCAL_timeout_talking_to_server);
               } else {
                 alert("[s001] " + LOCAL_error_talking_to_server+": "+t+"\n"+m);
               }
             },
             success: function(data){
               $('#password').val('');
               if( $(data).find('error').text() ) {
                 alert( $(data).find('error').text() );
               } else {
                if( $(data).find('result').text()=='OK' ) {
                  $('#loginbutton').attr("disabled", false);
                  setLoginOutArea();
                }
                else {
                  $('#loginbutton').css({ display: 'none' });
                  setTimeout( function() {
                    $('#loginbutton').attr("disabled", false);
                    $('#loginbutton').css({ display: 'inline' });
                  }, parseInt( $(data).find('retry_throttle').text() ) * 1000 );
                  alert( $(data).find('message').text() );
               }
             }
           }
        });
  });


  $('#logoutbutton').click( function(){ 
    $('#password').val('');
    $.ajax({ url: "/opendias/dynamic",
             dataType: "xml",
             timeout: 10000,
             data: { action: "logout" },
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
                 deleteCookie("realname", null);
                 setLoginOutArea();
               }
             }
           });
    });
});


function setLoginOutArea() {
  // Display or not, the login area
  // If we have a cookie "realname=<anything>"
  // then show the logout only. Otherwise
  // show the login area.
  var realname = getCookie("realname");
  if( realname == null || realname == "" ) {
    $('#logout').css({ display: 'none' });
    $('#login').css({ display: 'block' });
  }
  else {
    $('#realname').html( realname );
    $('#login').css({ display: 'none' });
    $('#logout').css({ display: 'block' });
  }

}

function setCookie(name,value,days) {
    if (days) {
        var date = new Date();
        date.setTime(date.getTime()+(days*24*60*60*1000));
        var expires = "; expires="+date.toGMTString();
    }
    else var expires = "";
    document.cookie = name+"="+value+expires+"; path=/";
}

function getCookie(name) {
    var nameEQ = name + "=";
    var ca = document.cookie.split(';');
    for(var i=0;i < ca.length;i++) {
        var c = ca[i];
        while (c.charAt(0)==' ') c = c.substring(1,c.length);
        if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
    }
    return null;
}

function deleteCookie(name) {
    setCookie(name,"",-1);
}

