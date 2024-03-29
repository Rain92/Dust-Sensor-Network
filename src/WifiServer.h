#pragma once

#include <ESPmDNS.h>
#include <SdCard.h>
#include <WebServer.h>
#include <vector>
#include "datastore.h"

WebServer server(80);

File fsUploadFile;
const char *host = "dust-sensor-network";

void (*toggleCallback)(bool) = nullptr;
bool (*statusCallback)() = nullptr;
void (*tagCallback)() = nullptr;

String formatBytes(size_t bytes)
{
    if (bytes < 1024)
    {
        return String(bytes) + "B";
    }
    else if (bytes < (1024 * 1024))
    {
        return String(bytes / 1024.0) + "KB";
    }
    else if (bytes < (1024 * 1024 * 1024))
    {
        return String(bytes / 1024.0 / 1024.0) + "MB";
    }
    else
    {
        return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
    }
}

String getContentType(const String &filename)
{
    if (server.hasArg("download"))
    {
        return "application/octet-stream";
    }
    else if (filename.endsWith(".htm"))
    {
        return "text/html";
    }
    else if (filename.endsWith(".html"))
    {
        return "text/html";
    }
    else if (filename.endsWith(".css"))
    {
        return "text/css";
    }
    else if (filename.endsWith(".js"))
    {
        return "application/javascript";
    }
    else if (filename.endsWith(".png"))
    {
        return "image/png";
    }
    else if (filename.endsWith(".gif"))
    {
        return "image/gif";
    }
    else if (filename.endsWith(".jpg"))
    {
        return "image/jpeg";
    }
    else if (filename.endsWith(".ico"))
    {
        return "image/x-icon";
    }
    else if (filename.endsWith(".xml"))
    {
        return "text/xml";
    }
    else if (filename.endsWith(".pdf"))
    {
        return "application/x-pdf";
    }
    else if (filename.endsWith(".zip"))
    {
        return "application/x-zip";
    }
    else if (filename.endsWith(".gz"))
    {
        return "application/x-gzip";
    }
    return "text/plain";
}

bool handleFileRead(const String &path)
{
    Serial.println("handleFileRead: " + path);
    String contentType = getContentType(path);

    auto file = SD.open(path.c_str(), FILE_READ);
    if (file)
    {
        server.streamFile(file, contentType);
        file.close();
    }

    return file;
}

void handleToggle(bool start)
{
    if (toggleCallback)
    {
        toggleCallback(start);
        if (!statusCallback)
            server.send(200, "text/plain", "Success");
        else
            server.send(200, "text/plain", statusCallback() ? "Started Measuring" : "Stopped measuring");
    }
    else
    {
        server.send(200, "text/plain", "Failure");
    }
}

void handleFileUpload()
{
    if (server.uri() != "/edit")
    {
        return;
    }
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        String filename = upload.filename;
        if (!filename.startsWith("/"))
        {
            filename = "/" + filename;
        }
        Serial.print("handleFileUpload Name: ");
        Serial.println(filename);
        fsUploadFile = SD.open(filename.c_str(), FILE_WRITE);
        filename = String();
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (fsUploadFile)
        {
            fsUploadFile.write(upload.buf, upload.currentSize);
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (fsUploadFile)
        {
            fsUploadFile.close();
        }
        Serial.print("handleFileUpload Size: ");
        Serial.println(upload.totalSize);
    }
}

void handleFileDelete()
{
    if (server.args() == 0)
    {
        return server.send(500, "text/plain", "BAD ARGS");
    }
    String path = server.arg(0);
    Serial.println("handleFileDelete: " + path);
    if (path == "/")
    {
        return server.send(500, "text/plain", "BAD PATH");
    }
    if (checkPath(path) != IsFile)
    {
        return server.send(404, "text/plain", "FileNotFound");
    }
    SD.remove((char *)path.c_str());
    server.send(200, "text/plain", "");
    path = String();
}

void handleFileCreate()
{
    if (server.args() == 0)
    {
        return server.send(500, "text/plain", "BAD ARGS");
    }
    String path = server.arg(0);
    Serial.println("handleFileCreate: " + path);
    if (path == "/")
    {
        return server.send(500, "text/plain", "BAD PATH");
    }
    if (checkPath(path) == IsFile)
    {
        return server.send(500, "text/plain", "FILE EXISTS");
    }
    auto file = SD.open((char *)path.c_str(), FILE_WRITE);
    if (file)
    {
        file.close();
    }
    else
    {
        return server.send(500, "text/plain", "CREATE FAILED");
    }
    server.send(200, "text/plain", "");
    path = String();
}

void handleFileGet()
{
    if (!server.hasArg("file"))
    {
        server.send(500, "text/plain", "BAD ARGS");
        return;
    }

    String path = server.arg("file");
    Serial.println("handleFileGet: " + path);

    if (!handleFileRead(path))
    {
        server.send(404, "text/plain", "FileNotFound");
    }
}

String getParentDir(const String &path)
{
    int i = path.lastIndexOf("/");
    return path.substring(0, i + 1);
}

