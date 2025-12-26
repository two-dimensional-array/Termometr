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

function fetchWifiCredentials(){
  fetch('/api/wifi_credentials')
    .then(r => r.json())
    .then(d => {
      document.getElementById('ssid').placeholder = d.ssid ? d.ssid : 'SSID';
      document.getElementById('password').placeholder = d.password ? d.password : 'Password';
      if (d.ssid) {
        document.getElementById('resetWifiBtn').disabled = false;
      }
    })
    .catch(e => console.error(e));
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
});

fetchReadings();
fetchWifiCredentials();
setInterval(fetchReadings, 5000);
