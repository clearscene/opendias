(function(){

  var doneload = 0;
  var fulllist = '';
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
          $q('#testframe').attr('src', "/opendias/docList.html");
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
  asyncTest('action required - filters down', 1, function() {
    console.log("4. Running: action required filter down");

    setupWaitForValue( $('#filterProgress'), "Will return an estimated 0 docs" );
    $('#isActionRequired').prop("checked", true).change();
    waitForValue( $('#filterProgress'), 1000 );
  });

  // ------------------------------------------------
  asyncTest('action required - show all', 2, function() {
    console.log("5. Running: action required show all");

    $('#isActionRequired').prop("checked", false).change();
    setTimeout( function() {
      fulllist = $('#filterProgress').text();
      notEqual( fulllist, "", "Expected to show a full list count.");
      notEqual( fulllist, "Will return an estimated 0 docs", "Expected to show a full list count.");
      start();
    }, 1000 );
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

    setupWaitForValue( $('#filterProgress'), fulllist );
    $('#textSearch').val("").change();
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

    setupWaitForValue( $('#filterProgress'), fulllist );
    $('#startDate').val("").change();
    $('#endDate').val("").change();
    waitForValue( $('#filterProgress'), 1000 );
  });

  // ------------------------------------------------
  asyncTest('filter - tag autoselect popup', 1, function() {
    console.log("8. Running: tag autoselect populate");

    setupWaitForValue( $('body > ul.ui-autocomplete'), "WagesWater" );
    $('#tags_tag').trigger('keydown').val("w").trigger("keyup");
    waitForValue( $('body > ul.ui-autocomplete'), 1000 );
  });

  // ------------------------------------------------
//    asyncTest('filter - tag select', 1, function() {
//      console.log("9. Running: tag filter down");
//
//      setupWaitForValue( $('#filterProgress'), "Will return an estimated 1 docs" );
//      // I can't get to select an option from the autocomplete dropdown !!!
//      //$('body > ul.ui-autocomplete > li:first > a').focus();
//      //$('body > ul.ui-autocomplete > li:first > a').click();
//      //$('#tags_tag').autocomplete('search', 'wages').select().close();
//      //$('#tags').select();
//      //$('#tags_tag').select();
//      //waitForValue( $('#filterProgress'), 1000 );
//    });

  // ------------------------------------------------
  asyncTest('filter - tag autoselect remove', 1, function() {
    console.log("8. Running: tag autoselect remove");

    setupWaitForValue( $('body > ul.ui-autocomplete'), null );
    $('#tags_tag').trigger('keydown').val("").trigger("keyup");
    $('#doFilter').focus();
    waitForValue( $('body > ul.ui-autocomplete'), 1000 );
  });

  // ------------------------------------------------
  asyncTest('close filter form', 2, function() {  
    console.log(". Running: close filter form");

    ok( $('#filterInner').is(':visible'), 'Filter form should start out visible');  

    $('#filterTab').click();  
    $('#filterOptions').promise().done( function() {

      ok( ! $('#filterInner').is(':visible'), 'Filter form should be visible');
      start();

    });

  });

})();
