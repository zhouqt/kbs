function kbsrcStringBuffer() {
	this.buffer = [];
}
kbsrcStringBuffer.prototype = {
	append: function(string) {
		this.buffer.push(string);
		return this;
	},
	toString: function() {
		return this.buffer.join("");
	}
};

function kbsrcHost(host, userid, httpRequest, protocol) {
	this.host = host;
	this.lastSync = 0;
	this.rc = new Object();
	this.dirty = new Object();
	this.serial = new Object();
	this.userid = userid;
	this.status = 0;
	this.XMLHttpRequest = httpRequest ? httpRequest : XMLHttpRequest;
	this.protocol = protocol ? protocol : "http:";
}
kbsrcHost.prototype = {
	BRCMaxItem: 50,
	hexD: "0123456789ABCDEF",
	toHex: function(buf, num, digits) {
		digits = digits * 4 - 4;
		while(digits >= 0) {
			buf.append(this.hexD.substr((num & (0xF << digits)) >> digits, 1));
			digits -= 4;
		}
	},
	getRcList: function(bid) {
		var lst = this.rc[bid];
		if (!lst) {
			if (!this.serial[bid])
				return null;
			this.unserializeBoard(this.serial[bid], 0);
			lst = this.rc[bid];
		}
		return lst;
	},
	isUnread: function(bid, id) {
		var lst = this.getRcList(bid);
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
	clear: function(bid, lastpost) {
		kbsrc.debugOut(bid + "," + lastpost + " clear.", this);
		var lst = this.getRcList(bid);
		if (!lst) return;
		lst[0] = lastpost;
		lst[1] = 0;
		this.dirty[bid] = true;
	},
	addRead: function(bid, id) {
		kbsrc.debugOut(bid + "," + id + " read.", this);
		var lst = this.rc[bid];
		var n;
		var lst = this.getRcList(bid);
		for(n=0; n<this.BRCMaxItem && lst[n]; n++) {
			if (id == lst[n]) {
				return;
			} else if (id > lst[n]) {
				for (var i=this.BRCMaxItem-1; i>n; i--) {
					lst[i] = lst[i-1];
				}
				lst[n] = id;
				this.dirty[bid] = true;
				return;
			}
		}
		if (n==0) {
			lst[0] = id;
			lst[1] = 1;
			lst[2] = 0;
			this.dirty[bid] = true;
		}
	},
	serialize: function(isSync) {
		var bid, j;
		var str = new kbsrcStringBuffer();
		for(bid in this.rc) {
			if (this.dirty[bid]) {
				var lst = this.getRcList(bid);
				if (!lst) continue;
				this.toHex(str, bid, 4);
				for (j=0; j<this.BRCMaxItem; j++) if (lst[j] == 0) break;
				this.toHex(str, j, 4);
				for (j=0; j<this.BRCMaxItem; j++) {
					if (lst[j] == 0) break;
					this.toHex(str, lst[j], 8);
				}
			} else if (!isSync) {
				str.append(this.serial[bid]);
			}
			if (!isSync) str.append(",");
		}
		return str.toString();
	},
	fullSerialize: function() {
		var str = new kbsrcStringBuffer();
		this.toHex(str, Math.floor(this.lastSync / 1000), 8);
		var i = 0;
		for(bid in this.rc) {
			if (!this.dirty[bid]) continue;
			i++;
		}
		this.toHex(str, i, 4);
		for(bid in this.rc) {
			if (!this.dirty[bid]) continue;
			this.toHex(str, bid, 4);
		}
		var v = str.toString() + "," + this.serialize(false);
		return v;
	},
	logoutSync: function() {
		var str = this.serialize(true);
		if (str) {
			this.sync(function() {
				//alert("RC Saved");
			}, true);
		}
	},
	setStatus: function(s) {
		this.status = s;
		kbsrc.setStatus(this.host);
	},
	unserializeBoard: function(rc, pos) {
		try {
			var bid = parseInt(rc.substr(pos, 4), 16);
			var n = parseInt(rc.substr(pos + 4, 4), 16);
			this.rc[bid] = new Array();
			this.dirty[bid] = false;
			for (j=0; j<this.BRCMaxItem; j++) this.rc[bid][j] = 0;
			for (j=0; j<n; j++) {
				this.rc[bid][j] = parseInt(rc.substr(pos+8+j*8, 8), 16);
			}
			this.serial[bid] = rc.substr(pos, 8 + n * 8);
			return(pos + 8 + n * 8);
		} catch(e) {}
	},
	unserialize: function(rc) {
		var i=0,j;
		try {
			while(i<rc.length) {
				i = this.unserializeBoard(rc, i);
			}
			kbsrc.debugOut("sync OK.", this);
		} catch(e) {
			kbsrc.debugOut("sync error.", this);
		}
	},
	fullUnserialize: function(str) {
		var i=0,j,n;
		try {
			var arr = str.split(",");
			this.lastSync = parseInt(arr[0].substr(0, 8), 16) * 1000;
			n = parseInt(arr[0].substr(8, 4), 16);
			i = 12;
			for(j = 0; j < n; j++) {
				var bid = parseInt(arr[0].substr(i, 4), 16);
				this.dirty[bid] = true;
				i += 4;
			}
			for(j = 1; j < arr.length; j++) {
				var bid = parseInt(arr[j].substr(0, 4), 16);
				this.rc[bid] = false;
				this.serial[bid] = arr[j];
			}
		} catch(e) {}
	},
	sync: function(callback, logout) {
		this.setStatus(1);
		var req = new this.XMLHttpRequest();
		req.oHost = this;
		req.callback = callback;
		req.onload = function(event) {
			var self = event.target;
			var rc = self.responseText;
			self.oHost.setStatus(0);
			if (rc.length == 0) {
				kbsrc.debugOut("sync failed.", self.oHost);
				return;
			}
			self.oHost.unserialize(rc);
			self.oHost.lastSync = (new Date()).getTime();
			if (self.callback) self.callback();
		};
		// TODO: use relative path
		req.open("POST", this.protocol + "//" + this.host + "/kbsrc.php" + (logout?"?logout=1":""), callback ? false : true);
		req.send(this.serialize(true));
	},
	processDoc: function(doc, detectOnly) {
		try {
			var metas = doc.getElementsByTagName("meta");
			for(var i = 0; i < metas.length; i++) {
				if (metas[i].name == "kbsrc.doc") {
					if (detectOnly) return 1;
					var bid = metas[i].content;
					var spans = doc.getElementsByTagName("span");
					for (var j=0; j<spans.length; j++) {
						var span = spans[j];
						if (span.id.substr(0, 5) != "kbsrc") continue;
						var thisid = parseInt(span.id.substr(5));
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
				} else if (metas[i].name == "kbsrc.brd") {
					if (detectOnly) return 1;
					var tds = doc.getElementsByTagName("td");
					for (var j=0; j<tds.length; j++) {
						var td = tds[j];
						if (td.id.substr(0, 5) != "kbsrc") continue;
						var as = td.id.substr(5).split("_");
						var bid = parseInt(as[0]);
						var lastpost = parseInt(as[1]);
						var unread = this.isUnread(bid, lastpost);
						if (unread === null) continue;
						var f = doc.getElementById("kbsrc"+bid+"u");
						if (f) f.style.display = unread ? "block" : "none";
						f = doc.getElementById("kbsrc"+bid+"r");
						if (f) f.style.display = !unread ? "block" : "none";
					}
				} else if (metas[i].name == "kbsrc.con") {
					if (detectOnly) return 1;
					var ids = metas[i].content.split(",");
					var bid = parseInt(ids[0]);
					var thisid = parseInt(ids[1]);
					if (ids[2]) {
						if (ids[2] == 'f') { //clear all
							this.clear(bid, thisid);
						}
					} else {
						this.addRead(bid, thisid);
					}
				} else if (metas[i].name == "kbsrc.menu") {
					if (detectOnly) return 2;
					var f = doc.getElementById("kbsrc_logout");
					var self = this;
					if (f) f.addEventListener("click", function() { self.logoutSync(); }, false);
				} else {
					continue;
				}
				break;
			}
		} catch(e) {}
		return 0;
	}
};

function kbsrcIEEntry() {
	kbsrc = new Object();
	kbsrc.debugOut = function() {};
	kbsrc.setStatus = function() {};
	kbsrc.XMLHttpRequest = function() {};
	kbsrc.XMLHttpRequest.prototype = {
		open: function(method, url, sync) {
			this.method = method;
			this.url = url;
			this.sync = sync;
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
			this.req.open(this.method, this.url, this.sync);
			this.req.send(this.data);
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
	
	var oHost = new kbsrcHost(document.location.host, getCookie("UTMPUSERID", "guest"), kbsrc.XMLHttpRequest, document.location.protocol);
	var ret = oHost.processDoc(document, true);
	if (ret == 0) return;

	var kbsrcStore = document.createElement("div");
	kbsrcStore.addBehavior("#default#userData");
	kbsrcStore.style.display = "none";
	document.appendChild(kbsrcStore);
	if (ret == 1) {
		kbsrcStore.load("kbsrcData");
		var data = kbsrcStore.getAttribute("sPersist");
		if (data) {
			oHost.fullUnserialize(data);
			oHost.processDoc(document);
			kbsrcStore.setAttribute("sPersist", oHost.fullSerialize());
			kbsrcStore.save("kbsrcData");
		}
	} else if (ret == 2) {
		var f = document.getElementById("kbsrc_logout");
		if (f) f.onclick = function() {
			kbsrcStore.load("kbsrcData");
			var data = kbsrcStore.getAttribute("sPersist");
			if (data) {
				oHost.fullUnserialize(data);
				oHost.logoutSync();
			}
		};
		oHost.sync(function() {
			kbsrcStore.setAttribute("sPersist", oHost.fullSerialize());
			kbsrcStore.save("kbsrcData");
		});
	}
}
