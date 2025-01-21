const char index_html_template[] PROGMEM = R"rawhtml(
<!DOCTYPE HTML>
<html>

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0 maximum-scale=2.5, user-scalable=1">
    <title>Temperature Forecast Linechart demo</title>

    <style>
        body {
            background-color: powderblue;
            font-family: Arial, Helvetica, sans-serif;
        }

        /* Style the tab */
        .tab {
            overflow: hidden;
            border: 1px solid #ccc;
            background-color: #f1f1f1;
        }

        /* Style the buttons inside the tab */
        .tab button {
            background-color: inherit;
            float: left;
            border: none;
            outline: none;
            cursor: pointer;
            padding: 14px 16px;
            transition: 0.3s;
            font-size: 17px;
        }

        /* Change background color of buttons on hover */
        .tab button:hover {
            background-color: #ddd;
        }

        /* Create an active/current tablink class */
        .tab button.active {
            background-color: #ccc;
        }

        /* Style the tab content */
        .tabcontent {
            display: none;
            padding: 6px 12px;
            border: 1px solid #ccc;
            border-top: none;
        }

        .inputbox {
            margin: 5px;
            margin-left: 0px;
        }
    </style>
</head>

<body>
    <!-- <h2>Tabs</h2> -->
    <!-- <p>Click on the buttons inside the tabbed menu:</p> -->
    
    <div class="tab">
        <button class="tablinks active" onclick="openTab(event, 'Chart')">Chart</button>
        <button class="tablinks" onclick="openTab(event, 'Settings')">Settings</button>
    </div>

    <div id="Chart" class="tabcontent" style="display: block;">
        <!-- <h3>Chart</h3> -->
        <p>Here should be some description.</p>
        <!-- <h3 style='color:brown'>Room Temperature</h1> -->

        <div id='temp_chart' style='width:600px; height: 400px;'></div>
        <br>

        <form action="/" method="post">
            <button type="submit" name="button1" id="button1" value="button1" onclick='update()'>CLICK FOR
                UPDATE</button>
        </form>
    </div>

    <div id="Settings" class="tabcontent">
        <!-- <h3>Settings</h3> -->
        <p>Here should be some description.</p>
        <form action="/settings" method="post">
            <div>
                <input class="inputbox" type="text" id="wifiSsid" title="ssid">wifi ssid</input><br>
                <input class="inputbox" type="password" id="wifiPassword" title="password">wifi password</input><br>
                <input class="inputbox" type="text" id="longitude" title="longitude">longitude</input><br>
                <input class="inputbox" type="text" id="latitude" title="latitude">latitude</input><br>
                <input class="inputbox" type="text" id="openWeatherKey" title="openWeatherKey">Openweather key</input><br>
                <input class="inputbox" type="text" id="ip2geoKey" title="ip2geoKey">Ip to geo key</input><br>
            </div>
            <div>
                <button type="submit" name="button2" id="button2" value="button2" onclick='submitSettings()'>Submit</button>
            </div>
        </form>
    </div>

<!--
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
-->

    <script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script>
    <script type='text/javascript'>
        google.charts.load('current', { 'packages': ['corechart'] });
        google.charts.setOnLoadCallback(drawChart);

        function drawChart() {

//            let tmp = [['Time', 'Temp']
//                %ARRAYPLACEHOLDER%
//                        ];

            let tmp = [['Time', 'Temp']];
            for (let i = 0; i < 20; ++i)
                tmp.push([i, Math.sin(i)]);
                        
            var chartData = google.visualization.arrayToDataTable(tmp);
            var chartoptions =
            {
                title: 'Local Temperature Log',
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

        function openTab(evt, cityName) {
            let tabcontent = document.getElementsByClassName("tabcontent");
            for (let i = 0; i < tabcontent.length; i++) {
                tabcontent[i].style.display = "none";
            }

            let tablinks = document.getElementsByClassName("tablinks");
            for (let i = 0; i < tablinks.length; i++) {
                tablinks[i].className = tablinks[i].className.replace(" active", "");
            }

            document.getElementById(cityName).style.display = "block";
            evt.currentTarget.className += " active";
        }

        function submitSettings() {
            // collect data from form controls and post it to the server
        }
    </script>
</body>

</html>
)rawhtml";