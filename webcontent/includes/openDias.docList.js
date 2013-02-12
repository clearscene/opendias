var field = 0;
var order = 1;
var count = 0;
var ENTERIES_PER_PAGE = 12; // keep this even.
var odd = 0;
var check_result_block = 1;
var currentOrder = new Array(3, 1); // default: most recent docs first
var defaultSort = 'ui-icon-radio-on';
var sortFields = new Array("docid", "title", "type", "date");
var orderClass = new Array("ui-icon-carat-1-s", "ui-icon-carat-1-n", // Indicator
"ui-icon-triangle-1-s", "ui-icon-triangle-1-n"); // What's selected

$(document).ready(function () {

  // Get expected sort order
  t = get_cookie('sortOrder');
  if (t != null) {
    currentOrder = t.split(",");
  }

  $('#tags').tagsInput({
    autocomplete_url: '',
    onAddTag: function (tag) {
      return false;
    },
    onRemoveTag: function (tag) {
      getRecordCount()
    }
  });

  $('#tags_tag').autocomplete({
    source: function (request, response) {
      $.ajax({
        url: "/opendias/dynamic",
        timeout: AJAX_TIMEOUT,
        dataType: "json",
        type: "POST",
        data: {
          action: "tagsAutoComplete",
          startsWith: request.term,
          docid: 0
        },
        success: function (data) {
          response($.map(data.results, function (item) {
            return {
              label: item.tag,
              value: item.tag
            }
          }));
        },
        error: function (x, t, m) {
          if (t == "timeout") {
            alert("[l001] " + LOCAL_timeout_talking_to_server);
          } else {
            alert("[l001] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
          }
        },
      });
    },
    minLength: 1, // because most tags are so short - and there are not that many tags,
    select: function (event, ui) {
      getRecordCount()
    },
    open: function () {
      $(this).removeClass("ui-corner-all").addClass("ui-corner-top");
    },
    close: function () {
      $(this).removeClass("ui-corner-top").addClass("ui-corner-all");
    }
  });

  var role = getCookie("role");
  if (get_priv_from_role(role, 'view_doc')) {
    reloadResults();
    $(window).scroll(function () {
      loadListData(check_result_block);
    });
  }

});

function get_cookie(cookie_name) {
  var results = document.cookie.match('(^|;) ?' + cookie_name + '=([^;]*)(;|$)');
  if (results) return results[2];
  else return null;
}

function reloadResults() {

  check_result_block = 0;
  $('#binding_block').replaceWith("<div id='binding_block'><div id='result_block_1'></div></div>");

  $.each(sortFields, function (fieldIndex, fieldName) {

    // reset styles
    $('#sortby' + fieldName + ' span.ui-icon').removeClass(defaultSort);
    $.each(orderClass, function (orderClassIndex, orderClassName) {
      $('#sortby' + fieldName + ' span.ui-icon').removeClass(orderClassName);
    });
    $('#sortby' + fieldName).unbind('click');
    $('#sortby' + fieldName).unbind('mouseenter mouseleave'); // 'hover' does not work

    // Set the curent order
    if (sortFields[currentOrder[field]] == fieldName) {
      $('#sortby' + fieldName + ' span.ui-icon').addClass(orderClass[2 + parseInt(currentOrder[order])]);
      $('#sortby' + fieldName).hover(

      function () {
        $('#sortby' + fieldName + ' span.ui-icon').addClass(orderClass[oposite(currentOrder[order])]);
        $('#sortby' + fieldName + ' span.ui-icon').removeClass(orderClass[2 + parseInt(currentOrder[order])]);
      },

      function () {
        $('#sortby' + fieldName + ' span.ui-icon').addClass(orderClass[2 + parseInt(currentOrder[order])]);
        $('#sortby' + fieldName + ' span.ui-icon').removeClass(orderClass[oposite(currentOrder[order])]);
      });
      $('#sortby' + fieldName).click(function () {
        document.cookie = 'sortOrder=' + fieldIndex + ',' + oposite(currentOrder[order]) + '; path=/';
        currentOrder = new Array(fieldIndex, oposite(currentOrder[order]));
        reloadResults();
      });

    } else {
      $('#sortby' + fieldName + ' span.ui-icon').addClass(defaultSort);
      $('#sortby' + fieldName).hover(

      function () {
        $('#sortby' + fieldName + ' span.ui-icon').removeClass(defaultSort);
        $('#sortby' + fieldName + ' span.ui-icon').addClass(orderClass[0]);
      },

      function () {
        $('#sortby' + fieldName + ' span.ui-icon').removeClass(orderClass[0]);
        $('#sortby' + fieldName + ' span.ui-icon').addClass(defaultSort);
      });
      $('#sortby' + fieldName).click(function () {
        document.cookie = 'sortOrder=' + fieldIndex + ',0; path=/';
        currentOrder = new Array(fieldIndex, 0);
        reloadResults();
      });
    }
  });

  loadListData(1);
}

function oposite(inn) {
  if (inn == 1) {
    return 0;
  } else {
    return 1;
  }
}

function loadListData(currentPage) {

  if (currentPage == 0 // We're already calculating the next blocl
  ||
  !is_in_viewport(currentPage) // is the block we want to fill in view?
  ) {
    return 0;
  }
  check_result_block = 0;
  $('#loading').css({
    display: 'block'
  });

  $.ajax({
    url: "dynamic",
    timeout: AJAX_TIMEOUT,
    dataType: "xml",
    data: {
      action: "docFilter",
      subaction: "fullList",
      textSearch: $('#textSearch').val(),
      isActionRequired: $('#isActionRequired').is(':checked'),
      startDate: $('#startDate').val(),
      endDate: $('#endDate').val(),
      tags: $('#tags').val(),
      page: currentPage,
      range: ENTERIES_PER_PAGE,
      sortfield: currentOrder[field],
      sortorder: currentOrder[order]
    },
    cache: false,
    type: "POST",
    success: function (data) {
      if ($(data).find('error').text()) {
        alert($(data).find('error').text());
        $('#loading').css({
          display: 'none'
        });
        return 1;
      }
      var totalRows = $(data).find('DocFilter').find('count').text();
      if (totalRows > 0) {
        // Display rows
        $('#nodocs').css({
          display: 'none'
        });
        $('#doclist').css({
          display: 'block'
        });
        var dta = $(data).find('DocFilter').find('Results');
        count = 0;
        processData(dta, currentPage);

        // Setup new results_block
        currentPage++;
        var new_result = document.createElement("div");
        var id = document.createAttribute('id');
        new_result.setAttribute('id', 'result_block_' + currentPage);
        document.getElementById('binding_block').appendChild(new_result);
        $('#loading').css({
          display: 'none'
        });
        if (!loadListData(currentPage)) {
          check_result_block = currentPage;
        }
      } else {
        $('#loading').css({
          display: 'none'
        });
        check_result_block = 0;
      }
    },
    error: function (x, t, m) {
      if (t == "timeout") {
        alert("[l002] " + LOCAL_timeout_talking_to_server);
      } else {
        alert("[l002] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
      }
    },
  });
  return 1;
}

function processData(in_data, thispage) {

  rw = $(in_data).find('Row')[count];

  if (rw) {

    // Gather data
    var docid = $(rw).find('docid').text();
    var actionrequired = $(rw).find('actionrequired').text();
    var hardcopyKept = $(rw).find('hardcopyKept').text();
    var title = $(rw).find('title').text();
    var type = $(rw).find('type').text();
    var date = $(rw).find('date').text();

    // Create row with id
    var tr = document.createElement("div");
    var id = document.createAttribute('id');
    tr.setAttribute('id', 'docid_' + docid);
    if (odd) {
      tr.setAttribute('class', 'tablerow zebra');
      odd = 0;
    } else {
      tr.setAttribute('class', 'tablerow');
      odd = 1;
    }

    // Create docid column
    var e_docid = document.createElement("div");
    e_docid.setAttribute('class', 'indicator');
    e_docid.setAttribute('class', 'tabledocid');
    e_docid.appendChild(document.createTextNode(docid));
    if (actionrequired != "1") {
      e_docid.appendChild(document.createTextNode(" "));
    } else {
      var img = document.createElement("img");
      img.setAttribute('src', '/opendias/images/actionrequired.png');
      e_docid.appendChild(img);
    }
    tr.appendChild(e_docid);

    // Create 'title' column
    var e_title = document.createElement("div");
    e_title.setAttribute('class', 'tabletitle');
    e_title.appendChild(document.createTextNode(title));
    tr.appendChild(e_title);

    // Create 'type' column
    var e_type = document.createElement("div");
    e_type.setAttribute('class', 'tabletype');
    e_type.appendChild(document.createTextNode(type));
    tr.appendChild(e_type);

    // Create 'data' column
    var e_date = document.createElement("div");
    e_date.setAttribute('class', 'tabledate');
    e_date.appendChild(document.createTextNode(date));
    tr.appendChild(e_date);

    // Add to the main document.
    document.getElementById('result_block_' + thispage).appendChild(tr);

    // Make clickable
    $('#docid_' + docid).click(function () {
      document.location.href = "/opendias/docDetail.html?docid=" + docid;
    });

    count++;
    processData(in_data, thispage);

  }

}

function is_in_viewport(el) {

  if (el == 1) {
    return 1; // The first page we'll fill regardless.
  }
  var rect = document.getElementById('result_block_' + el).getBoundingClientRect()
  return (
  rect.top >= 0 && rect.left >= 0 && rect.bottom <= window.innerHeight && rect.right <= window.innerWidth)
}
