
$('#filterToggle').click( function() { 
	$('#filterToggle').toggleClass('closed');
	$('#filterToggle').toggleClass('open');
	$('#filterOptions').slideToggle('slow');
} );

//$('#filterdate').DatePicker({
//	flat: true,
//	date: ['2008-07-28','2008-07-31'],
//	current: '2008-07-31',
//	calendars: 3,
//	mode: 'range',
//	starts: 1,
//});
//$('#filterdate').DatePickerShow();

