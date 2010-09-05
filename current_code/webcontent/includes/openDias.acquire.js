$(document).ready(function() {

  $("#tabs").tabs();

  $.ajax({ url: "dynamic",
         dataType: "xml",
         data: {action: "getScannerList"},
         cache: false,
         type: "POST",
         success: function(data){
           if( $(data).find('error').text() ){
             alert("Error in fetching details.");
             return 1;
           }
           var device=0;
           $(data).find('device').each( function() {

             device++;

             // Create new tab
             var li = document.createElement("li");
             var a = document.createElement("a");
             a.setAttribute('href','#device_'+device);
             a.appendChild(document.createTextNode($(this).find("type").text() + ": " + 
                                                    $(this).find("vendor").text() + " - " + 
                                                    $(this).find("model").text() ));
             li.appendChild(a);
             $('#tabList').append(li);

             // Create scan doc
             var div = document.createElement("div");
             div.setAttribute('id','device_'+device);

             $('#tabs').append(div);

//      var tr = document.createElement("tr");
//      var id = document.createAttribute('id');
//      tr.setAttribute('id','docid_'+docid);
//      var e_docid = document.createElement("td");
//      e_docid.appendChild(document.createTextNode(docid));
//      tr.appendChild(e_docid);
//      var e_title = document.createElement("td");
//      e_title.appendChild(document.createTextNode(title));
//      tr.appendChild(e_title);
//      var e_type = document.createElement("td");
//      e_type.appendChild(document.createTextNode(type));
//      tr.appendChild(e_type);
//      var e_date = document.createElement("td");
//      e_date.appendChild(document.createTextNode(date));
//      tr.appendChild(e_date);
//      document.getElementById('docList_table').getElementsByTagName('tbody')[0].appendChild(tr);
             
           });

         $("#tabs").tabs();
         }
  });

});

