# Sniff
A credit card-sized device, wall-mounted for monitoring hazardous gases in public washrooms. Shows result on smartphone by scanning a QR code.



### About

Started as a challenge and a fun project to convince authorities of how much our public washrooms are (silently/gaseously) dirty, using the scientific methods of "gas concentration measurement" and "gas sampling."

In the future, it could come up as a standard product or norm in public washrooms or act as a qualitative indicator.

Public toilets often face significant challenges due to the accumulation of hazardous gases such as ammonia, hydrogen sulfide, and methane. These gases can originate from various sources, including human waste, poor ventilation, and inadequate maintenance. 

These issues contribute to unsanitary conditions, making public toilets unpleasant and unsafe for users. Effective monitoring systems can significantly enhance public health and safety.


### Sensors

* mq2 - Methane (Natural Gas) 
* mq135 - Air Quality (CO2) 
* mq136 - H2S (Hydrogen Sulfide) 
* mq137 - NH3 (Ammonia)
* BME280 - Temp + Humidity + Pressure

Note: MQ series datasheet often includes correction curves for various humidity levels (e.g., 33%, 85%) at 20°C.
| Feature                      | Without Temp/Humid | With Temp/Humid |
| ---------------------------- | ------------------ | --------------- |
| Raw gas detection            | ✅                  | ✅               |
| Accurate ppm estimation      | ❌                  | ✅               |
| Environmental robustness     | ❌                  | ✅               |
| Scientific or industrial use | ❌                  | ✅               |


### MCU
  * ESP 32

### Power
* 5 Volts, 1 Amp - USB suply 
