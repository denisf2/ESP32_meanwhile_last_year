#include "HtmlBuilder.h"

// =========================================
// This is where we build the HTML page
// =========================================
String BuildHTML(int aTempArray[], String aTimeArray[], size_t aArraySize)
{
  String page;

  page += "<!DOCTYPE HTML>";
  page += "<html>";
  page += "<head>";
  page += "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0 maximum-scale = 2.5, user-scalable=1\">";
  page += "<title>Luc's Linechart demo</title>";
  page += "<style>";
  page += "body { background-color:powderblue; }";
  page += "</style>";
  page += "</head>";
  page += "<body>";

  page += "<h1 style='color:red'>Hobby room";
  page += "<br>";
  page += "temperature</h1>";

  page += "<div id='temp_chart' style='width:600px; height: 400px;'></div>";
  page += "<br>";

  page += "<FORM action=\"/\" method=\"post\">";
  page += "<button type=\"submit\" name=\"button1\" id=\"button1\" value=\"button1\" onclick=\'update()\'>CLICK FOR UPDATE</button>";
  page += "</form>";

  page += "<script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script>";
  page += "<script type='text/javascript'>";
  page += "google.charts.load('current', {'packages':['corechart']});";
  page += "google.charts.setOnLoadCallback(drawChart);";

  page += "function drawChart() {";

  page += "let tmp = [['Time' , 'Temp']";
  for (size_t i = 0; i < aArraySize; ++i)
  {
    page += ",[";
    page += String(aTimeArray[i]);
    page += ",";
    page += String(aTempArray[i]);
    page += "]";
  }

  page += "];";

  page += "var chartData = google.visualization.arrayToDataTable(tmp);";

  page += "var chartoptions =";
  page += "{";
  page += "title: 'Mancave temperature',";
  page += "legend: { position: 'bottom' },";
  page += "width: 600,";
  page += "height: 400,";
  page += "chartArea: {width: '90%', height: '75%'}";
  page += "}; ";

  page += "var chart = new google.visualization.LineChart(document.getElementById('temp_chart'));";
  page += "chart.draw(chartData, chartoptions);";
  page += "}";

  page += "function update() {";
  page += "chart.draw(chartData, chartoptions);";
  page += "}";

  page += "</script>";
  page += "</body>";
  page += "</html>";

  return page;
}