function fetchReadings(){
  fetch('/api/readings')
    .then(r => r.json())
    .then(d => {
      document.getElementById('temp').textContent = d.temperature.toFixed(2);
      document.getElementById('rh').textContent = d.relative_humidity.toFixed(2);
    })
    .catch(e => console.error(e));
}

function submitWifi(){
  const ssid = document.getElementById('ssid').value;
  const password = document.getElementById('password').value;
  if (ssid) {
      fetch('/api/connect_wifi', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({ssid: ssid, password: password})
    });
  }
}

function resetWifi(){
    fetch('/api/reset_wifi', {
    method: 'POST'
  });
}

// Returns Promise<boolean> — resolves to true when we got a successful response
function fetchWifiCredentials(){
  return fetch('/api/wifi_credentials')
    .then(r => {
      if (!r.ok) throw new Error('Network response was not ok');
      return r.json();
    })
    .then(d => {
      document.getElementById('ssid').placeholder = d.ssid ? d.ssid : 'SSID';
      document.getElementById('password').placeholder = d.password ? d.password : 'Password';
      if (d.ssid) {
        document.getElementById('resetWifiBtn').disabled = false;
      }
      return true;
    })
    .catch(e => { console.error(e); return false; });
}

function submitServer(){
  const url = document.getElementById('serverUrl').value;
  const port = Number(document.getElementById('serverPort').value);
  const deviceName = document.getElementById('deviceName').value;
  if (url && deviceName) {
      fetch('/api/connect_server', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({url: url, port: port, device_name: deviceName})
    });
  }
}

function resetServer(){
    fetch('/api/reset_server', {
    method: 'POST'
  });
}

// Returns Promise<boolean> — resolves to true when we got a successful response
function fetchServerInfo(){
  return fetch('/api/server_info')
    .then(r => {
      if (!r.ok) throw new Error('Network response was not ok');
      return r.json();
    })
    .then(d => {
      document.getElementById('serverUrl').placeholder = d.url ? d.url : 'URL or IP address';
      document.getElementById('serverPort').placeholder = d.port ? d.port : 'Port';
      document.getElementById('deviceName').placeholder = d.device_name ? d.device_name : 'Device Name';
      if (d.url) {
        document.getElementById('resetServerBtn').disabled = false;
      }
      return true;
    })
    .catch(e => { console.error(e); return false; });
}

// Helper: call async fn repeatedly every intervalMs until it resolves true
function startPolling(fn, intervalMs){
  // initial try
  fn().then(success => {
    if (success) return;
    const id = setInterval(() => {
      fn().then(s => { if (s) clearInterval(id); });
    }, intervalMs);
  });
}

function showTab(tabId, btn){
  document.querySelectorAll('.tabcontent').forEach(c => c.classList.remove('active'));
  document.querySelectorAll('.tablinks').forEach(b => b.classList.remove('active'));
  document.getElementById(tabId).classList.add('active');
  btn.classList.add('active');
}

document.addEventListener('DOMContentLoaded', function(){
  document.getElementById('tabSensorBtn').addEventListener('click', function(){ showTab('sensorTab', this); });
  document.getElementById('tabWifiBtn').addEventListener('click', function(){ showTab('wifiTab', this); });
  document.getElementById('tabServerBtn').addEventListener('click', function(){ showTab('serverTab', this); });

  fetchReadings();
  startPolling(fetchWifiCredentials, 1000);
  startPolling(fetchServerInfo, 1000);
  setInterval(fetchReadings, 5000);
});