void deletePath(const String &path)
{
    auto ftype = checkPath(path);
    if (ftype == IsFile)
    {
        SD.remove((char *)path.c_str());
    }

    else if (ftype == IsDirectory)
    {
        auto root = SD.open(path.c_str());
        root.rewindDirectory();
        auto file = root.openNextFile();
        while (file)
        {
            if (file.isDirectory())
                deletePath(path + "/" + file.name());
            else
                SD.remove((char *)(path + "/" + file.name()).c_str());

            file = root.openNextFile();
        }

        SD.rmdir((char *)path.c_str());
    }
}

void handleFileList(String path)
{
    auto start = millis();

    Serial.println("handleFileList: " + path);

    auto root = SD.open(path.c_str());
    if (!root.isDirectory())
    {
        root.close();
        root = SD.open("/");
    }

    if (path.endsWith("/"))
        path = path.substring(0, path.length() - 1);

    bool isRoot = path.length() <= 1;

    String output = F(
        R"(<html><head><link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css">)"
        R"(<meta name="viewport" content="width=device-width, initial-scale=1"></head><body class="container"><br /><br />)");

    if (!statusCallback || !statusCallback())
    {
        output +=
            F("<a href='?action=start'>"
              "Start Measuring</a><br /><br />");
    }
    else
    {
        output +=
            F("<a href='?action=stop'>"
              "Stop Measuring</a>&emsp;&emsp;&emsp;"
              "<a href='?action=tag'>Set tag</a><br /><br />");
    }

    if (root.isDirectory())
    {
        auto tuple = listFiles(root);

        auto parentDir = getParentDir(path);
        for (int round = 0; round < 2; round++)
        {
            auto *entrylist = round == 0 ? &std::get<0>(tuple) : &std::get<1>(tuple);
            bool isdir = round == 0 ? true : false;

            if (round == 0 && !isRoot)
                output += "<a href='" + parentDir + "'>../</a><br /><br />";

            for (auto &filename : *entrylist)
            {
                String fpath = path + "/" + filename;

                output += "<a href='" + fpath + "'>" + filename;
                if (isdir)
                    output += "/";

                output +=
                    "</a>\n"
                    "&emsp;<a href='?delete=" +
                    fpath +
                    "'><i class='glyphicon glyphicon-remove'></i></a>"
                    "<br /><br />";
            }
        }
    }
    output += "</body></html>";

    root.close();

    Serial.printf("handleFileList took: %lums\n", millis() - start);

    server.send(200, "text/html", output);
}

bool handleGeneralRequest(const String &path)
{
    bool redirect = false;
    String deletefilepath = server.arg("delete");
    if (deletefilepath.length() > 1)
    {
        deletePath(deletefilepath);
        redirect = true;
    }

    String actionarg = server.arg("action");
    if (actionarg.length() > 1)
    {
        if (actionarg == "start" && toggleCallback)
            toggleCallback(true);
        else if (actionarg == "stop" && toggleCallback)
            toggleCallback(false);
        else if (actionarg == "tag" && tagCallback)
            tagCallback();
        redirect = true;
    }

    if (redirect)
    {
        server.sendHeader("Location", path);
        server.send(303);
        return true;
    }
    else
    {
        auto res = checkPath(path);
        if (res == NotFound)
            return false;
        else if (res == IsFile)
            return handleFileRead(path);
        else if (res == IsDirectory)
            handleFileList(path);
        return true;
    }
}

void handleFileListArg()
{
    String path = server.arg("dir");
    handleFileList(path);
}

void handleSendData()
{
    int sensor = server.arg("sensor").toInt();
    float pm25 = server.arg("pm25").toFloat();
    float pm10 = server.arg("pm10").toFloat();

    if (sensor <= 0 || pm25 <= 0 || pm10 <= 0)
        return server.send(500, "text/plain", "BAD ARGS");

    registerDustSensorData(sensor, DustSensorData{pm25, pm10});

    server.send(200, "text/plain", "Success");
}

void initWebServer()
{
    MDNS.begin(host);
    Serial.print("Open http://");
    Serial.print(host);
    Serial.println(".local");

    // SERVER INIT
    // list directory
    server.on("/list", HTTP_GET, handleFileListArg);

    // create file
    server.on("/edit", HTTP_PUT, handleFileCreate);

    // delete file
    server.on("/edit", HTTP_DELETE, handleFileDelete);

    // unused
    server.on(
        "/edit", HTTP_POST, []() { server.send(200, "text/plain", ""); }, handleFileUpload);

    server.on("/get", HTTP_GET, handleFileGet);


    // send sensordata via http, unused
    server.on("/senddata", HTTP_GET, handleSendData);

    // start measuring and logging
    server.on("/start", HTTP_GET, []() { handleToggle(true); });
    
    // stop measuring and logging
    server.on("/stop", HTTP_GET, []() { handleToggle(false); });

    // called when the url is not defined here
    // use it to load content from FILESYSTEM
    server.onNotFound([]() {
        if (!handleGeneralRequest(server.uri()))
        {
            server.send(404, "text/plain", "FileNotFound");
        }
    });

    // get heap status
    server.on("/mem", HTTP_GET, []() {
        String json = "{";
        json += "\"heap\":" + String(ESP.getFreeHeap());
        json += "}";
        server.send(200, "text/json", json);
        json = String();
    });
    server.begin();
    Serial.println("HTTP server started");
}

void handleRequests()
{
    server.handleClient();
}