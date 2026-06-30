#include "webserver.h"

WebServerHandler::WebServerHandler() : server(WEBSERVER_PORT), running(false) {
}

void WebServerHandler::begin(const AttitudeData* attitude, const FlightControl* fc, const FlightMode* mode, SemaphoreHandle_t mutex) {
    attitude_data = attitude;
    flight_control = fc;
    flight_mode = mode;
    data_mutex = mutex;
    setupRoutes();
}

void WebServerHandler::start() {
    if (running) return;
    
    Serial.println("Starting WiFi AP...");
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    
    server.begin();
    running = true;
    Serial.println("Webserver started");
}

void WebServerHandler::stop() {
    if (!running) return;
    
    server.stop();
    WiFi.softAPdisconnect(true);
    running = false;
    Serial.println("Webserver stopped");
}

void WebServerHandler::handleClient() {
    if (running) {
        server.handleClient();
    }
}

void WebServerHandler::setupRoutes() {
    server.on("/", HTTP_GET, std::bind(&WebServerHandler::handleRoot, this));
    server.on("/data", HTTP_GET, std::bind(&WebServerHandler::handleData, this));
    server.onNotFound(std::bind(&WebServerHandler::handleNotFound, this));
}

void WebServerHandler::handleRoot() {
    server.send(200, "text/html", getHTMLPage());
}

void WebServerHandler::handleData() {
    String json = "{";
    
    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        json += "\"roll\": " + String(attitude_data->roll * 180.0f / M_PI, 2) + ",";
        json += "\"pitch\": " + String(attitude_data->pitch * 180.0f / M_PI, 2) + ",";
        json += "\"yaw\": " + String(attitude_data->yaw * 180.0f / M_PI, 2) + ",";
        
        const PID& roll_rate_pid = flight_control->getRollRatePID();
        const PID& pitch_rate_pid = flight_control->getPitchRatePID();
        const PID& roll_angle_pid = flight_control->getRollAnglePID();
        const PID& pitch_angle_pid = flight_control->getPitchAnglePID();
        
        json += "\"roll_rate_pid\": {";
        json += "\"kp\": " + String(roll_rate_pid.getKp(), 4) + ",";
        json += "\"ki\": " + String(roll_rate_pid.getKi(), 4) + ",";
        json += "\"kd\": " + String(roll_rate_pid.getKd(), 4) + ",";
        json += "\"integral\": " + String(roll_rate_pid.getIntegral(), 4) + ",";
        json += "\"error\": " + String(roll_rate_pid.getPrevError(), 4);
        json += "},";
        
        json += "\"pitch_rate_pid\": {";
        json += "\"kp\": " + String(pitch_rate_pid.getKp(), 4) + ",";
        json += "\"ki\": " + String(pitch_rate_pid.getKi(), 4) + ",";
        json += "\"kd\": " + String(pitch_rate_pid.getKd(), 4) + ",";
        json += "\"integral\": " + String(pitch_rate_pid.getIntegral(), 4) + ",";
        json += "\"error\": " + String(pitch_rate_pid.getPrevError(), 4);
        json += "},";
        
        json += "\"roll_angle_pid\": {";
        json += "\"kp\": " + String(roll_angle_pid.getKp(), 4) + ",";
        json += "\"ki\": " + String(roll_angle_pid.getKi(), 4) + ",";
        json += "\"kd\": " + String(roll_angle_pid.getKd(), 4) + ",";
        json += "\"integral\": " + String(roll_angle_pid.getIntegral(), 4) + ",";
        json += "\"error\": " + String(roll_angle_pid.getPrevError(), 4);
        json += "},";
        
        json += "\"pitch_angle_pid\": {";
        json += "\"kp\": " + String(pitch_angle_pid.getKp(), 4) + ",";
        json += "\"ki\": " + String(pitch_angle_pid.getKi(), 4) + ",";
        json += "\"kd\": " + String(pitch_angle_pid.getKd(), 4) + ",";
        json += "\"integral\": " + String(pitch_angle_pid.getIntegral(), 4) + ",";
        json += "\"error\": " + String(pitch_angle_pid.getPrevError(), 4);
        json += "},";
        
        json += "\"flight_mode\": " + String(*flight_mode);
        
        xSemaphoreGive(data_mutex);
    }
    
    json += "}";
    server.send(200, "application/json", json);
}

void WebServerHandler::handleNotFound() {
    server.send(404, "text/plain", "Not found");
}

