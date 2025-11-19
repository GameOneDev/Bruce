/**
 * @file ui_handler.cpp
 * @brief UI handler implementation
 */

#include "ui_handler.h"

UIHandler::UIHandler() :
    currentState(UI_DISCONNECTED),
    lastUpdate(0),
    callStartTime(0),
    btnAPressed(false),
    btnBPressed(false),
    btnPwrPressed(false),
    btnALastPress(0),
    btnBLastPress(0) {
}

bool UIHandler::begin() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(ROTATION);
    M5.Display.setBrightness(128);
    
    clearScreen();
    drawHeader("SIP Client");
    
    Serial.println("UI initialized");
    return true;
}

void UIHandler::updateStatus(UIState state, const char* message) {
    if (state == currentState && message == nullptr) return;
    
    currentState = state;
    
    clearScreen();
    
    switch (state) {
        case UI_DISCONNECTED:
            drawHeader("Disconnected");
            drawCenteredText("WiFi not connected", SCREEN_HEIGHT / 2, COLOR_ERROR);
            break;
            
        case UI_CONNECTING_WIFI:
            drawHeader("Connecting...");
            drawCenteredText("Connecting to WiFi", SCREEN_HEIGHT / 2, COLOR_WARNING);
            break;
            
        case UI_CONNECTED_WIFI:
            drawHeader("WiFi Connected");
            if (message) {
                drawCenteredText(message, SCREEN_HEIGHT / 2, COLOR_STATUS);
            }
            break;
            
        case UI_REGISTERING:
            drawHeader("Registering");
            drawCenteredText("Registering with", 40, COLOR_TEXT);
            drawCenteredText("SIP server...", 60, COLOR_TEXT);
            break;
            
        case UI_REGISTERED:
            drawHeader("Ready");
            drawCenteredText("Registered", 40, COLOR_STATUS);
            drawCenteredText("Press A to call", 60, COLOR_TEXT);
            drawCenteredText("Default number", 80, COLOR_TEXT);
            break;
            
        case UI_CALLING:
            drawHeader("Calling...");
            drawCenteredText("Calling", 50, COLOR_CALLING);
            if (message) {
                drawCenteredText(message, 70, COLOR_TEXT);
            }
            break;
            
        case UI_RINGING:
            drawHeader("Ringing");
            drawCenteredText("Call is ringing...", 50, COLOR_CALLING);
            drawCenteredText("Waiting for answer", 70, COLOR_TEXT);
            break;
            
        case UI_IN_CALL:
            drawHeader("In Call");
            if (callStartTime == 0) {
                callStartTime = millis();
            }
            showCallIndicator(true);
            break;
            
        case UI_ERROR:
            drawHeader("Error");
            if (message) {
                drawCenteredText(message, SCREEN_HEIGHT / 2, COLOR_ERROR);
            }
            break;
    }
    
    drawStatusBar();
    lastUpdate = millis();
}

void UIHandler::updateCallInfo(const char* callInfo) {
    if (currentState != UI_IN_CALL) return;
    
    M5.Display.fillRect(0, 30, SCREEN_WIDTH, 60, COLOR_BG);
    drawCenteredText(callInfo, 50, COLOR_TEXT);
}

void UIHandler::updateError(const char* error) {
    M5.Display.fillRect(0, SCREEN_HEIGHT - 30, SCREEN_WIDTH, 30, COLOR_BG);
    M5.Display.setTextColor(COLOR_ERROR);
    M5.Display.setCursor(5, SCREEN_HEIGHT - 25);
    M5.Display.print(error);
}

void UIHandler::displayWiFiStatus(bool connected, const char* ip) {
    if (connected) {
        updateStatus(UI_CONNECTED_WIFI, ip);
    } else {
        updateStatus(UI_DISCONNECTED);
    }
}

void UIHandler::displaySIPStatus(const char* status) {
    M5.Display.fillRect(0, 20, SCREEN_WIDTH, 20, COLOR_BG);
    M5.Display.setTextColor(COLOR_STATUS);
    M5.Display.setCursor(5, 20);
    M5.Display.print("SIP: ");
    M5.Display.print(status);
}

