<!DOCTYPE html>
<html lang="id">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Solar Panel SCADA</title>
    
    <script src="https://cdnjs.cloudflare.com/ajax/libs/paho-mqtt/1.0.1/mqttws31.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/xlsx/0.17.0/xlsx.full.min.js"></script>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css">
    <!-- <link rel="stylesheet" href="style_2.css"> -->
     <link rel="stylesheet" href="style.css">
</head>
<body>

    <h1><i class="fas fa-solar-panel"></i> Solar Panel SCADA</h1>

    <div class="dashboard">
        <div class="card">
            <div class="icon"><i class="fas fa-bolt"></i></div>
            <div class="title">Tegangan</div>
            <div class="value" id="val-voltage">0.00 <span class="unit">V</span></div>
        </div>
        <div class="card" style="border-left-color: #3498db;">
            <div class="icon" style="color: #3498db;"><i class="fas fa-plug"></i></div>
            <div class="title">Arus</div>
            <div class="value" id="val-current">0.00 <span class="unit">A</span></div>
        </div>
        <div class="card" style="border-left-color: #e74c3c;">
            <div class="icon" style="color: #e74c3c;"><i class="fas fa-temperature-high"></i></div>
            <div class="title">Suhu Panel</div>
            <div class="value" id="val-temp-panel">0.00 <span class="unit">°C</span></div>
        </div>
        <div class="card" style="border-left-color: #9b59b6;">
            <div class="icon" style="color: #9b59b6;"><i class="fas fa-server"></i></div>
            <div class="title">Suhu Box</div>
            <div class="value" id="val-temp-box">0.00 <span class="unit">°C</span></div>
        </div>
    </div>

    <div class="control-container">
        <div class="control-card">
            <div class="control-header">
                <i class="fas fa-fan" id="icon-fan"></i>
                <h3>Exhaust Fan</h3>
            </div>
            <p>Status: <span id="status-fan">OFF</span></p>
            <label class="switch">
                <input type="checkbox" id="btn-fan" onchange="toggleFan()">
                <span class="slider round"></span>
            </label>
        </div>

        <div class="control-card">
            <div class="control-header">
                <i class="fas fa-car-battery" style="color: #2ecc71;"></i>
                <h3>Battery Charge</h3>
            </div>
            <p>Status: <span id="status-charge">Idle</span> | Level: <span id="val-battery">0%</span></p>
            <label class="switch">
                <input type="checkbox" id="btn-charge" onchange="toggleCharge()">
                <span class="slider round"></span>
            </label>
        </div>
    </div>

    <div class="charts-container">
        <div class="chart-wrapper"><canvas id="chartElectrical"></canvas></div>
        <div class="chart-wrapper"><canvas id="chartThermal"></canvas></div>
    </div>

    <div class="history-container">
        <div class="history-header">
            <h3>Riwayat Data</h3>
            <button class="btn-download" onclick="downloadExcel()">Download Excel</button>
        </div>
        <div class="table-wrapper">
            <table id="historyTable">
                <thead>
                    <tr>
                        <th>Waktu</th>
                        <th>Volt (V)</th>
                        <th>Arus (A)</th>
                        <th>Suhu (°C)</th>
                        <th>Kipas</th>
                        <th>Baterai</th>
                    </tr>
                </thead>
                <tbody></tbody>
            </table>
        </div>
    </div>

    <div id="status">Status: Disconnected</div>
    <script src="script.js"></script>
</body>
</html>