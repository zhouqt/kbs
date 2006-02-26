/* TODO: 意外断线的情况：需要检查 cookie */
function KBSRC() {}
KBSRC.prototype = {
	data: false,
	BRCMaxItem: 50,
	isUnread: function(host, bid, id) {
		var lst = this.data[host].rc[bid];
		if (!lst) return false;
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
	addRead: function(host, bid, id) {
		var lst = this.data[host].rc[bid];
		var n;
		if (!lst) return;
		for(n=0; n<this.BRCMaxItem && lst[n]; n++) {
			if (id == lst[n]) {
				return;
			} else if (id > lst[n]) {
				for (var i=this.BRCMaxItem-1; i>n; i--) {
					lst[i] = lst[i-1];
				}
				lst[n] = id;
				this.data[host].dirty[bid] = true;
				return;
			}
		}
		if (n==0) {
			lst[0] = id;
			lst[1] = 1;
			lst[2] = 0;
			this.data[host].dirty[bid] = true;
		}
	},
	timer : function() {
		var host, o, now = (new Date()).getTime();
		for (host in this.data) {
			o = this.data[host];
			if (!o) continue;
			if (o.lastUserid != o.userid) {
				o.userid = o.lastUserid;
				new kbsrcHttpRequest(host);
			} else {
				if (now - o.lastSync > 600000) {
					o.lastSync = now;
					new kbsrcHttpRequest(host);
				}
			}
		}
	},
	init : function() {
		var browser = gBrowser;
		browser.addEventListener("DOMContentLoaded", kbsrcPageLoadedHandler, true);
		
		var hw = new kbsrcHTTPHeaderWatcher();
		var observerService = Components.classes["@mozilla.org/observer-service;1"].getService(Components.interfaces.nsIObserverService);
		observerService.addObserver(hw, "http-on-examine-response", false);
	    
		this.data = new Object();
	    
	    var self = this;
		setInterval(function() {
			self.timer.call(self);
		}, 1000);
	},
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
	getSyncString: function(host) {
		var bid, j, str = "";
		if (kbsrc.data[host]) {
			for(bid in kbsrc.data[host].dirty) {
				if (kbsrc.data[host].dirty[bid]) {
					var lst = kbsrc.data[host].rc[bid];
					str += kbsrc.toHex(bid, 4);
					for (j=0; j<kbsrc.BRCMaxItem; j++) if (lst[j] == 0) break;
					str += kbsrc.toHex(j, 4);
					for (j=0; j<kbsrc.BRCMaxItem; j++) {
						if (lst[j] == 0) break;
						str += kbsrc.toHex(lst[j], 8);
					}
				}
			}
		}
		return str;
	},
	trySync: function(host, doc) {
		var str = this.getSyncString(host);
		if (str) {
			new kbsrcHttpRequest(host, function() {
				//alert("RC Saved");
			});
		}
	}
};

var kbsrcHttpRequest = function(host, callback) {
	var req = new XMLHttpRequest();
	var bid, n, j;
	req.host = host;
	req.callback = callback;
	req.onload = function(event) {
		var self = event.target;
    	var i=0,rc = self.responseText;
    	try {
    		while(i<rc.length) {
    			bid = parseInt(rc.substr(i, 4), 16);
    			n = parseInt(rc.substr(i+4, 4), 16);
    			kbsrc.data[self.host].rc[bid] = new Array();
    			kbsrc.data[self.host].dirty[bid] = false;
    			for (j=0; j<kbsrc.BRCMaxItem; j++) kbsrc.data[self.host].rc[bid][j] = 0;
    			for (j=0; j<n; j++) {
    				kbsrc.data[self.host].rc[bid][j] = parseInt(rc.substr(i+8+j*8, 8), 16);
    			}
    			i += 8 + n * 8;
    		}
    	} catch(e) {}
    	kbsrc.data[self.host].lastSync = (new Date()).getTime();
    	if (self.callback) self.callback();
	};
	var str = kbsrc.getSyncString(host);
	/* TODO: use relative path */
	req.open("POST", "http://" + host + "/kbsrc.php", callback ? false : true);
	req.send(str);
}
	
var kbsrcPageLoadedHandler = function(event) {
	const doc = event.originalTarget;
	if(!(doc instanceof HTMLDocument)) return;
	
	if(doc._kbsrc_haveChecked) return;
	doc._kbsrc_haveChecked = true;
	
	const protocol = doc.location.protocol;
	if(!/^(?:https|http)\:$/.test(protocol)) return;
	
	const host = doc.location.host;
	if (!kbsrc.data[host]) return;

	var metas = doc.getElementsByTagName("meta");
	for(var i = 0; i < metas.length; i++) {
		if (metas[i].name == "kbsrc.doc") {
			var bid = metas[i].content;
			var tds = doc.getElementsByTagName("td");
			for (var j=0; j<tds.length; j++) {
				var td = tds[j];
				if (td.id.substr(0, 5) != "kbsrc") continue;
				var thisid = td.id.substr(5);
				if (kbsrc.isUnread(host, bid, thisid)) td.innerHTML += "*";
			}
			break;
		} else if (metas[i].name == "kbsrc.con") {
			var ids = metas[i].content.split(",");
			var bid = ids[0];
			var thisid = ids[1];
			if (ids[2]) {
				if (ids[2] == 'f') { //clear all
					
				}
			} else {
				kbsrc.addRead(host, bid, thisid);
			}
		} else if (metas[i].name == "kbsrc.menu") {
			var f = doc.getElementById("logoutlink");
			if (f) f.addEventListener("click", function() { kbsrc.trySync(host, doc); }, false);
		}
	}
};

function kbsrcHTTPHeaderWatcher() {}
kbsrcHTTPHeaderWatcher.prototype = {
	observe: function(aSubject, aTopic, aData) {
		if (aTopic == 'http-on-examine-response') {
			aSubject.QueryInterface(Components.interfaces.nsIHttpChannel);
			this.onExamineResponse(aSubject);
		}
	},
	onExamineResponse : function (oHttp) {
		var value = false;
		try {
			value = oHttp.getResponseHeader("Set-KBSRC");
		} catch(e) {}
		if (value) {
			var host = oHttp.URI.host;
			if (value == "/") {
				kbsrc.data[host] = false;
			} else {
				kbsrc.data[host] = new Object();
				kbsrc.data[host].lastUserid = false;
				kbsrc.data[host].lastSync = false;
	        	kbsrc.data[host].rc = new Object();
	        	kbsrc.data[host].dirty = new Object();
				kbsrc.data[host].userid = value;
			}
		}
	}
};

var kbsrc = new KBSRC();
window.addEventListener("load", function() { kbsrc.init.call(kbsrc) }, false); 
