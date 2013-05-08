var filteredDocs;
var master;

$(document).ready(function () {

  $('#filterTab').click(function () {
    $('#filterOptions').slideToggle('slow');
  });

  $('#startDate').datepicker({
    dateFormat: LOCAL_date_format
  });
  $('#endDate').datepicker({
    dateFormat: LOCAL_date_format
  });

  $('#doFilter').click(function () {
    reloadResults();
  });

  $('#textSearch').keypress(function () {
    getRecordCount()
  });
  $('#textSearch').change(function () {
    getRecordCount()
  });
  $('#isActionRequired').change(function () {
    getRecordCount()
  });
  $('#startDate').change(function () {
    getRecordCount()
  });
  $('#endDate').change(function () {
    getRecordCount()
  });

});

function getRecordCount() {
  $.ajax({
    url: "/opendias/dynamic",
    dataType: "xml",
    timeout: AJAX_TIMEOUT,
    data: {
      action: "docFilter",
      subaction: "count",
      textSearch: $('#textSearch').val(),
      isActionRequired: $('#isActionRequired').is(':checked'),
      startDate: $('#startDate').val(),
      endDate: $('#endDate').val(),
      tags: $('#tags').val()
    },
    cache: false,
    type: "POST",
    success: function (data) {
      if ($(data).find('error').text()) {
        alert(LOCAL_unable_to_get_filtered_list + ": " + $(data).find('error').text());
      } else {
        $('#filterProgress').text(sprintf(LOCAL_will_return_estimated_x_docs, $(data).find('DocFilter').find('count').text()));
      }
    },
    error: function (x, t, m) {
      if (t == "timeout") {
        alert("[f001] " + LOCAL_timeout_talking_to_server);
      } else {
        alert("[f001] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
      }
    },
  });
}
