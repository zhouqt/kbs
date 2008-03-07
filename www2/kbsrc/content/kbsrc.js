/**
 * Part of the KBS BBS Code
 * Copyright (c) 2005-2008 KBS Development Team. (http://dev.kcn.cn/)
 * Source file is subject to the pending KBS License.
 *
 * You may use and/or modify the source code only for pure personal study
 * purpose (e.g. using it in a public website is not acceptable), unless
 * you get explicit permission by the KBS Development Team.
 */

function kbsrcStringBuffer() {
	this.buffer = [];
}
kbsrcStringBuffer.prototype = {
	append: function(string) {
		this.buffer.push(string);
		return this;
	},
	hexD: "0123456789ABCDEF",
	appendHex: function(num, digits) {
		digits = digits * 4 - 4;
		while(digits >= 0) {
			this.append(this.hexD.substr((num & (0xF << digits)) >> digits, 1));
			digits -= 4;
		}
	},
	toString: function() {
		return this.buffer.join("");
	}
};

function kbsrcBoard(bid, oHost, createRC) {
	this.bid = bid;
	this.rc = createRC ? new Array() : false;
	this.serial = false;
	this.dirty = false;
	this.oHost = oHost;
}
kbsrcBoard.prototype = {
	BRCMaxItem: 50,
	getRcList: function() {
		var lst = this.rc;
		if (!lst) {
			if (!this.serial)
				return null;
			try {
				this.oHost.unserializeBoard(this.serial, 0, this);
			} catch(e) {
				return null;
			}
			lst = this.rc;
		}
		return lst;
	},
	isUnread: function(id) {
		var lst = this.getRcList();
		if (!lst) return null;
		for(var n=0; n<this.BRCMaxItem; n++) {
			if (lst[n] == 0) {
				if (n == 0) return true;
				return false;
			}
			if (id > lst[n]) {
				return true;
			} else if (id == lst[n]) {
				return false;
			}
		}
		return false;
	},
	clear: function(lastpost) {
		kbsrc.debugOut(this.bid + "," + lastpost + " clear.", this.oHost);
		var lst = this.getRcList();
		if (!lst) return;
		lst[0] = lastpost;
		lst[1] = 0;
		this.dirty = true;
	},
	addRead: function(id) {
		kbsrc.debugOut(this.bid + "," + id + " read.", this.oHost);
		var n;
		var lst = this.getRcList();
		for(n=0; n<this.BRCMaxItem && lst[n]; n++) {
			if (id == lst[n]) {
				return;
			} else if (id > lst[n]) {
				for (var i=this.BRCMaxItem-1; i>n; i--) {
					lst[i] = lst[i-1];
				}
				lst[n] = id;
				this.dirty = true;
				return;
			}
		}
		if (n==0) {
			lst[0] = id;
			lst[1] = 1;
			lst[2] = 0;
			this.dirty = true;
		}
	}
};