String WebServerHandler::getHTMLPage() {
    return R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>OpenWing</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;background:#0d1117;color:#c9d1d9;padding:16px;min-height:100vh;-webkit-font-smoothing:antialiased}
.container{max-width:480px;margin:0 auto}
h1{font-size:16px;font-weight:600;color:#58a6ff;margin-bottom:16px;letter-spacing:2px;text-transform:uppercase}
.card{background:#161b22;border:1px solid #30363d;border-radius:12px;padding:16px;margin-bottom:12px}
.grid{display:grid;gap:12px;margin-bottom:12px}
.attitude{grid-template-columns:repeat(3,1fr)}
.attitude .card{padding:20px 12px;margin:0}
.card .lbl{font-size:10px;text-transform:uppercase;color:#8b949e;letter-spacing:1.5px;margin-bottom:6px;font-weight:500}
.card .val{font-size:26px;font-weight:700;font-family:"SF Mono","Fira Code","Cascadia Code",monospace}
.val.r{color:#f0883e}.val.p{color:#58a6ff}.val.y{color:#3fb950}
.mode{text-align:center;padding:16px}
.mode .lbl{font-size:10px;text-transform:uppercase;color:#8b949e;letter-spacing:1.5px;margin-bottom:4px;font-weight:500}
.mode .val{font-size:22px;font-weight:700;color:#f0883e;letter-spacing:1px}
.pid-title{font-size:12px;font-weight:600;color:#8b949e;margin-bottom:12px;letter-spacing:0.5px}
.pid-grid{display:grid;grid-template-columns:1fr 1fr;gap:8px}
.pid-item{background:#0d1117;border:1px solid #21262d;border-radius:8px;padding:8px 10px}
.pid-item .nm{font-size:10px;color:#8b949e;margin-bottom:4px;font-weight:500}
.pid-item .vs{font-family:"SF Mono","Fira Code","Cascadia Code",monospace;font-size:9px;color:#c9d1d9;line-height:1.7}
.st{text-align:center;font-size:10px;color:#484f58;margin-top:16px;letter-spacing:0.5px;transition:color .3s}
.st.ok{color:#3fb950}.st.er{color:#f85149}
</style>
</head>
<body>
<div class="container">
<h1>OpenWing</h1>
<div class="grid attitude">
<div class="card"><div class="lbl">Roll</div><div class="val r" id="roll">—</div></div>
<div class="card"><div class="lbl">Pitch</div><div class="val p" id="pitch">—</div></div>
<div class="card"><div class="lbl">Yaw</div><div class="val y" id="yaw">—</div></div>
</div>
<div class="card mode"><div class="lbl">Flight Mode</div><div class="val" id="mode">—</div></div>
<div class="card"><div class="pid-title">PID</div><div class="pid-grid" id="pid"></div></div>
<div class="st" id="st">disconnected</div>
</div>
<script>
var M={0:"MANUAL",1:"FBWA",2:"STABILIZE",3:"RTL",4:"AUTO",5:"LOITER"};
var A=["roll_rate","pitch_rate","roll_angle","pitch_angle"];
var L={roll_rate:"Roll Rate",pitch_rate:"Pitch Rate",roll_angle:"Roll Angle",pitch_angle:"Pitch Angle"};
var G=document.getElementById("pid");
A.forEach(function(a){var d=document.createElement("div");d.className="pid-item";d.id="p-"+a;d.innerHTML='<div class="nm">'+L[a]+'</div><div class="vs" id="v-'+a+'">—</div>';G.appendChild(d)});
function f(v){return (v||0).toFixed(4)}
function u(){fetch("/data").then(function(r){return r.json()}).then(function(d){document.getElementById("roll").textContent=(d.roll||0).toFixed(1)+"\u00b0";document.getElementById("pitch").textContent=(d.pitch||0).toFixed(1)+"\u00b0";document.getElementById("yaw").textContent=(d.yaw||0).toFixed(1)+"\u00b0";document.getElementById("mode").textContent=M[d.flight_mode]||"UNKNOWN";A.forEach(function(a){var p=d[a+"_pid"];if(p)document.getElementById("v-"+a).textContent="Kp "+f(p.kp)+"\u00a0Ki "+f(p.ki)+"\u00a0Kd "+f(p.kd)});var s=document.getElementById("st");s.textContent="connected";s.className="st ok"}).catch(function(){var s=document.getElementById("st");s.textContent="disconnected";s.className="st er"})}
setInterval(u,300);u();
</script>
</body>
</html>
    )";
}
