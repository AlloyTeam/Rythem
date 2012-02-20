(function(){

	/**
	 * Connection
	 * @param {uint} id
	 * @param {String} socketID
	 */
	var Connection = function(id, socketID){
		this.id = id;
		this.socketID = socketID;
	};
	Connection.prototype = {
		/**
		 * set request header
		 * @param {String} header
		 */
		setRequestHeader: function(header){
			this.requestHeader = header;
			//TODO parse the request header
			return this;
		},
		/**
		 * set response header
		 * @param {String} header
		 */
		setResponseHeader: function(header){
			this.responseHeader = header;
			return this;
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
			return 'www.???.com';
		},
		getRequestName: function(){
			return this.id.toString();
		},
		getRequestHost: function(){
			return 'www.???.com';
		},
		getRequestMethod: function(){
			return 'GET';
		},
		getResponseStatus: function(){
			return '0';
		},
		getStartTime: function(){
			return this.startTime
		},
		getResponseStartTime: function(){
			return this.responseStartTime
		},
		getResponseFinishTime: function(){
			return this.responseFinishTime
		},
		getWaitTime: function(){
			return this.responseStartTime - this.startTime;
		},
		getResponseTime: function(){
			return this.responseFinishTime - this.responseStartTime;
		},
		getSessionTime: function(){
			return this.responseFinishTime - this.startTime;
		},
		toString: function(){
			return JSON.stringify({
				"id": 					this.id,
				"socketID": 			this.socketID,
				"startTime": 			this.getStartTime(),
				"responseStartTime": 	this.getResponseStartTime(),
				"responseFinishTime": 	this.getResponseFinishTime(),
				"waitTime": 			this.getWaitTime(),
				"responseTime": 		this.getResponseTime(),
				"sessionTime": 			this.getSessionTime()
			});
		}
	};
	Connection.sockets = {};
	Connection.conns = {};
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

	function getSocketUI(socketID){
		var el = document.getElementById('socket-' + socketID);
		if(el){
			return el;
		}
		else{
			var group = document.createElement('article');
			group.id = 'socket-' + socketID;
			group.className = 'socket hgroup';
			group.innerHTML = '<header>' + socketID + '</header><div class="conns hgroup"></div>';
			socketGroupsEl.appendChild(group);
			return group;
		}
	}

	function updateConnUI(conn){
		var el = document.getElementById('conn-' + conn.id);
		if(el){
			el.querySelector('.name').textContent = conn.getRequestName();
			el.querySelector('.host').textContent = conn.getRequestHost();
			el.querySelector('.detail').textContent = conn.getRequestMethod() + ' ' + conn.getResponseStatus();
			return el;
		}
		else{
			var item = document.createElement('div');
			item.id = 'conn-' + conn.id;
			item.className = 'conn';
			item.innerHTML = '\
				<div class="name">' + conn.getRequestName() + '</div>\
				<div class="host">' + conn.getRequestHost() + '</div>\
				<div class="detail">' + conn.getRequestMethod() + ' ' + conn.getResponseStatus() + '</div> ';
			var socket = getSocketUI(conn.socketID);
			socket.querySelector('.conns').appendChild(item);
			return item;
		}
	}

	function main(){
		var c1 = Connection.get(1, 'socket a');
		var c2 = Connection.get(2, 'socket a');
		var c3 = Connection.get(3, 'socket b');
		var c4 = Connection.get(4, 'socket a');
		updateConnUI(c1);
		updateConnUI(c2);
		updateConnUI(c3);
		updateConnUI(c4);
	}

	window.Connection = Connection;
	document.addEventListener('DOMContentLoaded', main);

})();