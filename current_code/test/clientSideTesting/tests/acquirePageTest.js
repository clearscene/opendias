(function(){
  var doneload = 0;
  var initial = null;
  // ================================================
  module("Acquire", {
      setup : function(){
        if( ! doneload ) {
          stop();
          $q('#testframe').one("load", function() { 
            $ = window.frames[0].jQuery; 
            doneload = 1;
            start();
          });
          $q('#testframe').attr('src', "/opendias/acquire.html");
          window.setTimeout( function() {
            if( ! doneload ) {
              ok( 0, "Did not load the acquire page.");
            }
          }, 5000);
        }
      },
      teardown : function(){ }
  });

  // ------------------------------------------------
/*  asyncTest('Scanning for devices', 0, function() {
    console.log("2. Running: Scanning for devices");

    //setupWaitForValue( $('#tabList > li'), null );
    //ok( $('#scanning').is(':visible'), "Check that we're displaying a scanning devices message" );
    //equal( $('#tabList > li').length, 2);
    //waitForValue( $('#scanning'), 1000 );
  });
*/

  // ------------------------------------------------
  asyncTest('Info page reads OK', 2, function() {
    console.log("2. Running: Info page reads OK");

    equal( $('#tabList > li').length, 3);
    equal( $('#info > h3').text(), "Acquiring a new document", "Check we have the correct tab");
    start();
  });

  // ------------------------------------------------
  asyncTest('Scanning Tab', 8, function() {
    console.log("2. Running: Scanning Tab");

    $('#tabList > li:last > a').click();
    ok( $('#deviceTab_1 h3').is(':visible'), "Check the tab has become active");
    equal( $('#deviceTab_1 h3').text(), "Virtual Device: Noname - Frontend-tester (on host 'opendias server')", "Check we have the correct tab");
    equal( $('#resolutionDisplay_1').text(), "300 dpi", "Is the reported resolution what we expect at the start");

    ok( $('#ocr_1').is(':enabled'), "Is the OCR language selection available" );
    ok( ! $('#deviceTab_1 .resolutionQuality').hasClass("poorResolution"), "Is the quality indicator in the green" );

    var s = $('#resolutionSlider_1').slider();
    s.slider( 'value', 100 );
    s.slider('option','slide').call(s,null,{ handle: $('.ui-slider-handle', s), value: 100 });

    equal( $('#resolutionDisplay_1').text(), "100 dpi", "Did the slider update the reported resolution");

    ok( ! $('#ocr_1').is(':enabled'), "Has the OCR langauge selection gone grey" );
    ok( $('#deviceTab_1 .resolutionQuality').hasClass('poorResolution'), "Has the quality indicator gone grey" );

    start();
  });

  // ------------------------------------------------
  asyncTest('Scanning 1/5', 2, function() {
    console.log("2. Running: Scanning 1/5");

    equal( $('#status_1').text(), "", "Expecting the status to be blank at the start");

    setupWaitForValue( $('#status_1'), "Setting up the scanner" );
    $('#scanButton_1').click();
    waitForValue( $('#status_1'), 3000 );

    stop();
    setTimeout( function() {
      start();
    }, 30);
  });

  // ------------------------------------------------
  asyncTest('Scanning 2/5', 1, function() {
    console.log("2. Running: Scanning 2/5");

    setupWaitForValue( $('#status_1'), "Waiting on the database" );
    waitForValue( $('#status_1'), 3000 );
  });

  // ------------------------------------------------
  asyncTest('Scanning 3/5', 1, function() {
    console.log("2. Running: Scanning 3/5");

    setupWaitForValue( $('#status_1'), "Scanning in progress" );
    waitForValue( $('#status_1'), 3000 );
  });

  // ------------------------------------------------
  asyncTest('Scanning 3a/5', 1, function() {
    console.log("2. Running: Scanning 3a/5");

    setupWaitForValue( $('#status_1'), "Waiting on the database" );
    waitForValue( $('#status_1'), 3000 );
  });

  // ------------------------------------------------
  asyncTest('Scanning 4/5', 1, function() {
    console.log("2. Running: Scanning 4/5");

    setupWaitForValue( $('#status_1'), "Converting image format" );
    waitForValue( $('#status_1'), 3000 );
  });

  // ------------------------------------------------
  asyncTest('Scanning 5/5', 1, function() {
    console.log("2. Running: Scanning 5/5");

    setupWaitForValue( $('#status_1'), "Scan operation complete" );
    waitForValue( $('#status_1'), 3000 );
  });

})();
