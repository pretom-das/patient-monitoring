import serial
import matplotlib.pyplot as plt
import pandas as pd
import time

# Change this to your actual serial port (e.g., "COM10" or "/dev/ttyUSB0")
SERIAL_PORT = "COM10"
BAUD_RATE = 9600

# Attempt to connect to the serial port
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
except serial.serialutil.SerialException as e:
    print(f"Error opening serial port: {e}")
    exit()

time.sleep(2)  # Allow time for the connection to stabilize

data_list = []       # To store timestamp and raw serial data
sensor_values = []   # To store numeric sensor values for plotting

# Set up the live plot
plt.ion()  # Enable interactive mode
fig, ax = plt.subplots()
line, = ax.plot([], [], '-o', markersize=4)
ax.set_xlabel("Sample Number")
ax.set_ylabel("Sensor Value")
ax.set_title("Live Arduino Sensor Data")
ax.set_ylim(0, 1023)  # Assuming analogRead values range from 0 to 1023

print("Reading data... Press Ctrl+C to stop.")

try:
    while True:
        raw_line = ser.readline().decode('utf-8').strip()
        if raw_line:  # Only process non-empty lines
            timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
            data_list.append([timestamp, raw_line])
            
            try:
                # Try to convert the serial input to a float (for numeric data)
                sensor_value = float(raw_line)
                sensor_values.append(sensor_value)
                
                # Update the plot with the new sensor value
                line.set_data(range(len(sensor_values)), sensor_values)
                ax.relim()             # Recalculate limits
                ax.autoscale_view()    # Auto-scale the view to the data
                plt.draw()
                plt.pause(0.01)
            except ValueError:
                # If the value is non-numeric (for example, if it's a '!' indicator), skip plotting
                print(f"Non-numeric data received: {raw_line}")
except KeyboardInterrupt:
    print("\nStopping data collection...")

finally:
    ser.close()
    # Save the collected data to an Excel file using pandas
    df = pd.DataFrame(data_list, columns=["Timestamp", "Sensor Value"])
    df.to_excel("arduino_data.xlsx", index=False)
    print("Data saved to arduino_data.xlsx")
    
    # Turn off interactive plotting and show the final plot
    plt.ioff()
    plt.show()