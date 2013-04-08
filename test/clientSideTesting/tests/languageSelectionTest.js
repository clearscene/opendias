(function(){

  var doneload = 0;

  // delete any language cookie that may be present
  var date = new Date();
  date.setTime(date.getTime()-(24*60*60*1000));
  var expires = "; expires="+date.toGMTString();
  document.cookie = "requested_lang="+expires+"; path=/";

  // ================================================
  module("Homepage", {
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
  asyncTest('Page presented in broswers native language', 3, function() {
    console.log("1. Running: native language");

    equal($('#homeLink').text(), "Home", "Was the home link in the expeced language" );
    equal($('#home div p:first').text(), "The openDias architecture has been designed to allow many users to access the system.", "Was the welcome message expeced language" );
    equal(getCookie('requested_lang'), null, "Was the cookie set as expected.");

    start();
  });


  // ------------------------------------------------
  asyncTest('Select Test Lang', 3, function() {
    console.log("1. Running: update language dropdown");

    var langUpdated = 0;
    $q('#testframe').one('load', function() { 
      langUpdated = 1;
      $ = window.frames[0].jQuery; 
      equal($('#homeLink').text(), "####", "Was the home link in the expeced language" );
      equal($('#home div p:first').text(), "### ######## ############ ### #### ######## ## ##### #### ##### ## ###### ### #######", "Was the welcome message updated to the expeced language" );
      equal(getCookie('requested_lang'), 'hh', "Was the cookie set as expected.");
      start();
    });
    $('#language').val('hh').change();
    window.setTimeout( function() {
      if( ! langUpdated ) {
        ok( 0, "Did not load the home page.");
        start();
      }
    }, 5000);
  });


  // ------------------------------------------------
  asyncTest('Select German', 3, function() {
    console.log("1. Running: update language dropdown");

    var langUpdated = 0;
    $q('#testframe').one('load', function() { 
      langUpdated = 1;
      $ = window.frames[0].jQuery; 
      equal($('#homeLink').text(), "Zuhause", "Was the home link in the expeced language" );
      equal($('#home div p:first').text(), "openDias architektur wurde so konzipiert, dass viele Anwender auf das System zuzugreifen.", "Was the welcome message updated to the expeced language" );
      equal(getCookie('requested_lang'), 'de', "Was the cookie set as expected.");
      start();
    });
    $('#language').val('de').change();
    window.setTimeout( function() {
      if( ! langUpdated ) {
        ok( 0, "Did not load the home page.");
        start();
      }
    }, 5000);
  });


  // ------------------------------------------------
  asyncTest('Reset language dropdown', 3, function() {

    var langUpdated = 0;
    $q('#testframe').one('load', function() { 
      langUpdated = 1;
      $ = window.frames[0].jQuery; 
      equal($('#homeLink').text(), "Home", "Was the home link in the expeced language" );
      equal($('#home div p:first').text(), "The openDias architecture has been designed to allow many users to access the system.", "Was the welcome message expeced language" );
      equal(getCookie('requested_lang'), null, "Was the cookie set as expected.");
      start();
    });
    $('#language').val('--').change();
    window.setTimeout( function() {
      if( ! langUpdated ) {
        ok( 0, "Did not load the home page.");
        start();
      }
    }, 5000);
  });

})();
