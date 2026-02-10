#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WebServer.h>
#include <Preferences.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Default credentials (used on first boot or after reset)
const String DEFAULT_SSID = "wifi";
const String DEFAULT_PASSWORD = "psswd";
const String DEFAULT_API_KEY = "apikey";

String ssid = DEFAULT_SSID;
String password = DEFAULT_PASSWORD;
String API_KEY = DEFAULT_API_KEY;
const char* MAX_TOKENS = "100";

Preferences preferences;
WebServer server(80);

// HTML page for chat
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Gemini AI Chat</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            width: 100%;
            max-width: 600px;
            overflow: hidden;
        }
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
            position: relative;
        }
        .header h1 {
            font-size: 28px;
            margin-bottom: 10px;
        }
        .header p {
            opacity: 0.9;
            font-size: 14px;
        }
        .settings-btn {
            position: absolute;
            top: 20px;
            right: 20px;
            background: rgba(255,255,255,0.2);
            border: 2px solid white;
            color: white;
            padding: 8px 16px;
            border-radius: 20px;
            cursor: pointer;
            font-size: 14px;
            text-decoration: none;
            transition: background 0.3s;
        }
        .settings-btn:hover {
            background: rgba(255,255,255,0.3);
        }
        .chat-container {
            height: 400px;
            overflow-y: auto;
            padding: 20px;
            background: #f8f9fa;
        }
        .message {
            margin-bottom: 15px;
            padding: 12px 16px;
            border-radius: 12px;
            max-width: 80%;
            animation: slideIn 0.3s ease;
        }
        @keyframes slideIn {
            from {
                opacity: 0;
                transform: translateY(10px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }
        .user-message {
            background: #667eea;
            color: white;
            margin-left: auto;
            text-align: right;
        }
        .ai-message {
            background: white;
            color: #333;
            border: 1px solid #e0e0e0;
        }
        .input-container {
            padding: 20px;
            background: white;
            border-top: 1px solid #e0e0e0;
        }
        .input-group {
            display: flex;
            gap: 10px;
        }
        input[type="text"] {
            flex: 1;
            padding: 12px 16px;
            border: 2px solid #e0e0e0;
            border-radius: 25px;
            font-size: 14px;
            outline: none;
            transition: border-color 0.3s;
        }
        input[type="text"]:focus {
            border-color: #667eea;
        }
        button {
            padding: 12px 30px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 25px;
            cursor: pointer;
            font-size: 14px;
            font-weight: 600;
            transition: transform 0.2s, box-shadow 0.2s;
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }
        button:active {
            transform: translateY(0);
        }
        button:disabled {
            background: #ccc;
            cursor: not-allowed;
            transform: none;
        }
        .loading {
            display: none;
            text-align: center;
            padding: 10px;
            color: #667eea;
        }
        .loading.active {
            display: block;
        }
        .spinner {
            border: 3px solid #f3f3f3;
            border-top: 3px solid #667eea;
            border-radius: 50%;
            width: 30px;
            height: 30px;
            animation: spin 1s linear infinite;
            margin: 0 auto;
        }
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <a href="/settings" class="settings-btn">⚙️ Settings</a>
            <h1>🤖 Slushi3 AI Chat</h1>
            <p>Powered by ESP32</p>
        </div>
        <div class="chat-container" id="chatContainer">
            <div class="message ai-message">
                👋 Hello! I'm Gemini AI. Ask me anything!
            </div>
        </div>
        <div class="loading" id="loading">
            <div class="spinner"></div>
            <p>Thinking...</p>
        </div>
        <div class="input-container">
            <div class="input-group">
                <input type="text" id="userInput" placeholder="Type your message..." onkeypress="if(event.key==='Enter') sendMessage()">
                <button onclick="sendMessage()" id="sendBtn">Send</button>
            </div>
        </div>
    </div>

    <script>
        function addMessage(message, isUser) {
            const chatContainer = document.getElementById('chatContainer');
            const messageDiv = document.createElement('div');
            messageDiv.className = 'message ' + (isUser ? 'user-message' : 'ai-message');
            messageDiv.textContent = message;
            chatContainer.appendChild(messageDiv);
            chatContainer.scrollTop = chatContainer.scrollHeight;
        }

        async function sendMessage() {
            const input = document.getElementById('userInput');
            const message = input.value.trim();
            
            if (!message) return;
            
            addMessage(message, true);
            input.value = '';
            
            const sendBtn = document.getElementById('sendBtn');
            const loading = document.getElementById('loading');
            
            sendBtn.disabled = true;
            loading.classList.add('active');
            
            try {
                const response = await fetch('/ask', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded',
                    },
                    body: 'message=' + encodeURIComponent(message)
                });
                
                const data = await response.text();
                addMessage(data, false);
            } catch (error) {
                addMessage('Error: Could not get response', false);
            }
            
            sendBtn.disabled = false;
            loading.classList.remove('active');
        }
    </script>
</body>
</html>
)rawliteral";

