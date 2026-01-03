// --- KONFIGURASI MQTT ---
const broker = "broker.emqx.io";
const port = 8083;
const topic_data = "proyek/solar_panel/data"; 
const topic_control = "proyek/solar_panel/control"; // Topik baru untuk kirim perintah

const clientID = "WebClient-" + Math.random().toString(16).substr(2, 8);
const client = new Paho.MQTT.Client(broker, port, "/mqtt", clientID);

let dataHistory = []; 

// Setup Grafik (Sama seperti sebelumnya, disingkat disini)
const ctx1 = document.getElementById('chartElectrical').getContext('2d');
const chartElectrical = new Chart(ctx1, { /* ...Config Grafik Listrik... */ 
    type: 'line', data: { labels: [], datasets: [{ label: 'Tegangan', borderColor: '#f39c12', data: [] }, { label: 'Arus', borderColor: '#3498db', data: [] }] } 
});
const ctx2 = document.getElementById('chartThermal').getContext('2d');
const chartThermal = new Chart(ctx2, { /* ...Config Grafik Suhu... */ 
    type: 'line', data: { labels: [], datasets: [{ label: 'Suhu Panel', borderColor: '#e74c3c', data: [] }, { label: 'Suhu Box', borderColor: '#9b59b6', data: [] }] } 
});

// --- LOGIKA UTAMA ---

client.onConnectionLost = function (responseObject) {
    document.getElementById("status").innerText = "Status: Disconnected";
    document.getElementById("status").style.backgroundColor = "#e74c3c";
};

client.onMessageArrived = function (message) {
    try {
        const data = JSON.parse(message.payloadString);
        const now = new Date().toLocaleTimeString();

        // 1. Update Monitoring Cards
        document.getElementById("val-voltage").innerText = data.voltage.toFixed(2) + " V";
        document.getElementById("val-current").innerText = data.current.toFixed(2) + " A";
        document.getElementById("val-temp-panel").innerText = data.tempPanel.toFixed(2) + " °C";
        document.getElementById("val-temp-box").innerText = data.tempBox.toFixed(2) + " °C";

        // 2. Update CONTROL UI (Sinkronisasi dengan status asli ESP32)
        // Kipas
        const fanText = document.getElementById("status-fan");
        const fanIcon = document.getElementById("icon-fan");
        // Kita tidak update toggle button secara paksa agar user tidak bingung saat klik, 
        // tapi update teks statusnya saja.
        if(data.fanStatus === "ON") {
            fanText.innerText = "ON (Active)";
            fanText.style.color = "#2ecc71";
            fanIcon.classList.add("spin"); // Efek muter
        } else {
            fanText.innerText = "OFF";
            fanText.style.color = "#ccc";
            fanIcon.classList.remove("spin");
        }

        // Baterai
        document.getElementById("status-charge").innerText = data.chargeStatus;
        document.getElementById("val-battery").innerText = data.batteryLevel.toFixed(1) + "%";

        // 3. Update Grafik
        updateChart(chartElectrical, now, [data.voltage, data.current]);
        updateChart(chartThermal, now, [data.tempPanel, data.tempBox]);

        // 4. Update History
        addToHistory(now, data);

    } catch (e) { console.error(e); }
};

// --- FUNGSI KONTROL (KIRIM PERINTAH) ---
function toggleFan() {
    const isChecked = document.getElementById("btn-fan").checked;
    const command = isChecked ? "FAN_ON" : "FAN_OFF";
    sendMessage(command);
}

function toggleCharge() {
    const isChecked = document.getElementById("btn-charge").checked;
    const command = isChecked ? "CHARGE_ON" : "CHARGE_OFF";
    sendMessage(command);
}

function sendMessage(msg) {
    message = new Paho.MQTT.Message(msg);
    message.destinationName = topic_control;
    client.send(message);
    console.log("Perintah dikirim: " + msg);
}

// Helper Functions (Grafik & Excel) tetap sama...
function updateChart(chart, label, dataArr) {
    chart.data.labels.push(label);
    chart.data.datasets.forEach((ds, i) => ds.data.push(dataArr[i]));
    if(chart.data.labels.length > 20) {
        chart.data.labels.shift();
        chart.data.datasets.forEach(ds => ds.data.shift());
    }
    chart.update();
}

function addToHistory(time, data) {
    dataHistory.push({
        Waktu: time, 
        Volt: data.voltage, 
        Arus: data.current, 
        Kipas: data.fanStatus, 
        Baterai: data.batteryLevel 
    });
    
    const row = `<tr>
        <td>${time}</td>
        <td>${data.voltage.toFixed(2)}</td>
        <td>${data.current.toFixed(2)}</td>
        <td>${data.tempPanel.toFixed(2)}</td>
        <td>${data.fanStatus}</td>
        <td>${data.batteryLevel.toFixed(1)}%</td>
    </tr>`;
    document.querySelector("#historyTable tbody").insertAdjacentHTML('afterbegin', row);
}

function downloadExcel() {
    const ws = XLSX.utils.json_to_sheet(dataHistory);
    const wb = XLSX.utils.book_new();
    XLSX.utils.book_append_sheet(wb, ws, "Data");
    XLSX.writeFile(wb, "Solar_Data.xlsx");
}

function connectMQTT() {
    client.connect({
        onSuccess: function () {
            document.getElementById("status").innerText = "Connected";
            document.getElementById("status").style.backgroundColor = "#2ecc71";
            client.subscribe(topic_data);
        },
        onFailure: function (e) { console.log(e); }
    });
}
connectMQTT();