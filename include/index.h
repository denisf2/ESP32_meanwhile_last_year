const char index_html_template[] PROGMEM = R"rawhtml(
<!DOCTYPE HTML>
<html>

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0 maximum-scale=2.5, user-scalable=1">
    <title>Temperature Forecast Linechart demo</title>
    <style>
        body {
            background-color: powderblue;
        }
    </style>
</head>

<body>
    <h1 style='color:red'>Hobby room
        <br>
        temperature
    </h1>
    <div id='temp_chart' style='width:600px; height: 400px;'></div>
    <br>
    <FORM action="/" method="post">
        <button type="submit" name="button1" id="button1" value="button1" onclick='update()'>CLICK FOR
            UPDATE</button>
    </form>
    <script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script>
    <script type='text/javascript'>
        google.charts.load('current', { 'packages': ['corechart'] });
        google.charts.setOnLoadCallback(drawChart);
        function drawChart() {

            let tmp = [['Time', 'Temp']
                %ARRAYPLACEHOLDER%
                        ];
            var chartData = google.visualization.arrayToDataTable(tmp);
            var chartoptions =
            {

                title: 'Mancave temperature',
                legend: { position: 'bottom' },
                width: 600,
                height: 400,
                chartArea: { width: '90%%', height: '75%%' }
            };
            var chart = new google.visualization.LineChart(document.getElementById('temp_chart'));
            chart.draw(chartData, chartoptions);
        }
        function update() {

            chart.draw(chartData, chartoptions);
        } 
    </script>
</body>

</html>
)rawhtml";