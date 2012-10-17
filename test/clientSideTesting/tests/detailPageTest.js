(function(){

  var doneload = 0;
  // ================================================
  module("Filter", {
      setup : function(){
        if( ! doneload ) {
          stop();
          $q('#testframe').one('load', function() { 
            $ = window.frames[0].jQuery; 
            doneload = 1; 
            start();
          });
          $q('#testframe').attr('src', "/opendias/docDetail.html?docid=1");
          window.setTimeout( function() {
            if( ! doneload ) {
              ok( 0, "Did not load the list page.");
            }
          }, 5000);
        }
      },
      teardown : function(){ }
  });

  // ------------------------------------------------
  asyncTest('some test', 1, function() {
    console.log("2. Running: title - filters down");

    //setupWaitForValue( $('#filterProgress'), "Will return an estimated 2 docs" );
    //$('#textSearch').val("v").change();
    //waitForValue( $('#filterProgress'), 1000 );
    ok( 1, "This is OK" );
    start();
  });

})();
