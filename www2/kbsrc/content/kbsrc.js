function kbsrcHost(host, userid) {
	this.host = host;
	this.lastSync = 0;
	this.rc = new Object();
	this.dirty = new Object();
	this.userid = userid;
	this.status = 0;
}
kbsrcHost.prototype = {
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
				str += kbsrc.toHex(bid, 4);
				for (j=0; j<kbsrc.BRCMaxItem; j++) if (lst[j] == 0) break;
				str += kbsrc.toHex(j, 4);
				for (j=0; j<kbsrc.BRCMaxItem; j++) {
					if (lst[j] == 0) break;
					str += kbsrc.toHex(lst[j], 8);
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
	}
};
