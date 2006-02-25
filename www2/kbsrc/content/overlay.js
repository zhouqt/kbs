var gKbsrcData = false;
var gBRCMaxItem = 50;

function unread(host, bid, id) {
	var lst = gKbsrcData[host].rc[bid];
	for(var n=0; n<gBRCMaxItem; n++) {
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
}
function addread(host, bid, id) {
	var lst = gKbsrcData[host].rc[bid];
	var n;
	for(n=0; n<gBRCMaxItem && lst[n]; n++) {
		if (id == lst[n]) {
			return;
		} else if (id > lst[n]) {
			for (var i=gBRCMaxItem-1; i>n; i--) {
				lst[i] = lst[i-1];
			}
			lst[n] = id;
			gKbsrcData[host].dirty[bid] = true;
			return;
		}
	}
	if (n==0) {
		lst[0] = id;
		lst[1] = 1;
		lst[2] = 0;
		gKbsrcData[host].dirty[bid] = true;
	}
}

var kbsrcPageLoadedHandler = function(event) {
	var doc = event.originalTarget;
	if(!(doc instanceof HTMLDocument)) return;
	
	if(doc._kbsrc_haveChecked) return;
	doc._kbsrc_haveChecked = true;
	
	const protocol = doc.location.protocol;
	if(!/^(?:https|http)\:$/.test(protocol)) return;
	
	var host = doc.location.host;
	if (!gKbsrcData[host]) return;

	var metas = doc.getElementsByTagName("meta");
	for(var i = 0; i < metas.length; i++) {
		if (metas[i].name == "kbsrc.doc") {
			var bid = metas[i].content;
			var tds = doc.getElementsByTagName("td");
			for (var j=0; j<tds.length; j++) {
				var td = tds[j];
				if (td.id.substr(0, 5) != "kbsrc") continue;
				var thisid = td.id.substr(5);
				if (unread(host, bid, thisid)) td.innerHTML += "*";
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
				addread(host, bid, thisid);
			}
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
			if (!gKbsrcData[host]) {
				gKbsrcData[host] = new Object();
				gKbsrcData[host].lastUserid = false;
			}
			gKbsrcData[host].userid = ((value == "/") ? false : value);
		}
	}
};

var kbsrcHttpRequest = function(host) {
	this.host = host;
	this.req = new XMLHttpRequest();
	
	var self = this;
	this.req.onreadystatechange = function() {
		self.onStateChange.call(self);
	};
	/* TODO: relative path */
	this.req.open("GET", "http://" + host + "/kbsrc.php", true);
	this.req.send(null);
}
kbsrcHttpRequest.prototype = {
	onStateChange : function() {
        if (this.req.readyState == 4 && this.req.status == 200) {
        	gKbsrcData[this.host].rc = new Array();
        	gKbsrcData[this.host].dirty = new Array();
        	var i=0,j,rc = this.req.responseText;
        	try {
        		while(i<rc.length) {
        			var bid = parseInt(rc.substr(i, 4), 16);
        			var n = parseInt(rc.substr(i+4, 4), 16);
        			gKbsrcData[this.host].rc[bid] = new Array();
        			gKbsrcData[this.host].dirty[bid] = false;
        			for (j=0; j<gBRCMaxItem; j++) gKbsrcData[this.host].rc[bid][j] = 0;
        			for (j=0; j<n; j++) {
        				gKbsrcData[this.host].rc[bid][j] = parseInt(rc.substr(i+8+j*8, 8), 16);
        			}
        			i += 8 + n * 8;
        		}
        	} catch(e) {}
        }
    }
};

var kbsrcInit = function() {
	var browser = gBrowser;
	browser.addEventListener("DOMContentLoaded", kbsrcPageLoadedHandler, true);
	
	var hw = new kbsrcHTTPHeaderWatcher();
	var observerService = Components.classes["@mozilla.org/observer-service;1"].getService(Components.interfaces.nsIObserverService);
	observerService.addObserver(hw, "http-on-examine-response", false);
    
	gKbsrcData = new Array();
    
	setInterval(function() {
		var host, o;
		for (host in gKbsrcData) {
			o = gKbsrcData[host];
			if (o.lastUserid != o.userid) {
				o.userid = o.lastUserid;
				new kbsrcHttpRequest(host);
			}    		
		}
	}, 1000);
};

window.addEventListener("load", kbsrcInit, false); 
