/**
 * Part of the KBS BBS Code
 * Copyright (c) 2005-2008 KBS Development Team. (http://dev.kcn.cn/)
 * Source file is subject to the pending KBS License.
 *
 * You may use and/or modify the source code only for pure personal study
 * purpose (e.g. using it in a public website is not acceptable), unless
 * you get explicit permission by the KBS Development Team.
 */

function KBSRC() {}
KBSRC.prototype = {
	hosts: false,
	dumpInfo: function() {
		var str = "dirty BRC as follows:\n";
		var bid, host;
		for (host in this.hosts) {
			var oHost = this.hosts[host];
			if (!oHost) continue;
			str += "\t" + oHost.host + ":\n";
			for(bid in oHost.boards) {
				var oBoard = oHost.boards[bid];
				if (oBoard.dirty) {
					var lst = oBoard.rc;
					str += "\t\t" + bid + ": ";
					for (j=0; j<oBoard.BRCMaxItem; j++) {
						if (lst[j] == 0) break;
						if (j!=0) str += ",";
						str += lst[j];
					}
					str += "\n";
				}
			}
		}
		kbsrc.debugOut(str);
	},
	setStatus: function(host, force) {
		if (!force) {
			try {
				var curHost = gBrowser.currentURI.host;
				if (curHost != host) return;
			} catch(e) {
				return;
			}
		}
		var active = host && kbsrc.hosts[host];
		if (active) {
			var txt, img;
			const oHost = kbsrc.hosts[host];
			if (oHost.status == 1) {
				txt = "KBSRC Activated (synchronizing)"
				img = "chrome://kbsrc/skin/kbsrcTransfer.gif";
			} else {
				var minutes = -1;
				if (oHost.lastSync > 0) {
					minutes = Math.floor(((new Date()).getTime() - oHost.lastSync) / 60000);
				}
				txt = "KBSRC Activated (last sync: " + ((minutes>=0) ? (minutes + " minutes before") : "never") + ")";
				img = "chrome://kbsrc/skin/kbsrcEnabled.gif";
			}
			document.getElementById('kbsrc-tooltip-value').setAttribute("value", txt);
			document.getElementById('kbsrc-status-image').setAttribute("src", img);
		}
		document.getElementById('kbsrc-status').setAttribute("collapsed", !active);
		//kbsrc.debugOut("status set: " + host);
	},
	debugOut: function(str, oHost) {
		var now = new Date();
		var d = now.getHours() + ":" + now.getMinutes() + ":" + now.getSeconds();
		var d1 = "";
		if (oHost) d1 = oHost.host + "(" + oHost.userid + ") ";
		dump("KBSRC: [" + d + "] " + d1 + str + "\n");
	},
	init : function() {
		var browser = gBrowser;
		browser.addEventListener("DOMContentLoaded", kbsrcPageLoadedHandler, true);
		browser.addEventListener("select", kbsrcTabSelectedHandler, false);
		browser.addEventListener("pageshow", kbsrcPageShowHandler, false);
		browser.addEventListener("unload", kbsrcPageClosedHandler, true);
		browser.addEventListener("pagehide", kbsrcPageClosedHandler, false);

		this.hWatcher = new kbsrcHTTPHeaderWatcher();
		var observerService = Components.classes["@mozilla.org/observer-service;1"].getService(Components.interfaces.nsIObserverService);
		observerService.addObserver(this.hWatcher, "http-on-examine-response", false);
		
		this.hosts = new Object();
		
		kbsrc.debugOut("Loaded OK.");
	},
	deinit : function() {
		var browser = gBrowser;
		browser.removeEventListener("DOMContentLoaded", kbsrcPageLoadedHandler, true);
		browser.removeEventListener("select", kbsrcTabSelectedHandler, false);
		browser.removeEventListener("pageshow", kbsrcPageShowHandler, false);
		browser.removeEventListener("unload", kbsrcPageClosedHandler, true);
		browser.removeEventListener("pagehide", kbsrcPageClosedHandler, false);

		var observerService = Components.classes["@mozilla.org/observer-service;1"].getService(Components.interfaces.nsIObserverService);
		observerService.removeObserver(this.hWatcher, "http-on-examine-response", false);

		this.hosts = false;
		
		kbsrc.debugOut("Unloaded OK.");
	}
};

