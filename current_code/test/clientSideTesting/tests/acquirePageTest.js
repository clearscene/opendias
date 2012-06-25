
  var acquirePageTestDoneSetup = 0;
    // ================================================
    module("Acquire", {
        setup : function(){
          if( ! acquirePageTestDoneSetup ) {
            stop();
	          $q('#testframe').one('load', function() { 
              $ = window.frames[0].jQuery; 
              acquirePageTestDoneSetup = 1;
              start();
            });
            $q('#testframe').attr('src', "/opendias/acquire.html");
            window.setTimeout( function() {
              if( ! acquirePageTestDoneSetup ) {
                ok( 0, "Did not load the acquire page.");
              }
            }, 5000);
          }
        },
        teardown : function(){ }
    });

    // ------------------------------------------------
		asyncTest('loading', 1, function() {
      console.log("2. Running: title - filters down");

      ok( 1, "1 was expteded to be OK" );
      start();
		});


