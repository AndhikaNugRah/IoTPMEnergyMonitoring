# IoT PM EnergyMonitoring
This project provides a flexible and scalable solution for monitoring and analyzing power consumption in industrial environments. I uploaded three master programs to facilitate three popular Schneider power meter models (PM800, PM5350, and PM5560). 

In my case, I only used Active Power and Reactive Power as parameters. I have also provided a register list that can be used to adjust the program based on the desired parameters to be stored in the database.

Using the ESP32 microcontroller, this system efficiently collects power data using RS-485 and Modbus RTU protocols. The collected data is then transmitted via MQTT for real-time monitoring and analysis. This approach significantly improves operational efficiency, enables proactive maintenance, and supports data-driven decision-making.
