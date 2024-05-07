﻿# Weather-Station
For now the app reads data from AHT20, BMP280 sensors and GPS. Then the data is transfered to via GSM to API and stored. <br>
The repo has changed it's files so now the main code once per 30s call an Http post api endpoint (the microcontroller goes to STANDBY mode between calls). <br>
The sim800L dosen't have sufficient TLS encryption, so I created this API that takes http calls and will call https API.<br>
[Link to gateway api used in the project](https://github.com/kacperugorny1/GatewayApi)<br>
TODO:
- Low energy - read about sim800l sleep modes and implement it.
<br><br>
Assumptions:
- GPS
- GSM
- Low energy