void UIHandler::displayCallStatus(const char* status, const char* duration) {
    if (currentState != UI_IN_CALL) return;
    
    // Update duration
    unsigned long elapsed = (millis() - callStartTime) / 1000;
    String durationStr = formatCallDuration(elapsed);
    
    M5.Display.fillRect(0, 30, SCREEN_WIDTH, 60, COLOR_BG);
    drawCenteredText(status, 40, COLOR_TEXT);
    drawCenteredText(durationStr.c_str(), 60, COLOR_STATUS);
    
    // Show controls
    M5.Display.setTextColor(COLOR_TEXT);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(5, SCREEN_HEIGHT - 20);
    M5.Display.print("A:Mute B:Hangup");
}

void UIHandler::checkButtons() {
    M5.update();
    
    unsigned long now = millis();
    
    // Button A - Call/Answer
    if (M5.BtnA.wasPressed()) {
        if (now - btnALastPress > BUTTON_DEBOUNCE_MS) {
            btnAPressed = true;
            btnALastPress = now;
#if DEBUG_UI
            Serial.println("Button A pressed");
#endif
        }
    }
    
    // Button B - Hang up
    if (M5.BtnB.wasPressed()) {
        if (now - btnBLastPress > BUTTON_DEBOUNCE_MS) {
            btnBPressed = true;
            btnBLastPress = now;
#if DEBUG_UI
            Serial.println("Button B pressed");
#endif
        }
    }
    
    // Power button
    if (M5.BtnPWR.wasPressed()) {
        btnPwrPressed = true;
#if DEBUG_UI
        Serial.println("Power button pressed");
#endif
    }
}

bool UIHandler::isButtonAPressed() {
    if (btnAPressed) {
        btnAPressed = false;
        return true;
    }
    return false;
}

bool UIHandler::isButtonBPressed() {
    if (btnBPressed) {
        btnBPressed = false;
        return true;
    }
    return false;
}

void UIHandler::showCallIndicator(bool active) {
    static bool ledState = false;
    static unsigned long lastBlink = 0;
    
    if (active && millis() - lastBlink > 500) {
        ledState = !ledState;
        lastBlink = millis();
        
        // Draw indicator on screen
        int x = SCREEN_WIDTH - 15;
        int y = 5;
        M5.Display.fillCircle(x, y, 5, ledState ? COLOR_STATUS : COLOR_BG);
    }
}

void UIHandler::clearScreen() {
    M5.Display.fillScreen(COLOR_BG);
}

void UIHandler::drawHeader(const char* title) {
    M5.Display.fillRect(0, 0, SCREEN_WIDTH, 20, 0x0010);  // Dark blue header
    M5.Display.setTextColor(COLOR_TEXT);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(5, 2);
    M5.Display.print(title);
}

void UIHandler::drawStatusBar() {
    // Draw bottom status bar
    M5.Display.fillRect(0, SCREEN_HEIGHT - 15, SCREEN_WIDTH, 15, 0x0010);
    M5.Display.setTextColor(COLOR_TEXT);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(5, SCREEN_HEIGHT - 12);
    
    // Show button hints based on state
    switch (currentState) {
        case UI_REGISTERED:
            M5.Display.print("A:Call B:Menu");
            break;
        case UI_RINGING:
            M5.Display.print("A:Answer B:Reject");
            break;
        case UI_IN_CALL:
            M5.Display.print("B:Hangup");
            break;
        default:
            M5.Display.print("M5StickC Plus2");
            break;
    }
}

void UIHandler::drawCenteredText(const char* text, int y, uint16_t color) {
    M5.Display.setTextColor(color);
    M5.Display.setTextSize(1);
    
    int16_t x1, y1;
    uint16_t w, h;
    M5.Display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    
    int x = (SCREEN_WIDTH - w) / 2;
    M5.Display.setCursor(x, y);
    M5.Display.print(text);
}

String UIHandler::formatCallDuration(unsigned long seconds) {
    unsigned long hours = seconds / 3600;
    unsigned long minutes = (seconds % 3600) / 60;
    unsigned long secs = seconds % 60;
    
    char buffer[16];
    if (hours > 0) {
        sprintf(buffer, "%02lu:%02lu:%02lu", hours, minutes, secs);
    } else {
        sprintf(buffer, "%02lu:%02lu", minutes, secs);
    }
    
    return String(buffer);
}
