AJAX_TIMEOUT = 10000;
$(document).ready(function () {

  // Set the language dropdown to the current value
  var currentLang = getCookie("requested_lang");
  if (currentLang != null) {
    $('#language').val(currentLang);
  }

  // Handle an update to the language setting
  $('#language').change(function () {
    var setting = $('#language').val();
    if (setting == '--') {
      deleteCookie("requested_lang");
    } else {
      setCookie("requested_lang", setting);
    }
    window.location.reload(true);
  });

});

function setCookie(name, value, days) {
  if (days) {
    var date = new Date();
    date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));
    var expires = "; expires=" + date.toGMTString();
  } else var expires = "";
  document.cookie = name + "=" + value + expires + "; path=/";
}

function getCookie(name) {
  var nameEQ = name + "=";
  var ca = document.cookie.split(';');
  for (var i = 0; i < ca.length; i++) {
    var c = ca[i];
    while (c.charAt(0) == ' ') c = c.substring(1, c.length);
    if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length, c.length);
  }
  return null;
}

function deleteCookie(name) {
  setCookie(name, "", -1);
}
