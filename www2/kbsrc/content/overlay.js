var gKbsrcData = false;

var kbsrcPageLoadedHandler = function(event) {
	var doc = event.originalTarget;
	if(!(doc instanceof HTMLDocument)) return;
	
	if(doc._kbsrc_haveChecked) return;
	doc._kbsrc_haveChecked = true;
	
	const protocol = doc.location.protocol;
	if(!/^(?:https|http)\:$/.test(protocol)) return;
	
	const domain = doc.location.domain;
	
	var metas = doc.getElementsByTagName("meta");
	for(var i = 0; i < metas.length; i++) {
		if (metas[i].name == "kbsrc.doc") {
			//alert("DOC - " + metas[i].content);
			break;
		} else if (metas[i].name == "kbsrc.con") {
			//alert("CON - " + metas[i].content);
			break;
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
	this.req.open("GET", "http://" + host + "/kbsrc.php", true);
	this.req.send(null);
}
kbsrcHttpRequest.prototype = {
	onStateChange : function() {
        if (this.req.readyState == 4 && this.req.status == 200) {
        	alert(this.req.responseText.length);
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
