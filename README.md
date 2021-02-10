# CO2-Raumluftwaechter
Der CO2-Raumluftwächter misst mit Hilfe des SCD30 von Sensirion den CO2-Gehalt der Raumluft und setz optisch die CO2-Konzentration um, so dass in Klassenräumen bedarfsgerechter gelüftet werden können. Optional kann der CO2-Raumluftwächter mit einer Echtzeituhr DS3231 (Batteriegepuffert) und SD-Card-Slot und/oder OLED-Display ausgestattet werden. Per Arduino Nano V3.0 werden die Messwerte vom SCD30 abgerufen und können auf SD-Card protokolliert wie auch in Echtzeit auf einem OLED-Display ausgegeben werden.
# Hardware Bestandteile
* Trägerplatine
* Arduino nano V3 _[THT]_
* SCD30 _[THT]_
* WS2812b LED Ring mit 16 LEDs als Anzeige _[SMD]_
* Levelshifter BSS138BKSH _[SMD]_
* (optional) SD-Card Slot mit Echtzeituhr DS3231 mit Batteriepufferung CR1220 _[SMD]_
* (optional) I2C OLED Display 128 x 64 (VCC GND vertauschbar per Lötbrücke) _[THT]_

# License

The hardware of this repository is released under the following license:

- the "Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License" (CC BY-NC-SA 4.0) full text of this license is included in the [LICENSE.CC_NC-BY-SA-4.0](https://github.com/Doctor-B/CO2-Raumluftwaechter/main/LICENSE.CC_NC-BY-SA) file and a copy can also be found at https://creativecommons.org/licenses/by-nc-sa/4.0/

The software of this repository is released under the MIT license:

- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.