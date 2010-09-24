function addRow(table, newRow){
    //Rebuild html table from the modified rowsCopy
    $.tablesorter.clearTableBody(table);
    if (table.config.rowsCopy!=undefined)
        $.each(table.config.rowsCopy, function() {
            $(table).find('tbody:first').append($(this));
        });
    //Append new row to the HTML table
    $(table).find('tbody:first').append(newRow);
    rebuild(table);
}

function removeRow(table, rowId){
    var disposedRow = $.grep(table.config.rowsCopy, function(n, i){                                            
                                return ($(n).attr('id')==rowId);
                           });
    var newRowsCopy = $.grep(table.config.rowsCopy, function(n, i){                                            
                                return ($(n).attr('id')!=rowId);
                           });
    table.config.rowsCopy = newRowsCopy;
    $.tablesorter.clearTableBody(table);
    if (table.config.rowsCopy!=undefined)
        $.each(table.config.rowsCopy, function() {
          $(table).find('tbody:first').append($(this));
        });
    if (disposedRow!=undefined)
        $.each(disposedRow, function() {
          $('#safeKeeping').find('tbody:first').append($(this));
          //safeKeeping[rowId] = $(this);
        });
}

function rebuild(table) {
    //Update call will rebuild tablesorter.cache from the existing HTML table
    $(table).trigger("update");
    $(table).trigger("appendCache");
    $(table).trigger("sorton",[table.config.sortList]);                
}
 
// Add trigggers
//
//$this.bind("addRow",function(event,newRow){
//    addRow(this,newRow);
//});
//$this.bind("removeRow",function(event,rowId){
//    removeRow(this,rowId);
//});
//After which, you may call these 2 functions from your page
//var newTr = $('<tr><td>NEW ROW</td></tr>');
//$(sortTable).trigger("addRow",[newTr]); /*newTr is a new row object*/
//$(sortTable).trigger("removeRow",[id for the row to be removed]);
