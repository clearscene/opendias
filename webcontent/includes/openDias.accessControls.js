$(document).ready(function () {
  var role = getCookie("role");

  $("#newrealname").val(getCookie("realname"));
  $("#currentrole").html(get_priv_from_role(role, 'name') + " (" + role + ")");
  if (get_priv_from_role(role, 'update_access')) {

    $('.onlyadmin').css({
      display: 'block'
    });

    // Get a list of current users
    $.ajax({
      url: "/opendias/dynamic",
      dataType: "xml",
      timeout: AJAX_TIMEOUT,
      data: {
        action: "getUserList",
      },
      cache: false,
      type: "POST",
      error: function (x, t, m) {
        if (t == "timeout") {
          alert("[q013] " + LOCAL_timeout_talking_to_server);
        } else {
          alert("[q014] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
        }
      },
      success: function (data) {
        if ($(data).find('error').text()) {
          alert($(data).find('error').text());
        } else {
          $(data).find('GetUserList').find('Users').find('User').each(function () {
            var userid = $(this).find('username').text();
            var real = $("<td></td>").text($(this).find('realname').text() + " (" + userid + ")");
            var last = $("<td></td>").text($(this).find('last_access').text());

            var r = $(this).find('role').text();
            var s = "selected='selected'";
            var role = $("<td></td>").html("<select id='role_" + userid + "'>" 
              + "<option value='1'" + ((r == "1") ? s : "") + ">" + LOCAL_role_admin + "</option>" 
              + "<option value='2'" + ((r == "2") ? s : "") + ">" + LOCAL_role_user + "</option>" 
              + "<option value='3'" + ((r == "3") ? s : "") + ">" + LOCAL_role_view + "</option>" 
              + "<option value='4'" + ((r == "4") ? s : "") + ">" + LOCAL_role_add + "</option>" 
              + "</select>");

            var update = $("<button>Update</button>");
            update.click(function () {
              if (confirm("Set user '" + userid + "'s role to '" + $('#role_' + userid + ' option:selected').text() + "'?")) {
                $.ajax({
                  url: "/opendias/dynamic",
                  dataType: "xml",
                  timeout: AJAX_TIMEOUT,
                  data: {
                    action: "updateUser",
                    username: userid,
                    role: $('#role_' + userid).val(),
                  },
                  cache: false,
                  type: "POST",
                  error: function (x, t, m) {
                    if (t == "timeout") {
                      alert("[q015] " + LOCAL_timeout_talking_to_server);
                    } else {
                      alert("[q016] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
                    }
                  },
                  success: function (data) {
                    if ($(data).find('error').text()) {
                      alert($(data).find('error').text());
                    } else {
                      alert(LOCAL_details_updated);
                    }
                  }
                });
              }
            });

            var reset = $("<button>Reset Password</button>");
            reset.click(function () {
              if (confirm("Reset user '" + userid + "'s password to 'changeme'?")) {
                $.ajax({
                  url: "/opendias/dynamic",
                  dataType: "xml",
                  timeout: AJAX_TIMEOUT,
                  data: {
                    action: "updateUser",
                    username: userid,
                    password: 'changeme',
                  },
                  cache: false,
                  type: "POST",
                  error: function (x, t, m) {
                    if (t == "timeout") {
                      alert("[q017] " + LOCAL_timeout_talking_to_server);
                    } else {
                      alert("[q018] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
                    }
                  },
                  success: function (data) {
                    if ($(data).find('error').text()) {
                      alert($(data).find('error').text());
                    } else {
                      alert(LOCAL_details_updated);
                    }
                  }
                });
              }
            });

            var delet = $("<button>Delete</button>");
            delet.click(function () {
              if (confirm("Delete user '" + userid + "'?")) {
                $.ajax({
                  url: "/opendias/dynamic",
                  dataType: "xml",
                  timeout: AJAX_TIMEOUT,
                  data: {
                    action: "deleteUser",
                    username: userid,
                  },
                  cache: false,
                  type: "POST",
                  error: function (x, t, m) {
                    if (t == "timeout") {
                      alert("[q019] " + LOCAL_timeout_talking_to_server);
                    } else {
                      alert("[q020] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
                    }
                  },
                  success: function (data) {
                    if ($(data).find('error').text()) {
                      alert($(data).find('error').text());
                    } else {
                      alert(LOCAL_details_updated);
                    }
                  }
                });
              }
            });

            if (userid == "admin") {
              update.attr("disabled", true);
              delet.attr("disabled", true);
              role.html("&nbsp;&nbsp;" + LOCAL_role_admin);
            }

            var actions = $("<td class='actions'></td>").append(update).append(reset).append(delet);
            var row = $("<tr></tr>").append(real).append(last).append(role).append(actions);

            $('#useradmin table tbody').append(row);
          });
        }
      }
    });



    // Get a list of tags
    $.ajax({
      url: "/opendias/dynamic",
      dataType: "xml",
      timeout: AJAX_TIMEOUT,
      data: {
        action: "getTagsList",
      },
      cache: false,
      type: "POST",
      error: function (x, t, m) {
        if (t == "timeout") {
          alert("[q001] " + LOCAL_timeout_talking_to_server);
        } else {
          alert("[q002] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
        }
      },
      success: function (data) {
        if ($(data).find('error').text()) {
          alert($(data).find('error').text());
        } else {
          $(data).find('GetTagsList').find('Tags').find('Tag').each(function () {
            var tagid = $(this).find('tagid').text();
            var tagname = $("<td></td>").text($(this).find('tagname').text() + " (" + tagid + ")");
            var usedcount = $("<td></td>").text($(this).find('used_count').text());

            var purgephysical = $("<td><input size=8 value="+$(this).find('purgephysical').text()+" /></td>");
            purgephysical.change(function () {
              $.ajax({
                url: "/opendias/dynamic",
                dataType: "xml",
                timeout: AJAX_TIMEOUT,
                data: {
                  action: "updateTag",
                  subaction: "purgephysical",
                  tagid: tagid,
                  newvalue: $(this).val(),
                },
                cache: false,
                type: "POST",
                error: function (x, t, m) {
                  if (t == "timeout") {
                    alert("[q007] " + LOCAL_timeout_talking_to_server);
                  } else {
                    alert("[q008] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
                  }
                },
                success: function (data) {
                  if ($(data).find('error').text()) {
                    alert($(data).find('error').text());
                  } else {
                    alert(LOCAL_details_updated);
                  }
                }
              });
            });

            var purgedata = $("<td><input size=8 value="+$(this).find('purgedata').text()+" /></td>");
            purgedata.change(function () {
              $.ajax({
                url: "/opendias/dynamic",
                dataType: "xml",
                timeout: AJAX_TIMEOUT,
                data: {
                  action: "updateTag",
                  subaction: "purgedata",
                  tagid: tagid,
                  newvalue: $(this).val(),
                },
                cache: false,
                type: "POST",
                error: function (x, t, m) {
                  if (t == "timeout") {
                    alert("[q007] " + LOCAL_timeout_talking_to_server);
                  } else {
                    alert("[q008] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
                  }
                },
                success: function (data) {
                  if ($(data).find('error').text()) {
                    alert($(data).find('error').text());
                  } else {
                    alert(LOCAL_details_updated);
                  }
                }
              });
            });

            var row = $("<tr></tr>").append(tagname)
                                    .append(usedcount)
                                    .append(purgephysical)
                                    .append(purgedata);
            $('#managetags table tbody').append(row);
          });
        }
      }
    });

  }

  $("#tabs").tabs();

  $('#updateThisUser').click(function () {
    if ($('#newpassword').val() != $('#newpassword2').val()) {
      alert(LOCAL_new_password_do_not_match);
      return 0;
    }
    $.ajax({
      url: "/opendias/dynamic",
      dataType: "xml",
      timeout: AJAX_TIMEOUT,
      data: {
        action: "updateUser",
        username: "[current]",
        realname: $("#newrealname").val(),
        password: $("#newpassword").val(),
      },
      cache: false,
      type: "POST",
      error: function (x, t, m) {
        if (t == "timeout") {
          alert("[q009] " + LOCAL_timeout_talking_to_server);
        } else {
          alert("[q010] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
        }
      },
      success: function (data) {
        if ($(data).find('error').text()) {
          alert($(data).find('error').text());
        } else {
          setCookie("realname", $('#newrealname').val());
          setLoginOutArea();
          alert(LOCAL_details_updated);
        }
      }
    });
  });

  $('#createNewUser').click(function () {
    if ($('#createpassword').val() != $('#createpassword2').val()) {
      alert(LOCAL_new_password_do_not_match);
      return 0;
    }
    $.ajax({
      url: "/opendias/dynamic",
      dataType: "xml",
      timeout: AJAX_TIMEOUT,
      data: {
        action: "createUser",
        username: $("#createuserid").val(),
        realname: $("#createrealname").val(),
        password: $("#createpassword").val(),
        role: $("#createuserrole").val(),
      },
      cache: false,
      type: "POST",
      error: function (x, t, m) {
        if (t == "timeout") {
          alert("[q011] " + LOCAL_timeout_talking_to_server);
        } else {
          alert("[q012] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
        }
      },
      success: function (data) {
        if ($(data).find('error').text()) {
          alert($(data).find('error').text());
        } else {
          alert(LOCAL_user_created);
        }
      }
    });
  });

});
