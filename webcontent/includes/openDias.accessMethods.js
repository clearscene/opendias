$(document).ready(function () {

  setLoginOutArea();

  $('#loginbutton').click(function () {
    $('#loginbutton').attr("disabled", true);
    $.ajax({
      url: "/opendias/dynamic",
      dataType: "xml",
      timeout: 10000,
      data: {
        action: "checkLogin",
        username: $('#username').val(),
        password: $('#password').val(),
      },
      cache: false,
      type: "POST",
      error: function (x, t, m) {
        $('#password').val('');
        if (t == "timeout") {
          alert("[s001] " + LOCAL_timeout_talking_to_server);
        } else {
          alert("[s001] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
        }
      },
      success: function (data) {
        $('#password').val('');
        if ($(data).find('error').text()) {
          alert($(data).find('error').text());
        } else {
          if ($(data).find('result').text() == 'OK') {
            $('#loginbutton').attr("disabled", false);
            setLoginOutArea();
          } else {
            $('#loginbutton').css({
              display: 'none'
            });
            setTimeout(function () {
              $('#loginbutton').attr("disabled", false);
              $('#loginbutton').css({
                display: 'inline'
              });
            }, parseInt($(data).find('retry_throttle').text()) * 1000);
            alert($(data).find('message').text());
          }
        }
      }
    });
  });


  $('#logoutbutton').click(function () {
    $('#password').val('');
    $.ajax({
      url: "/opendias/dynamic",
      dataType: "xml",
      timeout: 10000,
      data: {
        action: "logout"
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
          alert($(data).find('error').text());
        } else {
          deleteCookie("realname", null);
          document.location.href = "/opendias/";
        }
      }
    });
  });
});


function setLoginOutArea() {
  // Display or not, the login area
  // If we have a cookie "realname=<anything>"
  // then show the logout only. Otherwise
  // show the login area.
  var realname = getCookie("realname");
  if (realname == null || realname == "") {
    $('#logout').css({
      display: 'none'
    });
    $('#login').css({
      display: 'block'
    });
  } else {
    $('#realname').html(realname);
    $('#login').css({
      display: 'none'
    });
    $('#logout').css({
      display: 'block'
    });
  }
  updateMenuLinks();
}

function updateMenuLinks() {
  // Remove links the user can't use
  var role = getCookie("role");
  if (get_priv_from_role(role, 'view_doc')) {
    $('#doclistLink').parent().css({
      display: 'inline-block'
    });
  } else {
    $('#doclistLink').parent().css({
      display: 'none'
    });
  }
  if (get_priv_from_role(role, 'add_import') || get_priv_from_role(role, 'add_scan')) {
    $('#acquireLink').parent().css({
      display: 'inline-block'
    });
  } else {
    $('#acquireLink').parent().css({
      display: 'none'
    });
  }
}


// This does not form security, it just stop
// presenting forms and calling API function, that 
// we know are going to casue a 'permission denied'
// response.

function get_priv_from_role(user_role, priv) {

  if (user_role == undefined) {
    return 0;
  }

  // These details are set in the application using the 'access_role' table.
  //         | update_access | view_doc | edit_doc | delete_doc | add_import | add_scan 
  // 1,"admin",   1,              1,          1,        1,            1,          1
  // 2,"user",    0,              1,          1,        1,            1,          1
  // 3,"view",    0,              1,          0,        0,            0,          0
  // 4,"add",     0,              0,          0,        0,            1,          1

  var role = [];
  role[1] = {};
  role[2] = {};
  role[3] = {};
  role[4] = {};

  role[1].name = LOCAL_role_admin;
  role[2].name = LOCAL_role_user;
  role[3].name = LOCAL_role_view;
  role[4].name = LOCAL_role_add;

  role[1].update_access = 1;
  role[2].update_access = 0;
  role[3].update_access = 0;
  role[4].update_access = 0;

  role[1].view_doc = 1;
  role[2].view_doc = 1;
  role[3].view_doc = 1;
  role[4].view_doc = 0;

  role[1].edit_doc = 1;
  role[2].edit_doc = 1;
  role[3].edit_doc = 0;
  role[4].edit_doc = 0;

  role[1].delete_doc = 1;
  role[2].delete_doc = 1;
  role[3].delete_doc = 0;
  role[4].delete_doc = 0;

  role[1].add_import = 1;
  role[2].add_import = 1;
  role[3].add_import = 0;
  role[4].add_import = 1;

  role[1].add_scan = 1;
  role[2].add_scan = 1;
  role[3].add_scan = 0;
  role[4].add_scan = 1;

  return role[user_role][priv];
}
