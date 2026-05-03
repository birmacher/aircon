#include "display.h"

void displayStatus(const String& line1, const String& line2) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(line1);
    if (line2.length() > 0) {
        display.setTextSize(1);
        display.setCursor(0, 24);
        display.println(line2);
    }
    display.display();
}

void displayBig(const String& msg) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(3);
    display.setCursor(0, 16);
    display.println(msg);
    display.display();
}
