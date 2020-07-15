#include "helper.h"
#include <ESP8266Ping.h>
#include <RGBConverter.h>

// IP

String ip2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3]); 
}

bool isValidIP(const IPAddress& ipAddress) {
  return (ipAddress[0] > 0 && ipAddress[1] >= 0 && ipAddress[2] >= 0 && ipAddress[3] >=0 && ipAddress[0] < 255 && ipAddress[1] < 255 && ipAddress[2] < 255 && ipAddress[3] < 255);
}

bool wifi_check(char* ssid, char* pass) {

  static long attempt_connect_timer;
  if (WiFi.status() != WL_CONNECTED && millis() - attempt_connect_timer >= 10000) {
    attempt_connect_timer = millis();
    WiFi.begin(ssid, pass);
    Serial.print("Disconnected. Reconnecting... Connection Status: ");
    Serial.println(WiFi.status());
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  IPAddress ip = WiFi.localIP();
  if (!isValidIP(ip)) {
    Serial.print("No IP... Connection Status: ");
    Serial.println(WiFi.status());
    WiFi.disconnect();
    return false;
  }

  static long wifi_ping_timer;
  if (millis() - wifi_ping_timer >= 30000) {
    IPAddress ip (12, 34, 56, 201);
    bool ret = Ping.ping(ip);
    if (!ret) {
      ret = Ping.ping(ip);
    }
    
    wifi_ping_timer = millis();

    if (!ret) {
      Serial.print("Ping failed... Disconnecting...");
      WiFi.disconnect();
    }
    return ret;
  }
  return true;
}

// EspAlexa Callback

void deskLED_callback(uint8_t brightness, uint32_t rgb) {
  
  // digital LEDs
  if (brightness != 0) {
    FastLED.setBrightness(brightness);
  }

}

void musicLED_callback(uint8_t brightness) {
}

// Color

String colorinttohexstr(uint8_t color) {
  String a = String(color, HEX);
  if (a.length() == 0) {
    return "00";
  }

  if (a.length() == 1) {
    return "0" + a;
  }

  if (a.length() > 2) {
    return "00";
  }
  return a;
}

void setAnalogRGB(uint32_t rgb, uint8_t brightness, uint8_t REDPIN, uint8_t GREENPIN, uint8_t BLUEPIN) {
  uint8_t r, g, b;
  r = (rgb >> 16) && 0xFF;
  g = (rgb >> 8) && 0xFF;
  b = (rgb) && 0xFF;
  analogWrite(REDPIN, r * brightness / 255); // Red
  analogWrite(GREENPIN, g * brightness / 255); // Green
  analogWrite(BLUEPIN, b * brightness / 255); // Blue
}

void setAnalogRGB(uint8_t red, uint8_t blue, uint8_t green, uint8_t brightness, uint8_t REDPIN, uint8_t GREENPIN, uint8_t BLUEPIN) {
  analogWrite(REDPIN, red * brightness / 255); // Red
  analogWrite(GREENPIN, green * brightness / 255); // Green
  analogWrite(BLUEPIN, blue * brightness / 255); // Blue
}

void setAnalogRGB(const CRGB& rgb, uint8_t brightness, uint8_t REDPIN, uint8_t GREENPIN, uint8_t BLUEPIN) {
  analogWrite(REDPIN, rgb.r * brightness / 255); // Red
  analogWrite(GREENPIN, rgb.g * brightness / 255); // Green
  analogWrite(BLUEPIN, rgb.b * brightness / 255); // Blue
}

uint8_t rgbtopercent(uint8_t rgb) {
  switch(rgb) {
    case 0:
      return 0;
    case 1:
      return 0;
    case 2:
      return 1;
    case 3:
      return 1;
    case 4:
      return 2;
    case 5:
      return 2;
    case 6:
      return 2;
    case 7:
      return 3;
    case 8:
      return 3;
    case 9:
      return 4;
    case 10:
      return 4;
    case 11:
      return 4;
    case 12:
      return 5;
    case 13:
      return 5;
    case 14:
      return 5;
    case 15:
      return 6;
    case 16:
      return 6;
    case 17:
      return 7;
    case 18:
      return 7;
    case 19:
      return 7;
    case 20:
      return 8;
    case 21:
      return 8;
    case 22:
      return 9;
    case 23:
      return 9;
    case 24:
      return 9;
    case 25:
      return 10;
    case 26:
      return 10;
    case 27:
      return 11;
    case 28:
      return 11;
    case 29:
      return 11;
    case 30:
      return 12;
    case 31:
      return 12;
    case 32:
      return 13;
    case 33:
      return 13;
    case 34:
      return 13;
    case 35:
      return 14;
    case 36:
      return 14;
    case 37:
      return 15;
    case 38:
      return 15;
    case 39:
      return 15;
    case 40:
      return 16;
    case 41:
      return 16;
    case 42:
      return 16;
    case 43:
      return 17;
    case 44:
      return 17;
    case 45:
      return 18;
    case 46:
      return 18;
    case 47:
      return 18;
    case 48:
      return 19;
    case 49:
      return 19;
    case 50:
      return 20;
    case 51:
      return 20;
    case 52:
      return 20;
    case 53:
      return 21;
    case 54:
      return 21;
    case 55:
      return 22;
    case 56:
      return 22;
    case 57:
      return 22;
    case 58:
      return 23;
    case 59:
      return 23;
    case 60:
      return 24;
    case 61:
      return 24;
    case 62:
      return 24;
    case 63:
      return 25;
    case 64:
      return 25;
    case 65:
      return 25;
    case 66:
      return 26;
    case 67:
      return 26;
    case 68:
      return 27;
    case 69:
      return 27;
    case 70:
      return 27;
    case 71:
      return 28;
    case 72:
      return 28;
    case 73:
      return 29;
    case 74:
      return 29;
    case 75:
      return 29;
    case 76:
      return 30;
    case 77:
      return 30;
    case 78:
      return 31;
    case 79:
      return 31;
    case 80:
      return 31;
    case 81:
      return 32;
    case 82:
      return 32;
    case 83:
      return 33;
    case 84:
      return 33;
    case 85:
      return 33;
    case 86:
      return 34;
    case 87:
      return 34;
    case 88:
      return 35;
    case 89:
      return 35;
    case 90:
      return 35;
    case 91:
      return 36;
    case 92:
      return 36;
    case 93:
      return 36;
    case 94:
      return 37;
    case 95:
      return 37;
    case 96:
      return 38;
    case 97:
      return 38;
    case 98:
      return 38;
    case 99:
      return 39;
    case 100:
      return 39;
    case 101:
      return 40;
    case 102:
      return 40;
    case 103:
      return 40;
    case 104:
      return 41;
    case 105:
      return 41;
    case 106:
      return 42;
    case 107:
      return 42;
    case 108:
      return 42;
    case 109:
      return 43;
    case 110:
      return 43;
    case 111:
      return 44;
    case 112:
      return 44;
    case 113:
      return 44;
    case 114:
      return 45;
    case 115:
      return 45;
    case 116:
      return 45;
    case 117:
      return 46;
    case 118:
      return 46;
    case 119:
      return 47;
    case 120:
      return 47;
    case 121:
      return 47;
    case 122:
      return 48;
    case 123:
      return 48;
    case 124:
      return 49;
    case 125:
      return 49;
    case 126:
      return 49;
    case 127:
      return 50;
    case 128:
      return 50;
    case 129:
      return 51;
    case 130:
      return 51;
    case 131:
      return 51;
    case 132:
      return 52;
    case 133:
      return 52;
    case 134:
      return 53;
    case 135:
      return 53;
    case 136:
      return 53;
    case 137:
      return 54;
    case 138:
      return 54;
    case 139:
      return 55;
    case 140:
      return 55;
    case 141:
      return 55;
    case 142:
      return 56;
    case 143:
      return 56;
    case 144:
      return 56;
    case 145:
      return 57;
    case 146:
      return 57;
    case 147:
      return 58;
    case 148:
      return 58;
    case 149:
      return 58;
    case 150:
      return 59;
    case 151:
      return 59;
    case 152:
      return 60;
    case 153:
      return 60;
    case 154:
      return 60;
    case 155:
      return 61;
    case 156:
      return 61;
    case 157:
      return 62;
    case 158:
      return 62;
    case 159:
      return 62;
    case 160:
      return 63;
    case 161:
      return 63;
    case 162:
      return 64;
    case 163:
      return 64;
    case 164:
      return 64;
    case 165:
      return 65;
    case 166:
      return 65;
    case 167:
      return 65;
    case 168:
      return 66;
    case 169:
      return 66;
    case 170:
      return 67;
    case 171:
      return 67;
    case 172:
      return 67;
    case 173:
      return 68;
    case 174:
      return 68;
    case 175:
      return 69;
    case 176:
      return 69;
    case 177:
      return 69;
    case 178:
      return 70;
    case 179:
      return 70;
    case 180:
      return 71;
    case 181:
      return 71;
    case 182:
      return 71;
    case 183:
      return 72;
    case 184:
      return 72;
    case 185:
      return 73;
    case 186:
      return 73;
    case 187:
      return 73;
    case 188:
      return 74;
    case 189:
      return 74;
    case 190:
      return 75;
    case 191:
      return 75;
    case 192:
      return 75;
    case 193:
      return 76;
    case 194:
      return 76;
    case 195:
      return 76;
    case 196:
      return 77;
    case 197:
      return 77;
    case 198:
      return 78;
    case 199:
      return 78;
    case 200:
      return 78;
    case 201:
      return 79;
    case 202:
      return 79;
    case 203:
      return 80;
    case 204:
      return 80;
    case 205:
      return 80;
    case 206:
      return 81;
    case 207:
      return 81;
    case 208:
      return 82;
    case 209:
      return 82;
    case 210:
      return 82;
    case 211:
      return 83;
    case 212:
      return 83;
    case 213:
      return 84;
    case 214:
      return 84;
    case 215:
      return 84;
    case 216:
      return 85;
    case 217:
      return 85;
    case 218:
      return 85;
    case 219:
      return 86;
    case 220:
      return 86;
    case 221:
      return 87;
    case 222:
      return 87;
    case 223:
      return 87;
    case 224:
      return 88;
    case 225:
      return 88;
    case 226:
      return 89;
    case 227:
      return 89;
    case 228:
      return 89;
    case 229:
      return 90;
    case 230:
      return 90;
    case 231:
      return 91;
    case 232:
      return 91;
    case 233:
      return 91;
    case 234:
      return 92;
    case 235:
      return 92;
    case 236:
      return 93;
    case 237:
      return 93;
    case 238:
      return 93;
    case 239:
      return 94;
    case 240:
      return 94;
    case 241:
      return 95;
    case 242:
      return 95;
    case 243:
      return 95;
    case 244:
      return 96;
    case 245:
      return 96;
    case 246:
      return 96;
    case 247:
      return 97;
    case 248:
      return 97;
    case 249:
      return 98;
    case 250:
      return 98;
    case 251:
      return 98;
    case 252:
      return 99;
    case 253:
      return 99;
    case 254:
      return 100;
    case 255:
      return 100;
  }
  return 100;
}

uint8_t percenttorgb(uint8_t percent) {
  switch(percent) {
    case 0:
      return 0;
    case 1:
      return 3;
    case 2:
      return 5;
    case 3:
      return 8;
    case 4:
      return 10;
    case 5:
      return 13;
    case 6:
      return 15;
    case 7:
      return 18;
    case 8:
      return 20;
    case 9:
      return 23;
    case 10:
      return 26;
    case 11:
      return 28;
    case 12:
      return 31;
    case 13:
      return 33;
    case 14:
      return 36;
    case 15:
      return 38;
    case 16:
      return 41;
    case 17:
      return 43;
    case 18:
      return 46;
    case 19:
      return 48;
    case 20:
      return 51;
    case 21:
      return 54;
    case 22:
      return 56;
    case 23:
      return 59;
    case 24:
      return 61;
    case 25:
      return 64;
    case 26:
      return 66;
    case 27:
      return 69;
    case 28:
      return 71;
    case 29:
      return 74;
    case 30:
      return 77;
    case 31:
      return 79;
    case 32:
      return 82;
    case 33:
      return 84;
    case 34:
      return 87;
    case 35:
      return 89;
    case 36:
      return 92;
    case 37:
      return 94;
    case 38:
      return 97;
    case 39:
      return 99;
    case 40:
      return 102;
    case 41:
      return 105;
    case 42:
      return 107;
    case 43:
      return 110;
    case 44:
      return 112;
    case 45:
      return 115;
    case 46:
      return 117;
    case 47:
      return 120;
    case 48:
      return 122;
    case 49:
      return 125;
    case 50:
      return 128;
    case 51:
      return 130;
    case 52:
      return 133;
    case 53:
      return 135;
    case 54:
      return 138;
    case 55:
      return 140;
    case 56:
      return 143;
    case 57:
      return 145;
    case 58:
      return 148;
    case 59:
      return 150;
    case 60:
      return 153;
    case 61:
      return 156;
    case 62:
      return 158;
    case 63:
      return 161;
    case 64:
      return 163;
    case 65:
      return 166;
    case 66:
      return 168;
    case 67:
      return 171;
    case 68:
      return 173;
    case 69:
      return 176;
    case 70:
      return 179;
    case 71:
      return 181;
    case 72:
      return 184;
    case 73:
      return 186;
    case 74:
      return 189;
    case 75:
      return 191;
    case 76:
      return 194;
    case 77:
      return 196;
    case 78:
      return 199;
    case 79:
      return 201;
    case 80:
      return 204;
    case 81:
      return 207;
    case 82:
      return 209;
    case 83:
      return 212;
    case 84:
      return 214;
    case 85:
      return 217;
    case 86:
      return 219;
    case 87:
      return 222;
    case 88:
      return 224;
    case 89:
      return 227;
    case 90:
      return 230;
    case 91:
      return 232;
    case 92:
      return 235;
    case 93:
      return 237;
    case 94:
      return 240;
    case 95:
      return 242;
    case 96:
      return 245;
    case 97:
      return 247;
    case 98:
      return 250;
    case 99:
      return 252;
    case 100:
      return 255;
  }
  return 255;
}