/****************** browser compatibility functions **********************/
/* origin for O'Reilly's "JavaScript & DHTML Cookbook" By Danny Goodman */
/* modified by roy@zixia.net */

// Global variables
var isCSS=false;
var isW3C=false;
var isIE4=false;
var isNN4=false;
var isIE6CSS=false;

// Initialize upon load to let all browsers establish content objects

if( ! document.body ) {
	alert("browers.js must be include after <body> tag!");
}


if (document.images) {
	isCSS = (document.body && document.body.style) ? true : false;
	isIE4 = (isCSS && document.all) ? true : false;
	if( isIE4 )	{
		isIE6CSS = (document.compatMode && document.compatMode.indexOf("CSS1") >= 0) ? 
			true : false;
	} else {
		isW3C = (isCSS && document.getElementById) ? true : false;
	}
}
// Convert other frame object name string or object reference
// into a valid element object reference 
function getRawObjectFrom(obj, oFrame) {
    var theObj;
    if (typeof obj == "string") {
        if (isIE4) {
            theObj = oFrame.document.all(obj);
        } else if (isW3C) {
            theObj = oFrame.document.getElementById(obj);
        } 
    } else {
        // pass through object reference
        theObj = obj;
    }
    return theObj;
}

// Convert parent object name string or object reference
// into a valid element object reference 
function getParentRawObject(obj) {
    return getRawObjectFrom(obj, parent);
}

// Convert object name string or object reference
// into a valid element object reference
function getRawObject(obj) {
    var theObj;
    if (typeof obj == "string") {
        if (isIE4) {
            theObj = document.all(obj);
        } else if (isW3C) {
            theObj = document.getElementById(obj);
        } 
    } else {
        // pass through object reference
        theObj = obj;
    }
    return theObj;
}



// Convert object id/name string or object references array
// into a valid element object references array
function getObjectCollection(obj) {
    var theObj;
    if (typeof obj == "string") {
		if (isIE4) {
			theObj=document.all.item(obj);
		} else if (isW3C) {
			theObj=document.getElementsByName(obj);
		}
    } else {
        // pass through object reference
        theObj = obj;
    }
    return theObj;
}

// Convert other frame's object id/name string or object references array
// into a valid element object references array
function getObjectCollectionFrom(obj,oFrame) {
    var theObj;
    if (typeof obj == "string") {
		if (isIE4) {
			theObj=oFrame.document.all.item(obj);
		} else if (isW3C) {
			theObj=oFrame.document.getElementsByName(obj);
		}
    } else {
        // pass through object reference
        theObj = obj;
    }
    return theObj;
}

// Convert parent object id/name string or object references array
// into a valid element object references array
function getParentObjectCollection(obj) {
    return getObjectCollectionFrom(obj,parent);
}
   
// Convert object name string or object reference
// into a valid style (or NN4 layer) reference
function getObjectStyle(obj) {
    var theObj = getRawObject(obj);
    if (theObj && isCSS) {
        theObj = theObj.style;
    }
    return theObj;
}

// Position an object at a specific pixel coordinate
function shiftTo(obj, x, y) {
    var theObj = getObjectStyle(obj);
    if (theObj) {
        if (isCSS) {
            // equalize incorrect numeric value type
            var units = (typeof theObj.left == "string") ? "px" : 0;
            theObj.left = x + units;
            theObj.top = y + units;
        } 
    }
}
   
// Move an object by x and/or y pixels
function shiftBy(obj, deltaX, deltaY) {
    var theObj = getObjectStyle(obj);
    if (theObj) {
        if (isCSS) {
            // equalize incorrect numeric value type
            var units = (typeof theObj.left == "string") ? "px" : 0;
            theObj.left = getObjectLeft(obj) + deltaX + units;
            theObj.top = getObjectTop(obj) + deltaY + units;
        } 
    }
}
   
// Set the z-order of an object
function setZIndex(obj, zOrder) {
    var theObj = getObjectStyle(obj);
    if (theObj) {
        theObj.zIndex = zOrder;
    }
}
   
// Set the background color of an object
function setBGColor(obj, color) {
    var theObj = getObjectStyle(obj);
    if (theObj) {
		if (isCSS) {
            theObj.backgroundColor = color;
        }
    }
}
   
// Set the visibility of an object to visible
function show(obj) {
    var theObj = getObjectStyle(obj);
    if (theObj) {
        theObj.visibility = "visible";
    }
}
   
// Set the visibility of an object to hidden
function hide(obj) {
    var theObj = getObjectStyle(obj);
    if (theObj) {
        theObj.visibility = "hidden";
    }
}
   
// Retrieve the x coordinate of a positionable object
function getObjectLeft(obj)  {
    var elem = getRawObject(obj);
    var result = 0;
    if (document.defaultView) {
        var style = document.defaultView;
        var cssDecl = style.getComputedStyle(elem, "");
        result = cssDecl.getPropertyValue("left");
    } else if (elem.currentStyle) {
        result = elem.currentStyle.left;
    } else if (elem.style) {
        result = elem.style.left;
    } 
    return parseInt(result);
}
   
// Retrieve the y coordinate of a positionable object
function getObjectTop(obj)  {
    var elem = getRawObject(obj);
    var result = 0;
    if (document.defaultView) {
        var style = document.defaultView;
        var cssDecl = style.getComputedStyle(elem, "");
        result = cssDecl.getPropertyValue("top");
    } else if (elem.currentStyle) {
        result = elem.currentStyle.top;
    } else if (elem.style) {
        result = elem.style.top;
    } 
    return parseInt(result);
}
   
// Retrieve the rendered width of an element
function getObjectWidth(obj)  {
    var elem = getRawObject(obj);
    var result = 0;
    if (elem.offsetWidth) {
        result = elem.offsetWidth;
    } else if (elem.clip && elem.clip.width) {
        result = elem.clip.width;
    } else if (elem.style && elem.style.pixelWidth) {
        result = elem.style.pixelWidth;
    }
    return parseInt(result);
}
   
// Retrieve the rendered height of an element
function getObjectHeight(obj)  {
    var elem = getRawObject(obj);
    var result = 0;
    if (elem.offsetHeight) {
        result = elem.offsetHeight;
    } else if (elem.clip && elem.clip.height) {
        result = elem.clip.height;
    } else if (elem.style && elem.style.pixelHeight) {
        result = elem.style.pixelHeight;
    }
    return parseInt(result);
}
   
// Return the available content width space in browser window
function getInsideWindowWidth( ) {
    if (window.innerWidth) {
        return window.innerWidth;
    } else if (isIE6CSS) {
        // measure the html element's clientWidth
        return document.body.parentElement.clientWidth;
    } else if (document.body && document.body.clientWidth) {
        return document.body.clientWidth;
    }
    return 0;
}
   
// Return the available content height space in browser window
function getInsideWindowHeight( ) {
    if (window.innerHeight) {
        return window.innerHeight;
    } else if (isIE6CSS) {
        // measure the html element's clientHeight
        return document.body.parentElement.clientHeight;
    } else if (document.body && document.body.clientHeight) {
        return document.body.clientHeight;
    }
    return 0;
}
