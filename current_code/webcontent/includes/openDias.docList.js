var pageLength = 5; // number of records in the table;
var sorting = [[3,1]]; // sort by date desc
var count = 0;
var totalRows = 0;
var dta;
var allDocs = new Array();
var showingDocs = new Array();
var cc = 0;

$(document).ready(function() {

  // Check the cookies for page setting (pageLength and sortOrder)
  t = get_cookie('pageLength');
  if(t != null) {
    pageLength = t;
    $('#pageSizeSelect').val(t);
  }

  t = get_cookie('sortOrder');
  if(t != null) {
    res = t.split(",");
    sorting = [[res[0],res[1]]];
  }

  $('#tags').tagsInput({ 
          autocomplete_url: '',
          onAddTag:function(tag) { return false; },
          onRemoveTag:function(tag) { getRecordCount() }
          });

  $('#tags_tag').autocomplete({
      source: function( request, response ) {
        $.ajax({
          url: "/opendias/dynamic",
          dataType: "json",
          type: "POST",
          data: {
            action: "tagsAutoComplete",
            startsWith: request.term,
            docid: 0
          },
          success: function( data ) {
            response( $.map( data.results, function( item ) {
              return {
                label: item.tag,
                value: item.tag
              }
            }));
          }
        });
      },
      minLength: 1, // because most tags are so short - and there are not that many tags,
      select: function( event, ui ) {
      //  log( ui.item ?  "Selected: " + ui.item.label : "Nothing selected, input was " + this.value);
          getRecordCount()
      },
      open: function() {
        $( this ).removeClass( "ui-corner-all" ).addClass( "ui-corner-top" );
      },
      close: function() {
        $( this ).removeClass( "ui-corner-top" ).addClass( "ui-corner-all" );
      }
    });

  $('#docList_table').css({ display: 'none' });
  $('#pager').css({ display: 'none' });
  $('#docList_table').tablesorter({widthFixed: true, widgets: ['zebra']});

  $('#pageSizeSelect').bind('change', function() {
    document.cookie = "pageLength="+$(this).val();
  });

  $("#progressbar")
         .progressbar({ value: 0, })
         .css({ display: '' });

  loadListData();
});

function loadListData() {

  $.ajax({ url: "dynamic",
         dataType: "xml",
             data: {action: "docFilter",
                    subaction: "fullList",
                    textSearch: $('#textSearch').val(),
                    startDate: $('#startDate').val(),
                    endDate: $('#endDate').val(),
                    tags: $('#tags').val()
                   },

         cache: false,
         type: "POST",
         success: function(data){
            if( $(data).find('error').text() ){
              alert($(data).find('error').text());
              return 1;
            }
            totalRows = $(data).find('DocFilter').find('count').text();
            if(totalRows==0) {
              $('#nodocs').css({ display: 'block' });
              $('#doclist').css({ display: 'none' });
            }
            else {
              count = 0;
              dta = $(data).find('DocFilter').find('Results');
              $('#docList_table').css({ display: 'none' });
              $('#pager').css({ display: 'none' });
              cc = 0;
              processData();
            }
         }
       });
}

function get_cookie (cookie_name) {
  var results = document.cookie.match( '(^|;) ?' + cookie_name + '=([^;]*)(;|$)' );
  if(results)
    return results[2];
  else
    return null;
}

function processData(){

  rw = $(dta).find('Row')[count];

  if(!rw) {
    $("#progressContainer").css({ display: 'none' });
    $("#progressbar").css({ display: 'none' });
    $("#docList_table")
      .trigger("update") 
      .trigger("appendCache")
      .trigger("sorton",[sorting]) 
      .tablesorterPager({size: pageLength, container: $("#pager"), positionFixed: false})
      .bind("addRow",function(event,newRow){ addRow(this,newRow); })
      .bind("removeRow",function(event,rowId){ removeRow(this,rowId); })
      .bind("rebuild",function(event){ rebuild(this); })
      .bind("sortEnd", function(){ document.cookie = "sortOrder="+this.config.sortList; });

    $('#docList_table').fadeIn();
    $('#pager').fadeIn();
  }
  else {
    count++;
    var docid = $(rw).find('docid').text();
    var actionrequired = $(rw).find('actionrequired').text();
    var title = $(rw).find('title').text();
    var type = $(rw).find('type').text();
    var date = $(rw).find('date').text();

    var tr = document.createElement("tr");
    var id = document.createAttribute('id');
    tr.setAttribute('id','docid_'+docid);
    var e_docid = document.createElement("td");
    e_docid.setAttribute('class','indicator');
    e_docid.appendChild(document.createTextNode(docid));
    if(actionrequired!="1") {
      e_docid.appendChild(document.createTextNode(" "));
    } else {
      var img = document.createElement("img");
      img.setAttribute('src','/opendias/images/actionrequired.png');
      e_docid.appendChild(img);
    }
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
      document.location.href = "/opendias/docDetail.html?docid="+docid;
    });

    $("#progressbar").progressbar({
      value: parseInt( (count*100)/totalRows ),
    });

    allDocs.push(docid);
    showingDocs.push(docid);
    cc++;
    if(cc>=4) {
      cc = 0;
      setTimeout("processData()", 1); // give the ui a chance to catchup
    } else {
      processData();
    }
  }

}

