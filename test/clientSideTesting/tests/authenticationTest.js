(function(){

  var doneload = 0;

  // delete any language cookie that may be present
  var date = new Date();
  date.setTime(date.getTime()-(24*60*60*1000));
  var expires = "; expires="+date.toGMTString();
  document.cookie = "requested_lang="+expires+"; path=/";
  document.cookie = "role="+expires+"; path=/";
  document.cookie = "realname="+expires+"; path=/";
  document.cookie = "o_session_id="+expires+"; path=/";
  document.cookie = "sortOrder="+expires+"; path=/";

  // ================================================
  module("Authentication", {
      setup : function(){
        if( ! doneload ) {
          stop();
          $q('#testframe').one('load', function() { 
            $ = window.frames[0].jQuery; 
            doneload = 1;
            start();
          });
          $q('#testframe').attr('src', "/opendias/");
          window.setTimeout( function() {
            if( ! doneload ) {
              ok( 0, "Did not load the home page.");
            }
          }, 5000);
        }
      },
      teardown : function(){ }
  });

  // ------------------------------------------------
  asyncTest('Correct Links available', 9, function() {
    console.log("1. Homepage links while logged out");

    ok( $('#homeLink').is(':visible'), "Was the home link visible" );
    ok( ! $('#doclistLink').is(':visible'), "Was the doc list link hidden" );
    ok( ! $('#acquireLink').is(':visible'), "Was the acquire link hidden" );
    ok( $('#ProjectHome').is(':visible'), "Was the project link visible" );
    ok( $('#username').is(':visible'), "Was the username box visible" );
    ok( $('#password').is(':visible'), "Was the password box visible" );
    ok( $('#loginbutton').is(':visible'), "Was the login button visible" );
    ok( ! $('#realname').is(':visible'), "Was the username area hidden " );
    ok( ! $('#logoutbutton').is(':visible'), "Was the logout button hidden" );

    start();
  });


  // ------------------------------------------------
  asyncTest('Bad Login', 2, function() {
    console.log("2. Attempt bad logins.");

    $('#username').val("admin").change();
    $('#password').val("admin").change();
    $('#loginbutton').click();

    window.setTimeout( function() {
      ok( $('#loginbutton').is(':visible'), "Was the login button visible" );
      ok( ! $('#logoutbutton').is(':visible'), "Was the logout button hidden" );
      start();
    }, 5000);

    start();
  });



  // ------------------------------------------------
  asyncTest('Good Login', 7, function() {
    console.log("2. Attempt good logins.");

    $('#username').val("admin").change();
    $('#password').val("admin").change();
    $('#loginbutton').click();

    window.setTimeout( function() {
      ok( $('#doclistLink').is(':visible'), "Was the doc list link visible" );
      ok( $('#acquireLink').is(':visible'), "Was the acquire link visible" );
      ok( ! $('#username').is(':visible'), "Was the username box hidden" );
      ok( ! $('#password').is(':visible'), "Was the password box hidden" );
      ok( ! $('#loginbutton').is(':visible'), "Was the login button hidden" );
      ok( $('#realname').is(':visible'), "Was the username area visible" );
      ok( $('#logoutbutton').is(':visible'), "Was the logout button visible" );
      start();
    }, 5000);

    start();
  });
})();
