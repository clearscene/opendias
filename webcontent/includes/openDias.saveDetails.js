formFields = new Array('title', 'actionrequired', 'docDate', 'ocrtext', 'hardcopyKept');

function sendUpdate(kkey, vvalue) {

  lockForm();
  $.ajax({
    url: "/opendias/dynamic",
    dataType: "xml",
    timeout: AJAX_TIMEOUT,
    data: {
      action: "updateDocDetails",
      docid: $('#docid').text(),
      kkey: kkey,
      vvalue: vvalue,
    },
    cache: false,
    type: "POST",
    error: function (x, t, m) {
      if (t == "timeout") {
        alert("[s001] " + LOCAL_timeout_talking_to_server);
      } else {
        alert("[s001] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
      }
    },
    success: function (data) {
      if ($(data).find('error').text()) {
        alert(LOCAL_error_while_updating);
        unlockForm(0);
      } else {
        unlockForm(1);
      }
    }
  });

}

function moveTag(tag, docid, action) {

  lockForm();
  $.ajax({
    url: "/opendias/dynamic",
    dataType: "xml",
    timeout: AJAX_TIMEOUT,
    data: {
      action: "moveTag",
      subaction: action,
      docid: docid,
      tag: tag,
    },
    cache: false,
    type: "POST",
    error: function (x, t, m) {
      if (t == "timeout") {
        alert("[s002] " + LOCAL_timeout_talking_to_server);
      } else {
        alert("[s002] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
      }
    },
    success: function (data) {
      if ($(data).find('error').text()) {
        alert(LOCAL_error_while_updating);
        unlockForm(0);
      } else {
        unlockForm(1);
      }
    }
  });
}

function lockForm() {
  changeFormState(true);
}

function unlockForm(success) {
  changeFormState(false);
  if (success) {
    $('#saveAlert').show();
    $('#saveAlert').fadeOut(4000);
  }
}

function changeFormState(state) {
  for (var key in formFields) {
    if (state) {
      $('#' + formFields[key]).attr('disabled', 'disabled');
    } else {
      $('#' + formFields[key]).removeAttr('disabled');
    }
  }
}

function oncloseEvent() {
  var notComplete = 0;
  var msg = "";
  if (document.getElementById('title').value == LOCAL_default_title) {
    notComplete = 1;
    msg += LOCAL_default_title_warning + ". ";
  }
  if (document.getElementById('docDate').value == LOCAL_default_date) {
    notComplete = 1;
    msg += LOCAL_default_date_warning + ". ";
  }
  if (document.getElementById('selected').getElementsByTagName('tr').length == 1) {
    notComplete = 1;
    msg += LOCAL_no_tags_assigned + ". ";
  }

  if (notComplete == 1) {
    return LOCAL_doc_incomplete_warning + ": " + msg;
  }
}

$(document).ready(function () {

  for (var key in formFields) {
    $('#' + formFields[key]).change(function () {
      if ($(this).is(':checkbox')) {
        sendUpdate($(this).attr('id'), $(this).is(':checked'));
      } else {
        sendUpdate($(this).attr('id'), $(this).val());
      }
    });
  }

  $('#delete').click(function () {
    var answer = confirm(LOCAL_sure_to_delete_doc);
    if (answer) {
      $.ajax({
        url: "/opendias/dynamic",
        dataType: "xml",
        timeout: AJAX_TIMEOUT,
        data: {
          action: "deleteDoc",
          docid: $('#docid').text(),
        },
        cache: false,
        type: "POST",
        error: function (x, t, m) {
          if (t == "timeout") {
            alert("[s003] " + LOCAL_timeout_talking_to_server);
          } else {
            alert("[s003] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
          }
        },
        success: function (data) {
          if ($(data).find('error').text()) {
            alert(LOCAL_error_while_deleting_doc);
            unlockForm(0);
          } else {
            document.location.href = "/opendias/";
          }
        },
      });
    }
  });

});

window.onbeforeunload = oncloseEvent;
