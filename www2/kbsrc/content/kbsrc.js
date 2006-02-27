function kbsrcHost(host, userid) {
	this.host = host;
	this.lastSync = 0;
	this.rc = new Object();
	this.dirty = new Object();
	this.userid = userid;
	this.status = 0;
}
kbsrcHost.prototype = {
	hexD: "0123456789ABCDEF",
	toHex: function(num, digits) {
		var ret = "";
		while(digits>0) {
			ret = this.hexD.substr(num & 15, 1) + ret;
			num >>= 4;
			digits--;
		}
		return ret;
	},
	isUnread: function(bid, id) {
		var lst = this.rc[bid];
		if (!lst) return null;
		for(var n=0; n<kbsrc.BRCMaxItem; n++) {
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
		var lst = this.rc[bid];
		if (!lst) return;
		lst[0] = lastpost;
		lst[1] = 0;
		this.dirty[bid] = true;
	},
	addRead: function(bid, id) {
		kbsrc.debugOut(bid + "," + id + " read.", this);
		var lst = this.rc[bid];
		var n;
		if (!lst) return;
		for(n=0; n<kbsrc.BRCMaxItem && lst[n]; n++) {
			if (id == lst[n]) {
				return;
			} else if (id > lst[n]) {
				for (var i=kbsrc.BRCMaxItem-1; i>n; i--) {
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
	getSyncString: function() {
		var bid, j, str = "";
		for(bid in this.dirty) {
			if (this.dirty[bid]) {
				var lst = this.rc[bid];
				str += this.toHex(bid, 4);
				for (j=0; j<kbsrc.BRCMaxItem; j++) if (lst[j] == 0) break;
				str += this.toHex(j, 4);
				for (j=0; j<kbsrc.BRCMaxItem; j++) {
					if (lst[j] == 0) break;
					str += this.toHex(lst[j], 8);
				}
			}
		}
		return str;
	},
	trySync: function() {
		var str = this.getSyncString();
		if (str) {
			this.sync(function() {
				//alert("RC Saved");
			});
		}
	},
	setStatus: function(s) {
		this.status = s;
		kbsrc.setStatus(this.host);
	},
	sync: function(callback) {
		this.setStatus(1);
		var req = new XMLHttpRequest();
		req.oHost = this;
		req.callback = callback;
		req.onload = function(event) {
			var self = event.target;
			var i=0,j,rc = self.responseText;
			self.oHost.setStatus(0);
			if (rc.length == 0) {
				kbsrc.debugOut("sync failed.", self.oHost);
				return;
			}
			try {
				while(i<rc.length) {
					var bid = parseInt(rc.substr(i, 4), 16);
					var n = parseInt(rc.substr(i+4, 4), 16);
					self.oHost.rc[bid] = new Array();
					self.oHost.dirty[bid] = false;
					for (j=0; j<kbsrc.BRCMaxItem; j++) self.oHost.rc[bid][j] = 0;
					for (j=0; j<n; j++) {
						self.oHost.rc[bid][j] = parseInt(rc.substr(i+8+j*8, 8), 16);
					}
					i += 8 + n * 8;
				}
				kbsrc.debugOut("sync OK.", self.oHost);
			} catch(e) {
				kbsrc.debugOut("sync error.", self.oHost);
			}
			self.oHost.lastSync = (new Date()).getTime();
			if (self.callback) self.callback();
		};
		/* TODO: use relative path */
		req.open("POST", "http://" + this.host + "/kbsrc.php", callback ? false : true);
		req.send(this.getSyncString());
	},
	processDoc: function(doc) {
		try {
			var metas = doc.getElementsByTagName("meta");
			for(var i = 0; i < metas.length; i++) {
				if (metas[i].name == "kbsrc.doc") {
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
				} else if (metas[i].name == "kbsrc.brd") {
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
					var f = doc.getElementById("logoutlink");
					if (f) f.addEventListener("click", function() { this.trySync(); }, false);
				} else {
					continue;
				}
				break;
			}
		} catch(e) {}
	}
};