function kbsrcTabSelectedHandler(event) {
	var n = event.originalTarget.localName;
	if (n == "tabs") kbsrcPageRefresh(true);
	else if (n == "tabpanels") kbsrc.setStatus(false, true);
}
function kbsrcPageShowHandler(event) {
	const doc = event.originalTarget;
	if(doc == gBrowser.contentDocument) kbsrcPageRefresh(false);
}
function kbsrcPageClosedHandler(event) {
	const doc = event.originalTarget;
	if(!(doc instanceof Document)) doc = doc.ownerDocument;
	if(doc != gBrowser.contentDocument) return;
	kbsrc.setStatus(false);
}
function kbsrcPageRefresh(force) {
	var host = false;
	try {
		var doc = content.document;
		host = doc.location.host;
	} catch(e) {}
	kbsrc.setStatus(host, force);
}
function kbsrcPageLoadedHandler(event) {
	kbsrcPageShowHandler(event);
	const doc = event.originalTarget;
	if(!doc instanceof HTMLDocument) return;
	
	if (doc._kbsrc_haveChecked) return;
	
	doc._kbsrc_haveChecked = true;
	
	const protocol = doc.location.protocol;
	if (!/^(?:https|http)\:$/.test(protocol)) return;
	
	const host = doc.location.host;
	const oHost = kbsrc.hosts[host];
	if (!oHost) return;
	
	oHost.processDoc(doc);

	var now = new Date().getTime();
	if (now - oHost.lastSync > 600000) {
		oHost.lastSync = now;
		oHost.sync();
	}
}

function kbsrcHTTPHeaderWatcher() {}
kbsrcHTTPHeaderWatcher.prototype = {
	observe: function(aSubject, aTopic, aData) {
		if (aTopic == 'http-on-examine-response') {
			aSubject.QueryInterface(Components.interfaces.nsIHttpChannel);
			this.onExamineResponse(aSubject);
		}
	},
	getCookie: function(oHttp, name, def) {
		var cname = name + "="; 
		var dc = "";
		try {
			dc = oHttp.getResponseHeader("Set-Cookie");
		} catch(e) {}
		if (dc.length > 0) {
			var begin = dc.indexOf(cname);
			if (begin != -1) {
				begin += cname.length;
				var end = dc.indexOf(";", begin);
				if (end == -1) end = dc.length;
				return unescape(dc.substring(begin, end));
			}
		}
		return def;
	},
	onExamineResponse : function (oHttp) {
		var userid = false;
		try {
			userid = oHttp.getResponseHeader("Set-KBSRC");
		} catch(e) {}
		if (userid) {
			var host = oHttp.URI.host;
			var newHost = false;
			var cookieID = this.getCookie(oHttp, "UTMPUSERID", null);
			if (userid != "/") {
				if (cookieID != userid) return;
				newHost = parseInt(this.getCookie(oHttp, "WWWPARAMS", 0));
				newHost = newHost & 0x1000;
			} else {
				/*
				 * This check is necessary because squid will send "Set-KBSRC" header
				 * even when the request is HIT.
				 */
				if (cookieID === null) return;
			}
			kbsrc.hosts[host] = newHost ? (new kbsrcHost(host, userid)) : false;
			kbsrc.setStatus(host);
		}
	}
};

var kbsrc = new KBSRC();
window.addEventListener("load", function() { kbsrc.init() }, false); 
window.addEventListener("unload", function() { kbsrc.deinit() }, false); 
