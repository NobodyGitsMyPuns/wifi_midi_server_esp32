#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "config.h"  // Include the config.h file
WebServer server(80);  // Create a web server object that listens on port 80

void setup() {
  Serial.begin(115200);

  // Initialize LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  // Connect to Wi-Fi
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  // Route to handle file listing
  server.on("/files", HTTP_GET, []() {
    String fileList = "";
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file) {
      fileList += String(file.name()) + "\n";
      file = root.openNextFile();
    }
    fileList += "Available Memory: " + String(LittleFS.totalBytes() - LittleFS.usedBytes()) + " bytes\n";
    server.send(200, "text/plain", fileList);
  });

  // Route to handle file upload with binary data
  server.on("/upload", HTTP_POST, []() {
    if (server.hasArg("filename")) {
      String filename = "/" + server.arg("filename");

      Serial.printf("Attempting to create file: %s\n", filename.c_str());

      // Check the available space before creating the file
      size_t totalBytes = LittleFS.totalBytes();
      size_t usedBytes = LittleFS.usedBytes();
      size_t freeBytes = totalBytes - usedBytes;

      Serial.printf("Total LittleFS space: %u bytes\n", totalBytes);
      Serial.printf("Used LittleFS space: %u bytes\n", usedBytes);
      Serial.printf("Free LittleFS space: %u bytes\n", freeBytes);

      if (freeBytes < server.arg("plain").length()) {
        server.send(507, "text/plain", "Not enough space to write file");
        return;
      }

      // Attempt to create the file
      File file = LittleFS.open(filename, FILE_WRITE);
      if (!file) {
        Serial.println("Failed to create file for writing");
        server.send(500, "text/plain", "Failed to create file for writing");
        return;
      }

      Serial.println("File created successfully");

      // Write the binary data to the file
      size_t len = server.arg("plain").length();
      file.write((uint8_t*)server.arg("plain").c_str(), len);
      file.close();

      Serial.println("File written successfully");

      // Send a response with the available memory
      String response = "File Uploaded: " + filename + "\nAvailable Memory: " + String(LittleFS.totalBytes() - LittleFS.usedBytes()) + " bytes";
      server.send(200, "text/plain", response);
    } else {
      server.send(400, "text/plain", "Filename missing");
    }
  });

  // Route to handle file deletion
  server.on("/delete", HTTP_DELETE, []() {
    if (server.hasArg("name")) {
      String filename = "/" + server.arg("name");
      if (LittleFS.remove(filename)) {
        String response = "File Deleted\nAvailable Memory: " + String(LittleFS.totalBytes() - LittleFS.usedBytes()) + " bytes";
        Serial.println("File deleted successfully");

        server.send(200, "text/plain", response);
      } else {
        server.send(404, "text/plain", "File Not Found");
      }
    } else {
      server.send(400, "text/plain", "Name parameter missing");
    }
  });

  // Start server
  server.begin();
}

void loop() {
  server.handleClient();  // Handle incoming client requests
}