function kbsrcHost(host, userid, utmpkey, httpRequest, protocol) {
	this.host = host;
	this.lastSync = 0;
	this.boards = new Object();
	this.userid = userid;
	this.status = 0;
	this.utmpkey = utmpkey ? utmpkey : 0;
	this.XMLHttpRequest = httpRequest ? httpRequest : XMLHttpRequest;
	this.protocol = protocol ? protocol : "http:";
}
kbsrcHost.prototype = {
	serialize: function(isSync) {
		var bid, j;
		var str = new kbsrcStringBuffer();
		for(bid in this.boards) {
			var board = this.boards[bid];
			if (board.dirty) {
				var lst = board.getRcList();
				if (!lst) continue;
				str.appendHex(bid, 4);
				for (j=0; j<board.BRCMaxItem; j++) if (lst[j] == 0) break;
				str.appendHex(j, 4);
				for (j=0; j<board.BRCMaxItem; j++) {
					if (lst[j] == 0) break;
					str.appendHex(lst[j], 8);
				}
			} else if (!isSync) {
				str.append(board.serial);
			}
			if (!isSync) str.append(",");
		}
		return str.toString();
	},
	ieFullSerialize: function() {
		var str = new kbsrcStringBuffer();
		str.appendHex(this.utmpkey, 8);
		str.appendHex(Math.floor(this.lastSync / 1000), 8);
		var i = 0;
		for(bid in this.boards) {
			if (!this.boards[bid].dirty) continue;
			i++;
		}
		str.appendHex(i, 4);
		for(bid in this.boards) {
			if (!this.boards[bid].dirty) continue;
			str.appendHex(bid, 4);
		}
		var v = str.toString() + "," + this.serialize(false);
		return v;
	},
	logoutSync: function() {
		var str = this.serialize(true);
		if (str) {
			this.sync(null, true);
		}
	},
	setStatus: function(s) {
		this.status = s;
		kbsrc.setStatus(this.host);
	},
	unserializeBoard: function(rc, pos, oBoard) {
		var bid = parseInt(rc.substr(pos, 4), 16);
		var n = parseInt(rc.substr(pos + 4, 4), 16);
		var j, oB = oBoard;
		if (!oB) {
			oB = this.boards[bid] = new kbsrcBoard(bid, this, true);
			oB.serial = rc.substr(pos, 8 + n * 8);
		} else {
			oB.rc = new Array();
		}
		for (j=0; j<oB.BRCMaxItem; j++) oB.rc[j] = 0;
		for (j=0; j<n; j++) {
			oB.rc[j] = parseInt(rc.substr(pos+8+j*8, 8), 16);
		}
		return(pos + 8 + n * 8);
	},
	unserialize: function(rc) {
		var i=0;
		this.boards = new Object();
		try {
			while(i<rc.length) {
				i = this.unserializeBoard(rc, i);
			}
			kbsrc.debugOut("sync OK.", this);
		} catch(e) {
			kbsrc.debugOut("sync error.", this);
		}
	},
	ieFullUnserialize: function(str) {
		var i=0,j,n;
		try {
			this.boards = new Object();
			var arr = str.split(",");
			this.utmpkey = parseInt(arr[0].substr(0, 8), 16);
			this.lastSync = parseInt(arr[0].substr(8, 8), 16) * 1000;

			for(j = 1; j < arr.length; j++) {
				var bid = parseInt(arr[j].substr(0, 4), 16);
				this.boards[bid] = new kbsrcBoard(bid, this);
				this.boards[bid].serial = arr[j];
			}

			n = parseInt(arr[0].substr(16, 4), 16);
			i = 20;
			for(j = 0; j < n; j++) {
				var bid = parseInt(arr[0].substr(i, 4), 16);
				this.boards[bid].dirty = true;
				i += 4;
			}
		} catch(e) {}
	},
	sync: function(callback, logout, nopost) {
		this.setStatus(1);
		var req = new this.XMLHttpRequest();
		req.oHost = this;
		req.callback = callback;
		req.onload = function(event) {
			var self = event.target;
			var rc = self.responseText;
			self.oHost.setStatus(0);
			if (rc.length == 0) {
				kbsrc.debugOut(logout?"logout sync done.":"sync failed.", self.oHost);
				return;
			}
			self.oHost.unserialize(rc);
			self.oHost.lastSync = (new Date()).getTime();
			if (self.callback) self.callback();
		};
		// TODO: use relative path
		req.open("POST", this.protocol + "//" + this.host + "/kbsrc.php?userid=" + this.userid + (logout?"&logout=1":""), !logout);
		req.send(nopost ? "" : this.serialize(true));
	},
	isUnread: function(bid, id) {
		var board = this.boards[bid];
		if (!board) return null;
		return (board.isUnread(id));
	},
	addRead: function(bid, id) {
		var board = this.boards[bid];
		if (board) board.addRead(id);
	},
	clear: function(bid, id) {
		var board = this.boards[bid];
		if (board) board.clear(id);
	},
	processDoc: function(doc, detectOnly) {
		try {
			var info = doc.getElementById('kbsrcInfo');
			if (!info) return 0;
			info = info.innerHTML.split(',');
			if (info[0] == 'doc') {
				if (detectOnly) return 1;
				var bid = info[1];
				for(var j=2; j<info.length; j++) {
					var thisid = parseInt(info[j]);
					var span = doc.getElementById('kbsrc' + thisid);
					if (!span) continue;
					var html = span.innerHTML;
					var unread = this.isUnread(bid, thisid);
					if (unread === null) continue;
					var c = html.substr(0, 1);
					var cl = c.toLowerCase();
					if (c == ' ') c = unread ? '*' : ' ';
					else if (cl == 'b' || cl == 'm' || cl == 'g') c = unread ? c.toUpperCase() : c.toLowerCase();
					else c = (unread ? '*' : '') + c;
					span.innerHTML = c + html.substr(1);
				}
				var f = doc.getElementById("kbsrc_clear");
				if (f) f.style.display = "inline";
			} else if (info[0] == 'brd') {
				if (detectOnly) return 1;
				for (var j=1; j<info.length; j+=2) {
					var bid = parseInt(info[j]);
					var lastpost = parseInt(info[j+1]);
					var unread = this.isUnread(bid, lastpost);
					if (unread === null) continue;
					var f = doc.getElementById("kbsrc"+bid+"u");
					if (f) f.style.display = unread ? "block" : "none";
					f = doc.getElementById("kbsrc"+bid+"r");
					if (f) f.style.display = !unread ? "block" : "none";
				}
			} else if (info[0] == 'con') {
				if (detectOnly) return 1;
				var bid = parseInt(info[1]);
				var thisid = parseInt(info[2]);
				if (info[3]) {
					if (info[3] == 'f') { //clear all
						this.clear(bid, thisid);
					}
				} else {
					this.addRead(bid, thisid);
				}
			} else if (info[0] == 'tcon') {
				if (detectOnly) return 1;
				var bid = parseInt(info[1]);
				for(var j=2; j<info.length; j++) {
					var id = parseInt(info[j]);
					this.addRead(bid, id);
				}
			} else if (info[0] == 'menu') {
				if (detectOnly) return 2;
				var f = doc.getElementById("kbsrc_logout");
				var self = this;
				if (f) f.addEventListener("click", function() { self.logoutSync(); }, false);
			}
		} catch(e) {}
		return 0;
	}
};

