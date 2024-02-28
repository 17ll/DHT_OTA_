# DHT_OTA

## Description
 
 - Reads data from DHT sensor
 - If values are received OTA, those are shown 

 ## Note
 
- Make sure you have setup ESP-IDF and ESP-ADF including environmental variables, prerequisites and tools.
- Make sure to use corect IP address. Chanhe the wifi and password credentials. 
- Send the followng curl command. You can use any value corrsponding to h and t
 ```  
curl -d 'h:10|t:40' -H 'Content-Type:text-plain' -X POST http://192.168.230.246/post

```
Click links to setup->
[ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html)
[ESP-ADF ](https://espressif-docs.readthedocs-hosted.com/projects/esp-adf/en/latest/get-started/index.html)
- Supported ESP-IDF versions are [v3.3, v4.0, v4.1, v4.2, v4.3 and v4.4]

