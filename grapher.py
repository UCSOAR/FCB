import re
import matplotlib.pyplot as plt

# ---------- Load File ----------
with open("flash_dump.txt", "r", encoding="utf-8", errors="ignore") as f:
    lines = f.readlines()

# ---------- Storage ----------
imu16_time = []
imu16_gx = []
imu16_gy = []
imu16_gz = []
imu16_ax = []
imu16_ay = []
imu16_az = []

imu32_time = []
imu32_gx = []
imu32_gy = []
imu32_gz = []
imu32_ax = []
imu32_ay = []
imu32_az = []

baro07_time = []
baro07_pressure = []

baro11_time = []
baro11_pressure = []

# ---------- Regex ----------
imu_pattern = re.compile(
    r"(IMU\d+G)\(ID=\d+\) Timestamp=(\d+).*Accel=\[(-?\d+),(-?\d+),(-?\d+)\].*Gyro=\[(-?\d+),(-?\d+),(-?\d+)\]"
)

baro_pattern = re.compile(
    r"(BARO\d+)\(ID=\d+\) Timestamp=(\d+) Pressure=(-?\d+)"
)

# ---------- Parse ----------
for line in lines:
    imu_match = imu_pattern.search(line)
    if imu_match:
        sensor = imu_match.group(1)
        ts = int(imu_match.group(2))

        ax = int(imu_match.group(3))
        ay = int(imu_match.group(4))
        az = int(imu_match.group(5))

        gx = int(imu_match.group(6))
        gy = int(imu_match.group(7))
        gz = int(imu_match.group(8))

        if sensor == "IMU16G":
            imu16_time.append(ts)
            imu16_ax.append(ax)
            imu16_ay.append(ay)
            imu16_az.append(az)
            imu16_gx.append(gx)
            imu16_gy.append(gy)
            imu16_gz.append(gz)

        elif sensor == "IMU32G":
            imu32_time.append(ts)
            imu32_ax.append(ax)
            imu32_ay.append(ay)
            imu32_az.append(az)
            imu32_gx.append(gx)
            imu32_gy.append(gy)
            imu32_gz.append(gz)

    baro_match = baro_pattern.search(line)
    if baro_match:
        sensor = baro_match.group(1)
        ts = int(baro_match.group(2))
        pressure = int(baro_match.group(3))

        if sensor == "BARO07":
            baro07_time.append(ts)
            baro07_pressure.append(pressure)

        elif sensor == "BARO11":
            baro11_time.append(ts)
            baro11_pressure.append(pressure)

# ---------- IMU16G Gyro ----------
plt.figure()
plt.plot(imu16_time, imu16_gx)
plt.plot(imu16_time, imu16_gy)
plt.plot(imu16_time, imu16_gz)
plt.title("IMU16G Gyro vs Time")
plt.xlabel("Timestamp")
plt.ylabel("Gyro")
plt.show()

# ---------- IMU16G Accel ----------
plt.figure()
plt.plot(imu16_time, imu16_ax)
plt.plot(imu16_time, imu16_ay)
plt.plot(imu16_time, imu16_az)
plt.title("IMU16G Accel vs Time")
plt.xlabel("Timestamp")
plt.ylabel("Accel")
plt.show()

# ---------- IMU32G Gyro ----------
plt.figure()
plt.plot(imu32_time, imu32_gx)
plt.plot(imu32_time, imu32_gy)
plt.plot(imu32_time, imu32_gz)
plt.title("IMU32G Gyro vs Time")
plt.xlabel("Timestamp")
plt.ylabel("Gyro")
plt.show()

# ---------- IMU32G Accel ----------
plt.figure()
plt.plot(imu32_time, imu32_ax)
plt.plot(imu32_time, imu32_ay)
plt.plot(imu32_time, imu32_az)
plt.title("IMU32G Accel vs Time")
plt.xlabel("Timestamp")
plt.ylabel("Accel")
plt.show()

# ---------- BARO07 ----------
plt.figure()
plt.plot(baro07_time, baro07_pressure)
plt.title("BARO07 Pressure vs Time")
plt.xlabel("Timestamp")
plt.ylabel("Pressure")
plt.show()

# ---------- BARO11 ----------
plt.figure()
plt.plot(baro11_time, baro11_pressure)
plt.title("BARO11 Pressure vs Time")
plt.xlabel("Timestamp")
plt.ylabel("Pressure")
plt.show()