function kbsrcIEEntry() {
	if (kbsrc !== null) return;
	kbsrc = new Object();
	kbsrc.debugOut = function() {};
	kbsrc.setStatus = function() {};
	kbsrc.XMLHttpRequest = function() {};
	kbsrc.XMLHttpRequest.prototype = {
		open: function(method, url, async) {
			this.method = method;
			this.url = url;
			this.async = async;
		},
		send: function(data) {
			try {
				this.req = new XMLHttpRequest();
			} catch(e) {
				this.req = new ActiveXObject("Microsoft.XMLHTTP");
			}
			var self = this;
			this.req.onreadystatechange = function() {
				self.onStateChange.call(self);
			};
			this.req.open(this.method, this.url, this.async);
			this.req.send(data);
		},
		onStateChange : function() {
			if (this.req.readyState == 4 && this.req.status == 200) {
				this.responseText = this.req.responseText;
				var ev = new Object();
				ev.target = this;
				this.onload(ev);
			}
		}
	}
	
	var userid = getCookie("UTMPUSERID", "guest");
	var oHost = new kbsrcHost(document.location.host, userid, getCookie("UTMPKEY", ""), kbsrc.XMLHttpRequest, document.location.protocol);
	var ret = oHost.processDoc(document, true);
	if (ret == 0) return;

	var kbsrcStore = document.createElement("div");
	kbsrcStore.addBehavior("#default#userData");
	kbsrcStore.style.display = "none";
	document.appendChild(kbsrcStore);
	
	var kbsrcIELoad = function() {
		kbsrcStore.load("kbsrcData" + userid);
		var data = kbsrcStore.getAttribute("sPersist");
		if (data) {
			oHost.ieFullUnserialize(data);
			return true;
		} else return false;
	};
	var kbsrcIESave = function(remove) {
		if (remove) kbsrcStore.removeAttribute("sPersist");
		else kbsrcStore.setAttribute("sPersist", oHost.ieFullSerialize());
		kbsrcStore.save("kbsrcData" + userid);
	};
	
	if (ret == 1) {
		if (kbsrcIELoad()) {
			oHost.processDoc(document);
			kbsrcIESave();
		}
	} else if (ret == 2) {
		var f = document.getElementById("kbsrc_logout");
		if (f) f.onclick = function() {
			if (kbsrcIELoad()) {
				oHost.logoutSync();
			}
			kbsrcIESave(true);
		};
		var toLoad = true;
		if (kbsrcIELoad()) {
			if (oHost.utmpkey == getCookie("UTMPKEY", "")) toLoad = false;
		}
		if (toLoad) {
			oHost.sync(function() {
				kbsrcIESave();
			}, false, true);
		}
		setInterval(function() {
			if (!kbsrcIELoad()) return;
			var now = (new Date()).getTime();
			if (now - oHost.lastSync > 600000) {
				oHost.lastSync = now;
				kbsrcIESave();
				oHost.sync(function() {
					kbsrcIESave();
				});
			}
		}, 310000);
	}
}
