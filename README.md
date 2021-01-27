# CO2-Raumluftwaechter
Der CO2-Raumluftwächter misst mit Hilfe des SCD30 von Sensirion den CO2-Gehalt der Raumluft und setz optisch die CO2-Konzentration um, so dass in Klassenräumen bedarfsgerechter gelüftet werden können. Optional kann der CO2-Raumluftwächter mit einer Echtzeituhr DS3231 (Batteriegepuffert) und SD-Card-Slot und/oder OLED-Display ausgestattet werden. Per Arduino Nano V3.0 werden die Messwerte vom SCD30 abgerufen und können auf SD-Card protokolliert wie auch in Echtzeit auf einem OLED-Display ausgegeben werden.
# Hardware Bestandteile
* Trägerplatine
* Arduino nano V3
* SCD30
* WS2812b LED Ring mit 16 LEDs als Anzeige
* (optional) SD-Card Slot mit Echtzeituhr DS3231 mit Batteriepufferung CR1220
* (optional) I2C OLED Display 128 x 64 (VCC GND vertauschbar per Lötbrücke)
