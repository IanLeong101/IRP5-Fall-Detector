
  // Import the functions you need from the SDKs you need
  import { initializeApp } from "https://www.gstatic.com/firebasejs/10.7.1/firebase-app.js";
  import { getAnalytics } from "https://www.gstatic.com/firebasejs/10.7.1/firebase-analytics.js";
  import { getDatabase, ref, onValue, set } from "https://www.gstatic.com/firebasejs/10.7.1/firebase-database.js";
  // TODO: Add SDKs for Firebase products that you want to use
  // https://firebase.google.com/docs/web/setup#available-libraries

  // Your web app's Firebase configuration
  // For Firebase JS SDK v7.20.0 and later, measurementId is optional
  const firebaseConfig = {
    apiKey: "AIzaSyBlUYAkNN1a2hBtP2jkFCdOFxD6ePvSHS0",
    authDomain: "irprxandian.firebaseapp.com",
    databaseURL: "https://irprxandian-default-rtdb.asia-southeast1.firebasedatabase.app",
    projectId: "irprxandian",
    storageBucket: "irprxandian.appspot.com",
    messagingSenderId: "971031263418",
    appId: "1:971031263418:web:73ab8c5014204ef045aa83",
    measurementId: "G-6QPFD60XN9"
  };

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const database = getDatabase();

const TempRef = ref(database, 'DHT11/Temp');
const HumiRef = ref(database, 'DHT11/Humi');

const BPMRef = ref(database, 'HeartRate/BPM');

/*const AXRef = ref(database, 'Accelerometer/AX');
const AYRef = ref(database, 'Accelerometer/AY');
const AZRef = ref(database, 'Accelerometer/AZ');

const GXRef = ref(database, 'Accelerometer/GX');
const GYRef = ref(database, 'Accelerometer/GY');
const GZRef = ref(database, 'Accelerometer/GZ');*/

const FallRef = ref(database, 'Accelerometer/Fall');

let chartData = [];

let myChart;

