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
  fetch('/configure', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ssid: ssid, password: password})
  });
}

fetchReadings();
setInterval(fetchReadings, 5000);
