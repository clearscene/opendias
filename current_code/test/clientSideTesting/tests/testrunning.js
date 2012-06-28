// On DOM Ready
$q(function() {

  QUnit.config.reorder = false;
  $q.ajaxSetup({async:false});

  // Attempt to load the testswarm inject include (if available)
  $q.getScript("http://testswarmhost/testswarm/js/inject.js");

  var tests = [
              'listPageTest',
              'acquirePageTest',
              'homePageTest',
              ];
  var runners = [];

  // Load all the test cases;
  $q.each(tests, function(i,v) {
    console.log('loading: /opendias/includes/tests/'+v+'.js');
    $q.getScript('/opendias/includes/tests/'+v+'.js')
    .done(function(script, textStatus) {
      console.log('loaded: /opendias/includes/tests/'+v+'.js');
      runners.push(v);
    })
    .fail(function(jqxhr, settings, exception){ 
      alert("failed to loaded test '" + v + "', because: " + exception);
    });
  });
  $q.ajaxSetup({async:true});

});

// ====================================

var delay = 100;

function setupWaitForValue( element, expected ) {
  if(expected == null) {
    window.waitForValue_original = element.is(':visible');
  } 
  else {
    window.waitForValue_original = element.text();
  }
  window.waitForValue_expected = expected;
}

function waitForValue( element, timeout) {
  console.log( "waitForValue called with a timeout of "+timeout );

  if ( window.waitForValue_expected == null ) {
    if ( element.is(':visible') != window.waitForValue_original) {
      ok( 1, element.attr('id') + " has changed visibility" );
      start();
      return;
    }
  }

  else {
    if (element.text() != window.waitForValue_original) {
      equal( element.text(), window.waitForValue_expected, element.attr('id') + " was expected to be " + window.waitForValue_expected );
      start();
      return;
    }
  }

  timeout = timeout - delay;
  if( timeout > 0 ) {
    window.setTimeout( function() { waitForValue(element, timeout) }, delay);
  }
  else {
    ok( 0, "Timeout while waiting for "+element.attr('id')+" to change.");
    start();
  }

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

