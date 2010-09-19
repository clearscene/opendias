var sorting = [[3,1]]; // sort by date desc
var count = 0;
var totalRows = 0;
var dta;
var allDocs = new Array();
var showingDocs = new Array();

$(document).ready(function() {

  $('#docList_table').css({ display: 'none' });
  $('#pager').css({ display: 'none' });
  $('#docList_table').tablesorter({widthFixed: true, widgets: ['zebra']});

  $("#progressbar")
         .progressbar({ value: 0, })
         .css({ display: '' });

  $.ajax({ url: "dynamic",
         dataType: "xml",
         data: {action: "getDocList"},
         cache: false,
         type: "POST",
         success: function(data){
            totalRows = $(data).find('count').text();
            count = 0;
            dta = data;
            $('#docList_table').css({ display: 'none' });
            $('#pager').css({ display: 'none' });
            processData();
         }
       });
});

  function processData(){

    rw = $(dta).find('row')[count];

    if(!rw) {
      $("#progressContainer").css({ display: 'none' });
      $("#progressbar").css({ display: 'none' });
      $("#docList_table")
        .trigger("update") 
        .trigger("appendCache")
        .trigger("sorton",[sorting]) 
        .tablesorterPager({size: 5, container: $("#pager"), positionFixed: false})
        .bind("addRow",function(event,newRow){ addRow(this,newRow); })
        .bind("removeRow",function(event,rowId,moveTo){ removeRow(this,rowId,moveTo); });

      $('#docList_table').fadeIn();
      $('#pager').fadeIn();
    }
    else {
      count++;
      var docid = $(rw).find('docid').text();
      var title = $(rw).find('title').text();
      var type = $(rw).find('type').text();
      var date = $(rw).find('date').text();

      var tr = document.createElement("tr");
      var id = document.createAttribute('id');
      tr.setAttribute('id','docid_'+docid);
      var e_docid = document.createElement("td");
      e_docid.appendChild(document.createTextNode(docid));
      tr.appendChild(e_docid);
      var e_title = document.createElement("td");
      e_title.appendChild(document.createTextNode(title));
      tr.appendChild(e_title);
      var e_type = document.createElement("td");
      e_type.appendChild(document.createTextNode(type));
      tr.appendChild(e_type);
      var e_date = document.createElement("td");
      e_date.appendChild(document.createTextNode(date));
      tr.appendChild(e_date);
      document.getElementById('docList_table').getElementsByTagName('tbody')[0].appendChild(tr);

      $('#docid_'+docid).click(function() { 
        document.location.href = "/docDetail.html?docid="+docid;
      });

      $("#progressbar").progressbar({
        value: parseInt( (count*100)/totalRows ),
      });

      allDocs.push(docid);
      showingDocs.push(docid);
      setTimeout("processData()", 1); // give the ui a chance to catchup
    }

  }