function getData() {
   //Get temperature value
    onValue(TempRef, (Snapshot) => {
      var Data = Snapshot.val();
      console.log('Temperature Snapshot:', Snapshot);
      console.log('Temperature Reading:', Data);
  
      // Update the humidity label
      updateTempLabel(Data);
    });

    //Get Humidity value
    onValue(HumiRef, (Snapshot) => {
      var Data = Snapshot.val();
      console.log('Humidity Snapshot:', Snapshot);
      console.log('Humidity Reading:', Data);
  
      // Update the humidity label
      updateHumiLabel(Data);
    });

    //Get BPM value
    onValue(BPMRef, (Snapshot) => {
      var Data = Snapshot.val();
      console.log('BPM Snapshot:', Snapshot);
      console.log('BPM Reading:', Data);
  
      // Update the BPM label
      updateBPMLabel(Data);
    }); 

    // Listener for Rotary data
    onValue(BPMRef, (Snapshot) => {
    var BPMData = Snapshot.val();
    console.log('BPM Reading:', BPMData);

    // Update the chart data array
    chartData.push(BPMData);

    // Destroy the existing chart if it exists
    if (myChart) {
      myChart.destroy();
    }

    // Call the function to create/update the line chart with the latest data
    createLineChart();
    });

    /*//Get AX value
    onValue(AXRef, (Snapshot) => {
      var Data = Snapshot.val();
      console.log('AX Snapshot:', Snapshot);
      console.log('AX Reading:', Data);
  
      // Update the AX label
      updateAXLabel(Data);
    });

    //Get AY value
    onValue(AYRef, (Snapshot) => {
      var Data = Snapshot.val();
      console.log('AY Snapshot:', Snapshot);
      console.log('AY Reading:', Data);
  
      // Update the AY label
      updateAYLabel(Data);
    });

    //Get AZ value
    onValue(AZRef, (Snapshot) => {
      var Data = Snapshot.val();
      console.log('AZ Snapshot:', Snapshot);
      console.log('AZ Reading:', Data);
  
      // Update the AZ label
      updateAZLabel(Data);
    });

    //Get GX value
    onValue(GXRef, (Snapshot) => {
      var Data = Snapshot.val();
      console.log('GX Snapshot:', Snapshot);
      console.log('GX Reading:', Data);
  
      // Update the GX label
      updateGXLabel(Data);
    });

    //Get GY value
    onValue(GYRef, (Snapshot) => {
      var Data = Snapshot.val();
      console.log('GY Snapshot:', Snapshot);
      console.log('GY Reading:', Data);
  
      // Update the GY label
      updateGYLabel(Data);
    });

    //Get GZ value
    onValue(GZRef, (Snapshot) => {
      var Data = Snapshot.val();
      console.log('GZ Snapshot:', Snapshot);
      console.log('GZ Reading:', Data);
  
      // Update the GZ label
      updateGZLabel(Data);
    });*/
    //Get Fall value
    onValue(FallRef, (Snapshot) => {
      var Data = Snapshot.val();
      console.log('Fall Snapshot:', Snapshot);
      console.log('Fall Reading:', Data);

      // Update the GZ label
      updateFallLabel(Data);
    });

  }
  
  // Function to update the temperature label
  function updateTempLabel(Data) {
    const tempLabel = document.getElementById('TempLb');
    if (tempLabel) {
      tempLabel.innerHTML = `<h3>${Data !== null ? Data + '°C' : 'N/A'}</h3>`;
    }
  }

  // Function to update the humidity label
  function updateHumiLabel(Data) {
    const humiLabel = document.getElementById('HumiLb');
    if (humiLabel) {
      humiLabel.innerHTML = `<h3>${Data !== null ? Data + '%' : 'N/A'}</h3>`;
    }
  }

  // Function to update the BPM label
  function updateBPMLabel(Data) {
    const BPMLabel = document.getElementById('BPMLb');
    if (BPMLabel) {
      BPMLabel.innerHTML = `<h3>${Data !== null ? Data + 'BPM' : 'N/A'}</h3>`;
    }
  }

  // Function to update the Fall label
  function updateFallLabel(Data) {
    const FallLabel = document.getElementById('FallLb');
    if (FallLabel) {
      FallLabel.innerHTML = `<h3>${Data !== null ? 'Fall: ' + Data : 'N/A'}</h3>`;
    }
  }

  /*// Function to update the AX label
  function updateAXLabel(Data) {
    const AXLabel = document.getElementById('AXLb');
    if (AXLabel) {
      AXLabel.innerHTML = `<h3>${Data !== null ? 'X = ' + Data + 'm/s²' : 'N/A'}</h3>`;
    }
  }

  // Function to update the AY label
  function updateAYLabel(Data) {
    const AYLabel = document.getElementById('AYLb');
    if (AYLabel) {
      AYLabel.innerHTML = `<h3>${Data !== null ? 'Y = ' + Data + 'm/s²' : 'N/A'}</h3>`;
    }
  }

  // Function to update the AZ label
  function updateAZLabel(Data) {
    const AZLabel = document.getElementById('AZLb');
    if (AZLabel) {
      AZLabel.innerHTML = `<h3>${Data !== null ? 'Z = ' + Data + 'm/s²': 'N/A'}</h3>`;
    }
  }

  // Function to update the GX label
  function updateGXLabel(Data) {
    const GXLabel = document.getElementById('GXLb');
    if (GXLabel) {
      GXLabel.innerHTML = `<h3>${Data !== null ? 'X = ' + Data   : 'N/A'}</h3>`;
    }
  }

  // Function to update the GY label
  function updateGYLabel(Data) {
    const GYLabel = document.getElementById('GYLb');
    if (GYLabel) {
      GYLabel.innerHTML = `<h3>${Data !== null ? 'Y = ' + Data : 'N/A'}</h3>`;
    }
  }

  // Function to update the GZ label
  function updateGZLabel(Data) {
    const GZLabel = document.getElementById('GZLb');
    if (GZLabel) {
      GZLabel.innerHTML = `<h3>${Data !== null ? 'Z = ' + Data : 'N/A'}</h3>`;
    }
  }*/

  // Function to create/update the line chart
function createLineChart() {
   console.log('Creating chart with data:', chartData);

    const ctx = document.getElementById('healthChart').getContext('2d');
    myChart = new Chart(ctx, {
     type: 'line',
      data: {
        labels: chartData.map((_, index) => index + 1), // Assuming array index as labels
        datasets: [{
         label: 'Readings',
        data: chartData,
         borderColor: 'blue',
          borderWidth: 1,
         fill: false,
        }],
     },
      options: {
       scales: {
          x: {
            type: 'category',
            labels: chartData.map((_, index) => index + 1), // Assuming array index as labels
            scaleLabel: {
             display: true,
              labelString: 'Time',
           },
          },
          y: {
            beginAtZero: true,
            scaleLabel: {
             display: true,
             labelString: 'Reading (bpm)',
           },
          },
       },
        maintainAspectRatio: false,
        responsive: true,
       plugins: {
          title: {
           display: true,
         },
        },
      },
    });
  }
  
  document.addEventListener('DOMContentLoaded', getData);
  document.addEventListener('DOMContentLoaded', createLineChart);

