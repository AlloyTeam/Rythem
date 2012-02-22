(function(){

	/**
	 * Connection
	 * @param {uint} id
	 * @param {String} socketID
	 */
	var Connection = function(id, socketID){
		this.id = id;
		this.socketID = socketID;
		this.requestHeader = {};
		this.responseHeader = {};
	};
	Connection.prototype = {
		/**
		 * parse http request/response header
		 * @param {String} header
		 * @return {Object}
		 */
		parseHttpHeader: function(header){
			//split the header
			header = header.split('\r\n\r\n')[0];
			var lines = header.split('\r\n');

			//parse the header fields
			var i, len = lines.length, fields = {};
			for(i=1; i<len; i++){
				var kv = lines[i].split(': ');
				fields[kv[0]] = kv[1];
			}

			//parse the first line("GET /xxx HTTP/1.1" for request or "HTTP/1.1 200 OK" for response)
			var firstLine = lines[0].split(' ');
			var method, url, host, path, file, httpVersion, status, description;
			if(parseInt(firstLine[1])){
				//this is a response
				httpVersion = firstLine[0];
				status = parseInt(firstLine[1]);
				description = firstLine[2];
			}
			else{
				//this is a request
				method = firstLine[0];
				url = firstLine[1];
				httpVersion = firstLine[2];

				var uri = parseUri(url);
				host = uri.host || fields['Host'];
				path = uri.path;
				file = uri.file;
			}
			return {
				method: method,
				url: url,
				host: host,
				path: path,
				file: file,
				httpVersion: httpVersion,
				status: status,
				desc: description,
				fields: fields
			};
		},
		/**
		 * set request header
		 * @param {String} header
		 */
		setRequestHeader: function(header){
			this.requestHeader = this.parseHttpHeader(header);
			return this;
		},
		/**
		 * set response header
		 * @param {String} header
		 */
		setResponseHeader: function(header){
			this.responseHeader = this.parseHttpHeader(header);
			return this;
		},
		setHeaders: function(host, url, method, status, reqContentLength, respContentLength){
			var parsedUrl = parseUri(url);
			this.requestHeader = {
				method: method,
				url: url,
				host: host,
				path: parsedUrl.path,
				file: parsedUrl.file,
				contentLength: reqContentLength
			};
			this.responseHeader = {
				status: status,
				contentLength: respContentLength
			};
		},
		/**
		 * set request start time
		 * @param {int} time
		 */
		setStartTime: function(time){
			this.startTime = time;
			if(time > this.responseStartTime){
				this.setResponseStartTime(time);
				this.setResponseFinishTime(time);
			}
			return this;
		},
		/**
		 * set response start time(when you receive the first byte of the response)
		 * @param {int} time
		 */
		setResponseStartTime: function(time){
			//init startTime if it's not yet set
			if(!this.startTime){
				this.setStartTime(time);
			}
			//responseStartTime must not smaller than startTime,
			//and must not bigger than responseEndTime
			if(time < this.startTime){
				time = this.startTime;
			}
			if(time > this.responseFinishTime){
				this.setResponseFinishTime(time);
			}
			this.responseStartTime = time;
			return this;
		},
		/**
		 * set response finish time(when you received the complete response)
		 * @param {int} time
		 */
		setResponseFinishTime: function(time){
			//init starTime and responseStartTime if they are not yet set
			if(!this.responseStartTime){
				this.setResponseStartTime(time);
			}
			//responseEndTime must not smaller than responseStartTime
			if(this.responseStartTime > time) time = this.responseStartTime;
			this.responseFinishTime = time;
			return this;
		},
		getFullUrl: function(){
			return this.requestHeader.url || '?';
		},
		getRequestName: function(){
			return this.requestHeader.file || '/';
		},
		getRequestHost: function(){
			return this.requestHeader.host || '?';
		},
		getRequestMethod: function(){
			return this.requestHeader.method || '?';
		},
		getResponseStatus: function(){
			return this.responseHeader.status || 0;
		},
		getResponseContentLength: function(){
			return this.responseHeader.contentLength || 0;
		},
		/**
		 * 請求開始的時間
		 */
		getStartTime: function(){
			return this.startTime
		},
		/**
		 * 開始接收到response的時間
		 */
		getResponseStartTime: function(){
			return this.responseStartTime
		},
		/**
		 * response接收完畢的時間
		 */
		getResponseFinishTime: function(){
			return this.responseFinishTime
		},
		/**
		 * 請求開始到開始接收到response之間的等待時間
		 */
		getWaitTime: function(){
			return this.responseStartTime - this.startTime;
		},
		/**
		 * 開始接收到response到接收完畢之間的時間
		 */
		getResponseTime: function(){
			return this.responseFinishTime - this.responseStartTime;
		},
		/**
		 * 從請求開始到response接收完畢的整段時間
		 */
		getSessionTime: function(){
			return this.responseFinishTime - this.startTime;
		},
		/**
		 * 與同一個socket上的前一個請求之間所間隔的時間，即該socket發送本請求前的空閒時間
		 */
		getIdelTime: function(){
			var socket = Connection.sockets[this.socketID];
			var index = socket.indexOf(this);
			if(index > 0){
				return this.startTime - socket[index-1].getResponseFinishTime();
			}
			else{
				return Connection.beginningTime ? this.startTime - Connection.beginningTime : 0;
			}
		},
		toString: function(){
			return JSON.stringify({
				id: 				this.id,
				socketID: 			this.socketID,
				startTime: 			this.getStartTime(),
				responseStartTime: 	this.getResponseStartTime(),
				responseFinishTime: this.getResponseFinishTime(),
				waitTime: 			this.getWaitTime(),
				responseTime: 		this.getResponseTime(),
				sessionTime: 		this.getSessionTime()
			});
		}
	};
	Connection.sockets = {};
	Connection.conns = {};
	Connection.beginningTime = 0;
	/**
	 * create a new connection instance, or return an existing one
	 * @param {uint} id
	 * @param {String} [socketID=undefined]
	 */
	Connection.get = function(id, socketID){
		var conn;
		if(Connection.conns[id]){
			conn = Connection.conns[id];
		}
		else{
			conn = new Connection(id, socketID);
			if(!Connection.sockets[socketID]){
				Connection.sockets[socketID] = [];
			}
			Connection.sockets[socketID].push(conn);
			Connection.conns[id] = conn;
		}
		return conn;
	};
	Connection.setRequestHeader = function(id, header){
		return Connection.get(id).setRequestHeader(header);
	};
	Connection.setResponseHeader = function(id, header){
		return Connection.get(id).setResponseHeader(header);
	};
	Connection.setStartTime = function(id, time){
		return Connection.get(id).setStartTime(time);
	};
	Connection.setResponseStartTime = function(id, time){
		return Connection.get(id).setResponseStartTime(time);
	};
	Connection.setResponseFinishTime = function(id, time){
		return Connection.get(id).setResponseFinishTime(time);
	};


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	var socketGroupsEl = document.getElementById('socketGroups');

	/**
	 * get the socket group element by socket id
	 * @param socketID
	 */
	function getSocketUI(socketID){
		var el = document.getElementById('socket-' + socketID);
		if(el){
			return el;
		}
		else{
			var group = document.createElement('article');
			group.id = 'socket-' + socketID;
			group.className = 'socket hgroup';
			group.setData('id', socketID);
			group.innerHTML =
				'<header>\
					<div class="title">' + socketID + '</div>\
				</header>\
				<div class="conns hgroup"></div>';
			socketGroupsEl.appendChild(group);
			return group;
		}
	}

	/**
	 * update corresponding http request element
	 * @param {Connection} conn
	 */
	function updateConnUI(conn){
		var el = document.getElementById('conn-' + conn.id);
		if(!el){
			var item = document.createElement('div');
			item.id = 'conn-' + conn.id;
			item.className = 'conn min';
			item.innerHTML = '\
				<div class="info">\
					<div class="name"></div>\
					<div class="host"></div>\
					<div class="detail"></div>\
					<div class="timing"></div>\
				</div>\
				<div class="time">\
					<div class="wait"></div>\
				</div>';
			var socket = getSocketUI(conn.socketID);
			socket.querySelector('.conns').appendChild(item);
			el = item;
		}

		//get time value and time bar length (in px)
		var waitTime = conn.getWaitTime();
		var responseTime = conn.getResponseTime();
		var waitTimeLen = Math.round(waitTime/10) || 1;
		var totalTimeLen = (Math.round(responseTime/10) || 1) + waitTimeLen;

		//get time gap between this conn and the previous one in the same socket
		el.marginLeft = Math.round(conn.getIdelTime() / 10);

		//time text is use as tooltip of the time bar
		var timeText = waitTime + '+' + responseTime + '=' + (waitTime + responseTime) + 'ms';

		//detail text (request method, response status and response content length)
		var detailText = [
			conn.getRequestMethod(),
			conn.getResponseStatus(),
			conn.getResponseContentLength()/1000 + 'KB'
		].join(' ');

		el.querySelector('.name').textContent = conn.getRequestName();
		el.querySelector('.host').textContent = conn.getRequestHost();
		el.querySelector('.detail').textContent = detailText;
		el.querySelector('.timing').textContent = timeText;
		el.querySelector('.info').title = conn.getFullUrl();
		var timebar = el.querySelector('.time');
		var waitbar = el.querySelector('.wait');
		timebar.width = totalTimeLen;
		waitbar.width = waitTimeLen;
		//timebar.style['width'] = totalTimeLen + 'px';
		//waitbar.style['width'] = waitTimeLen + 'px';
		timebar.title = timeText;
		waitbar.title = timeText;
		return el;
	}

	/**
	 * update all connections, this method is invoke by the client
	 * conns = [conn1, conn2, ...]
	 * conn = {id, socketID, requestHeader, responseHeader, startTime, responseStartTime, responseFinishTime}
	 * @param conns
	 */
	function updateAllConnections(conns){
		var i, len=conns.length;
		for(i=0; i<len; i++){
			var conn = conns[i];
			var c = Connection.get(conn.id, conn.socketID);
			if(!Connection.beginningTime && conn.startTime){
				Connection.beginningTime = conn.startTime;
			}
			c.setHeaders(
				conn.host, conn.url, conn.method, conn.status,
				conn.requestContentLength, conn.responseContentLength
			);
			c.setStartTime(conn.startTime);
			c.setResponseStartTime(conn.responseStartTime);
			c.setResponseFinishTime(conn.responseFinishTime);
			updateConnUI(c);
		}
		Connection.beginningTime = 0;
	}

	function toggleSocketDetailPanel(){
		var panel = document.getElementById('socketDetail');
		panel.classList.toggle('hidden');
		return !panel.classList.contains('hidden');
	}
	function updateSocketDetailPanel(el){
		var panel = document.getElementById('socketDetail');
		panel.left = el.absoluteLeft + 5;
		panel.top = el.absoluteTop + 5;

		var socketID = el.parentNode.getData('id');
		var conns = Connection.sockets[socketID];
		var requestCount = conns.length;
		var requestTime = 0, requestWaitTime = 0, responseTime = 0, idelTime = 0;
		var i, len=conns.length;
		for(i=0; i<len; i++){
			var c = conns[i];
			requestTime += c.getSessionTime();
			requestWaitTime += c.getWaitTime();
			responseTime += c.getResponseTime();
			idelTime += c.getIdelTime();
		}
		panel.querySelector('header').textContent = socketID;
		panel.querySelector('.item.requestCount > mark').textContent = requestCount;
		panel.querySelector('.item.requestTime > mark').textContent = requestTime;
		panel.querySelector('.item.requestWaitTime > mark').textContent = requestWaitTime;
		panel.querySelector('.item.responseTime > mark').textContent = responseTime;
		//panel.querySelector('.item.idelTime > mark').textContent = idelTime;
	}

	function main(){
		//var c = Connection.get(1, 2);
		//updateConnUI(c);
	}

	document.addEventListener('DOMContentLoaded', main);

	socketGroupsEl.delegate('.conn', 'click', function(e, el){
		el.classList.toggle('min');
	}, false, true);

	socketGroupsEl.delegate('.socket > header', 'click', function(e, el){
		var visible = toggleSocketDetailPanel();
		if(visible) updateSocketDetailPanel(el);
	}, false, true);

	document.getElementById('socketDetail').addEventListener('click', function(){
		toggleSocketDetailPanel();
	});

	window.Connection = Connection;
	window.updateAllConnections = updateAllConnections;

})();