// HTML page for settings
const char settings_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Settings - Gemini AI</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            width: 100%;
            max-width: 600px;
            overflow: hidden;
        }
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
            position: relative;
        }
        .header h1 {
            font-size: 28px;
            margin-bottom: 10px;
        }
        .back-btn {
            position: absolute;
            top: 20px;
            left: 20px;
            background: rgba(255,255,255,0.2);
            border: 2px solid white;
            color: white;
            padding: 8px 16px;
            border-radius: 20px;
            cursor: pointer;
            font-size: 14px;
            text-decoration: none;
            transition: background 0.3s;
        }
        .back-btn:hover {
            background: rgba(255,255,255,0.3);
        }
        .settings-container {
            padding: 30px;
        }
        .form-group {
            margin-bottom: 25px;
        }
        label {
            display: block;
            margin-bottom: 8px;
            color: #333;
            font-weight: 600;
            font-size: 14px;
        }
        input[type="text"],
        input[type="password"] {
            width: 100%;
            padding: 12px 16px;
            border: 2px solid #e0e0e0;
            border-radius: 10px;
            font-size: 14px;
            outline: none;
            transition: border-color 0.3s;
        }
        input[type="text"]:focus,
        input[type="password"]:focus {
            border-color: #667eea;
        }
        .btn-group {
            display: flex;
            gap: 10px;
            margin-top: 30px;
        }
        button {
            flex: 1;
            padding: 14px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            font-size: 16px;
            font-weight: 600;
            transition: transform 0.2s, box-shadow 0.2s;
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }
        button:active {
            transform: translateY(0);
        }
        .alert {
            padding: 12px 16px;
            border-radius: 10px;
            margin-bottom: 20px;
            display: none;
        }
        .alert.success {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .alert.error {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        .alert.active {
            display: block;
        }
        .note {
            background: #fff3cd;
            color: #856404;
            padding: 12px 16px;
            border-radius: 10px;
            margin-top: 20px;
            font-size: 13px;
            border: 1px solid #ffeaa7;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <a href="/" class="back-btn">← Back</a>
            <h1>⚙️ Settings</h1>
        </div>
        <div class="settings-container">
            <div class="alert" id="alert"></div>
            
            <form id="settingsForm">
                <div class="form-group">
                    <label for="ssid">WiFi SSID (Network Name)</label>
                    <input type="text" id="ssid" name="ssid" placeholder="Enter WiFi name" required>
                </div>
                
                <div class="form-group">
                    <label for="password">WiFi Password</label>
                    <input type="password" id="password" name="password" placeholder="Enter WiFi password" required>
                </div>
                
                <div class="form-group">
                    <label for="apikey">Google Gemini API Key</label>
                    <input type="text" id="apikey" name="apikey" placeholder="Enter API key" required>
                </div>
                
                <div class="btn-group">
                    <button type="submit">💾 Save Settings</button>
                </div>
            </form>
            
            <div class="note">
                ⚠️ <strong>Note:</strong> After saving new WiFi settings, the device will restart and try to connect to the new network. Make sure your credentials are correct!
            </div>
        </div>
    </div>

    <script>
        // Load current settings on page load
        window.onload = async function() {
            try {
                const response = await fetch('/get-settings');
                const data = await response.json();
                document.getElementById('ssid').value = data.ssid;
                document.getElementById('password').value = data.password;
                document.getElementById('apikey').value = data.apikey;
            } catch (error) {
                console.error('Error loading settings:', error);
            }
        };

        document.getElementById('settingsForm').addEventListener('submit', async function(e) {
            e.preventDefault();
            
            const formData = new FormData(e.target);
            const data = new URLSearchParams(formData);
            
            const alert = document.getElementById('alert');
            
            try {
                const response = await fetch('/save-settings', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded',
                    },
                    body: data
                });
                
                const result = await response.text();
                
                if (response.ok) {
                    alert.className = 'alert success active';
                    alert.textContent = '✓ ' + result;
                    
                    setTimeout(() => {
                        alert.textContent += ' Device will restart in 3 seconds...';
                    }, 1000);
                } else {
                    alert.className = 'alert error active';
                    alert.textContent = '✗ ' + result;
                }
            } catch (error) {
                alert.className = 'alert error active';
                alert.textContent = '✗ Error saving settings';
            }
        });
    </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleSettings() {
  server.send(200, "text/html", settings_html);
}

void handleGetSettings() {
  String json = "{";
  json += "\"ssid\":\"" + ssid + "\",";
  json += "\"password\":\"" + password + "\",";
  json += "\"apikey\":\"" + API_KEY + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleSaveSettings() {
  if (server.hasArg("ssid") && server.hasArg("password") && server.hasArg("apikey")) {
    String new_ssid = server.arg("ssid");
    String new_password = server.arg("password");
    String new_apikey = server.arg("apikey");
    
    // Save to preferences
    preferences.begin("wifi-config", false);
    preferences.putString("ssid", new_ssid);
    preferences.putString("password", new_password);
    preferences.putString("apikey", new_apikey);
    preferences.end();
    
    Serial.println("Settings saved!");
    Serial.println("New SSID: " + new_ssid);
    Serial.println("New API Key: " + new_apikey);
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Settings Saved!");
    display.println("Restarting...");
    display.display();
    
    server.send(200, "text/plain", "Settings saved successfully!");
    
    delay(3000);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Missing parameters");
  }
}

void resetToDefault() {
  Serial.println("\n=============================");
  Serial.println("RESETTING TO DEFAULT SETTINGS");
  Serial.println("=============================");
  
  // Clear all preferences
  preferences.begin("wifi-config", false);
  preferences.clear();
  preferences.end();
  
  // Set back to default values
  ssid = DEFAULT_SSID;
  password = DEFAULT_PASSWORD;
  API_KEY = DEFAULT_API_KEY;
  
  Serial.println("Default SSID: " + DEFAULT_SSID);
  Serial.println("Default Password: " + DEFAULT_PASSWORD);
  Serial.println("Default API Key: " + DEFAULT_API_KEY);
  Serial.println("=============================");
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("RESET!");
  display.println("Default Settings");
  display.println("Restored");
  display.println("");
  display.println("Restarting...");
  display.display();
  
  delay(3000);
  ESP.restart();
}

String askGemini(String user_statement) {
  String user_statement_json = "\"Answer in exactly 3 words:" + user_statement + "\"";
  
  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
    client->setInsecure();
    
    HTTPClient https;
    if (https.begin(*client, "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=" + API_KEY)) {

      https.addHeader("Content-Type", "application/json");
      String payload = String("{\"contents\": [{\"parts\":[{\"text\":" + user_statement_json + "}]}],\"generationConfig\": {\"maxOutputTokens\": " + (String)MAX_TOKENS + "}}");

      int httpCode = https.POST(payload);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
        String response = https.getString();

        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, response);
        
        if (error) {
          delete client;
          https.end();
          return "Error parsing response";
        }
        
        String Answer = doc["candidates"][0]["content"]["parts"][0]["text"];
        
        https.end();
        delete client;
        return Answer;
        
      } else {
        https.end();
        delete client;
        return "Error: " + String(httpCode);
      }
    }
    delete client;
  }
  return "Connection failed";
}

