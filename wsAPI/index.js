;(function(){

	function createWebSocket(url, onMessage, onClose){
		var ws = new WebSocket(url);
		if(onMessage) ws.onmessage = onMessage;
		if(onClose) ws.onclose = onClose;
		return ws;
	}

	document.addEventListener('DOMContentLoaded', function(){
		var addrField = document.getElementById('addr');
		var msgField = document.getElementById('message');
		var connectBtn = document.getElementById('connectBtn');
		var sendBtn = document.getElementById('sendBtn');
		var closeBtn = document.getElementById('closeBtn');
		var socket;
		var requests = {};

		function sendMsg(msg){
			if(msg.length){
				console.log('send: ' + msg);
				socket.send(msg);
			}
		};

        //expode sendMsg
        window.sendMsg = sendMsg;


		function sendMsgFromMsgField(){
			sendMsg(msgField.value);
			msgField.value = '';
			console.log(jo2.getCallStack());
		};

		msgField.onkeydown = function(e){
			if(e.keyCode == 13){
				sendMsgFromMsgField();
			}
		};

		connectBtn.onclick = function(){
			socket = createWebSocket(
					addrField.value,
					function(e){
						console.log('[WebSocket] data', e);
						var requestDetails = e.data.split(', ');
						var id = requestDetails[0];
						if(!requests[id]){
							requests[id] = {
								method: requestDetails[1],
								statusCode: requestDetails[2],
								contentType: requestDetails[3],
								startTime: (new Date()).getTime()
							};
						}
						else{
							requests[id].statusCode = requestDetails[2];
							requests[id].endTime = (new Date()).getTime();
						}
						console.log(requests[id], requestDetails);
					},
					function(e){
						console.log('[WebSocket] close', e);
					}
			);
			console.log('connecting');
			msgField.focus();
		};

		sendBtn.onclick = function(){
			sendMsgFromMsgField();
			msgField.focus();
		};

		closeBtn.onclick = function(){
			socket.close();
			console.log('closed');
		};
	});

})();
