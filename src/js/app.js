// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Initialize Clay
var clay = new Clay(clayConfig);

var myAPIKey = '';

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL
//   var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
//       pos.coords.latitude + "&lon=" + pos.coords.longitude + '&appid=' + myAPIKey;

    var url = "https://murmuring-tor-41917.herokuapp.com/weather"
    + "?lat=" +      pos.coords.latitude + "&lon=" + pos.coords.longitude;
//&forecast_key=
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
            console.log(responseText);

      //encode daily hi/low pairs into a string
      //max, min, high1, low1, ... , high7, low7
      var string = "";
      for (var i=0; i<json.length; i++){
        string = string + String.fromCharCode(Math.round(json[i]));
      }
      console.log(string);

      // Temperature in Kelvin requires adjustment
//       var temperature = Math.round(json.main.temp - 273.15);
//       var temperatureChar = String.fromCharCode(temperature);

//       console.log("Temperature is " + temperatureChar);

      // Conditions
//       var conditions = json.weather[0].main;
//       console.log("Conditions are " + conditions);


      // Assemble dictionary using our keys
      var dictionary = {
        "Temps": string
//         "KEY_CONDITIONS": string
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }
  );
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getWeather();
  }
);
