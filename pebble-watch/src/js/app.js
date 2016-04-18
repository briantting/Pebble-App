var message = "Sent to computer";
var count = 0;

Pebble.addEventListener("appmessage", 
	function(e) {
		if(e.payload) {
			if(e.payload.hello_msg) {
				Pebble.sendAppMessage({ "1": "Recvd: " + e.payload.hello_msg });
			}
			else Pebble.sendAppMessage({ "1": "nokey" });
		}
		else {
			Pebble.sendAppMessage({ "1" : "nopayload"});
		}
		var request = e.payload.hello_msg;
		sendToServer(request); 
	}
);

function sendToServer(request) {
  var req = new XMLHttpRequest();
  var ipAddress = "158.130.104.125"; // Hard coded IP address "192.168.1.151"
  var port = "3001"; // Same port specified as argument to server
  var url = "http://" + ipAddress + ":" + port + "/" + request;
  var method = "GET";
  var async = true;

  if(request.localeCompare("change") == 0 || request.localeCompare("read_temp") == 0) {
  	method = "POST";
  }

  req.onload = function(e) {
    // see what came back

    var msg = "no response";
    var response = JSON.parse(req.responseText);
    if (response) {
      if (response.name) {
        msg = response.name;
      }
      else msg = "noname";
    }
    // sends message back to pebble
    Pebble.sendAppMessage({ "2": msg });

  };

  req.open(method, url, async);
  req.send(null);
}