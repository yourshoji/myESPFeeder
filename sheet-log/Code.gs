function doGet(e){
  return handleResponse(e);
}

function doPost(e){
  return handleResponse(e);
}

function handleResponse(e){
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  var params = e.parameter;

  // Append new row: timestamp + any data you send
  sheet.appendRow([new Date(), params.value1, params.value2]);

  return ContentService.createTextOutput("Success");
}
