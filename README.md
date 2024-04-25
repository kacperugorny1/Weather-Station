# Weather-Station
For now the app reads data from AHT20, BMP280 sensors and GPS. Then the data is transfered to UART. <br>
The repo has changed it's files so now the main code once per 30s call an Http post api endpoint (the microcontroller goes to STANDBY mode between calls). <br>
TODO:
- Low energy - read about sim800l sleep modes and implement it.
- Add sensors readings and add those to api.
<br><br>
Assumptions:
- GPS
- GSM
- Low energy
