function listPageTest() {

  var doneSetup = 0;
    // ================================================
    module("Filter", {
        setup : function(){
          if( ! doneSetup ) {
            stop();
            $q('#testframe').attr('src', "/opendias/docList.html");
	          $q('#testframe').one('load', function() { $ = window.frames[0].jQuery; start(); } );
            doneSetup = 1;
          }
        },
        teardown : function(){ }
    });

    // ------------------------------------------------
		asyncTest('open filter form', 3, function() {  
      console.log("1. Running: open filter form");

			ok( ! $('#filterInner').is(':visible'), 'Filter form should have started out hidden');  

			$('#filterTab').click();  
      $('#filterOptions').promise().done( function() {

			  ok( $('#filterInner').is(':visible'), 'Filter form should be visible');
        equal( $('#filterProgress').text(), "", "Estimated results was expected to be blank" );

        start();

      });
		});

    // ------------------------------------------------
		asyncTest('title - filters down', 1, function() {
      console.log("2. Running: title - filters down");

      setupWaitForValue( $('#filterProgress'), "Will return an estimated 2 docs" );
      $('#textSearch').val("v").change();
      waitForValue( $('#filterProgress'), 1000 );
		});

    // ------------------------------------------------
		asyncTest('title - show all', 1, function() {
      console.log("3. Running: title show all");

      setupWaitForValue( $('#filterProgress'), "Will return an estimated 8 docs" );
      $('#textSearch').val("").change();
      waitForValue( $('#filterProgress'), 1000 );
		});

    // ------------------------------------------------
		asyncTest('action required - filters down', 1, function() {
      console.log("4. Running: action required filter down");

      setupWaitForValue( $('#filterProgress'), "Will return an estimated 0 docs" );
      $('#isActionRequired').prop("checked", true).change();
      waitForValue( $('#filterProgress'), 1000 );
		});

    // ------------------------------------------------
		asyncTest('action required - show all', 1, function() {
      console.log("5. Running: action required show all");

      setupWaitForValue( $('#filterProgress'), "Will return an estimated 8 docs" );
      $('#isActionRequired').prop("checked", false).change();
      waitForValue( $('#filterProgress'), 1000 );
		});

    // ------------------------------------------------
		asyncTest('date - filters down', 1, function() {
      console.log("6. Running: date filter down");

      setupWaitForValue( $('#filterProgress'), "Will return an estimated 1 docs" );
      $('#startDate').val("2010-01-01").change();
      $('#endDate').val("2012-12-01").change();
      waitForValue( $('#filterProgress'), 1000 );
		});

    // ------------------------------------------------
		asyncTest('date - show all', 1, function() {
      console.log("7. Running: date show all");

      setupWaitForValue( $('#filterProgress'), "Will return an estimated 8 docs" );
      $('#startDate').val("").change();
      $('#endDate').val("").change();
      waitForValue( $('#filterProgress'), 1000 );

		});


    // ------------------------------------------------
		asyncTest('close filter form', 2, function() {  
      console.log("8. Running: close filter form");

			ok( $('#filterInner').is(':visible'), 'Filter form should start out visible');  

			$('#filterTab').click();  
      $('#filterOptions').promise().done( function() {

			  ok( ! $('#filterInner').is(':visible'), 'Filter form should be visible');
        start();

      });

		});

}
