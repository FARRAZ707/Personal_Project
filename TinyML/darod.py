import pandas as pd
import numpy as np
from datetime import datetime, timedelta

# Inisialisasi parameter
n_rows = 200
data = []
start_time = datetime.now()

for i in range(n_rows):
    # Simulasi kondisi normal vs anomali (10% peluang anomali)
    is_anomaly = np.random.random() < 0.1
    
    if is_anomaly:
        temp = np.random.uniform(85, 100)
        hum = np.random.uniform(10, 25)
        vib = np.random.uniform(5, 8)
        label = 1
    else:
        temp = np.random.uniform(40, 50)
        hum = np.random.uniform(35, 45)
        vib = np.random.uniform(1, 2)
        label = 0
        
    timestamp = start_time + timedelta(minutes=5*i)
    data.append([timestamp.strftime("%Y-%m-%d %H:%M:%S"), "S001", round(temp, 2), round(hum, 2), round(vib, 2), label])

# Simpan ke DataFrame
df = pd.DataFrame(data, columns=['Timestamp', 'Sensor_ID', 'Temperature', 'Humidity', 'Vibration', 'Label'])

# Export ke CSV
df.to_csv('sensor_data_full.csv', index=False)
print("File 'sensor_data_full.csv' berhasil dibuat dengan 200 baris.")