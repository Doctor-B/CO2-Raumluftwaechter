# CO2-Raumluftwaechter
Der CO2-Raumluftwächter misst mit Hilfe des SCD30 von Sensirion den CO2-Gehalt der Raumluft und setz optisch die CO2-Konzentration um, so dass in Klassenräumen bedarfsgerechter gelüftet werden können. Optional kann der CO2-Raumluftwächter mit einer Echtzeituhr DS3231, SD-Card-Slot und OLED-Display ausgestattet werden. Per Arduino Nano V3.0 werden die Messwerte vom SCD30 abgerufen und können auf SD-Card protokolliert wie auch in Echtzeit auf einem OLED-Display ausgegeben werden.
# Hardware Bestandteile
* PCB
* Arduino nano V3
* SCD30
* (optional) SD-Card Slot mit RTC DS3231
* (optional) I2C OLED Display 128 x 64 (VCC GND vertauschbar per Lötbrücke)