void handleAsk() {
  if (server.hasArg("message")) {
    String message = server.arg("message");
    
    // Display on OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("Web Query:");
    display.println(message);
    display.display();
    
    Serial.println("Web User: " + message);
    
    String response = askGemini(message);
    
    // Display response on OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Response:");
    display.println(response.substring(0, 100));
    display.display();
    
    Serial.println("Response: " + response);
    
    server.send(200, "text/plain", response);
  } else {
    server.send(400, "text/plain", "Missing message parameter");
  }
}

void loadSettings() {
  preferences.begin("wifi-config", true);
  
  String stored_ssid = preferences.getString("ssid", "");
  String stored_password = preferences.getString("password", "");
  String stored_apikey = preferences.getString("apikey", "");
  
  if (stored_ssid.length() > 0) {
    ssid = stored_ssid;
    password = stored_password;
    API_KEY = stored_apikey;
    Serial.println("Loaded settings from memory");
  } else {
    Serial.println("Using default settings");
  }
  
  preferences.end();
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connecting to:");
  display.println(ssid);
  display.display();
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    Serial.print('.');
    delay(1000);
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WiFi Failed!");
    display.println("Check settings");
    display.display();
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();
  
  // Load settings from preferences
  loadSettings();
  
  initWiFi();
  
  // Setup web server
  server.on("/", handleRoot);
  server.on("/settings", handleSettings);
  server.on("/get-settings", handleGetSettings);
  server.on("/save-settings", HTTP_POST, handleSaveSettings);
  server.on("/ask", HTTP_POST, handleAsk);
  server.begin();
  
  Serial.println("Gemini AI Web Server started!");
  Serial.print("Access at: http://");
  Serial.println(WiFi.localIP());
  Serial.println("\n*** Type 'reset' in Serial Monitor to restore default settings ***\n");
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Server Ready!");
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
}

void loop() {
  server.handleClient();
  
  // Keep serial functionality
  if (Serial.available()) {
    String user_statement = "";
    while (Serial.available()) {    
      char c = Serial.read();
      user_statement += c;
    }
    int len = user_statement.length();
    user_statement = user_statement.substring(0, (len - 1));
    
    if (user_statement.length() > 0) {
      // Check if user typed "reset"
      user_statement.trim(); // Remove any whitespace
      user_statement.toLowerCase(); // Convert to lowercase for comparison
      
      if (user_statement == "reset") {
        resetToDefault();
        return; // Exit loop as device will restart
      }
      
      // Normal query processing
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.println("Serial Query:");
      display.println(user_statement);
      display.display();
      
      Serial.println("Serial User: " + user_statement);
      
      String response = askGemini(user_statement);
      
      Serial.println("Gemini AI Response: ");      
      Serial.println(response);
      
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Response:");
      display.println(response.substring(0, 100));
      display.display();
      
      Serial.println("____________________________________________________________________________");
    }
  }
  
  delay(10);
}
