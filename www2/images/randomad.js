		var browserName = navigator.appName;
		var agt=navigator.platform.toLowerCase();
		var ie5 = (browserName == "Microsoft Internet Explorer") && navigator.appVersion.indexOf('MSIE 5')>0;
		var ie6 = (browserName == "Microsoft Internet Explorer") && navigator.appVersion.indexOf('MSIE 6')>0;
		var ns6 = (navigator.vendor == "Netscape6");
		var ns7 = (navigator.userAgent.indexOf("Netscape/7")>=0);
		var mac  = (agt.indexOf('mac') != -1);
		var images_num = 4;
		var delay = 3000;
		var suffix = ".gif";
		var image_root = "images/ad/";
		var bottomImageName = "bottom";
		var userAgent = window.navigator.userAgent;
		var browserVersion = parseInt(userAgent.charAt(userAgent.indexOf("/")+1),10);
		
		//test la version du browser et la platform afin d'activer ou non les transitions
		function enableEffect(){
			if(!mac && ie5 || ie6 || ns7){
				return true;
			}
			else{
				return false;			
			}
		}

		//return true si le browser est une des versions d'ie
		function isExplorer(){
			return userAgent.indexOf("MSIE") > 0;
		}
		
		function getElement(name,layer){
			if (browserVersion < 4) return document[name];
			var curDoc = layer ? layer.document : document;
			var elem = curDoc[name];
			if (!elem)
			{
				for (var i=0;i<curDoc.layers.length;i++)
				{
					elem = getElement(name,curDoc.layers[i]);
					if (elem) return elem;
				}
			}
			return elem;
		}
		
		function getImage(name){
			return (!isExplorer()) ? getElement(name,0) : document[name];
		}

		//Instancié par notifyLoad et declenche soit l'action de swap image soit le fadeOut
		function loadImage(){
			if(enableEffect()){
				action = setInterval('fadeOut()',1);
			}
			else{
				var which = Math.floor(Math.random()*images_num);
				var bottomImage = getImage(bottomImageName);
				bottomImage.src = image_root+which+suffix;						
			}						
		}
		
		//Instancié par l'evenement onLoad de l'image name="bottom"
		function notifyLoad(source){
			if(enableEffect()){
				action = setInterval('fadeIn()',1);			
			}
			else if(ns6){			
				setInterval("loadImage()",4000);
			}
			else{
				setTimeout("loadImage()",delay);			
			}
		}
		
		var count = 100;
		//FadeOut avant le notifyLoad de l'image
		function fadeOut(){
			if(count > 10){
				count=count-10;
				document.getElementById('bottom').style.filter = 'Alpha(opacity='+count+')';
			}
			else if(count == 10){
				clearInterval(action);
				var which = Math.floor(Math.random()*images_num);
				var bottomImage = getImage(bottomImageName);
				bottomImage.src = image_root+which+suffix;			
			}
		}
		
		//FadeIn avant le loadImage de l'image
		function fadeIn(){
			if(count < 100){
				count=count+10;
				document.getElementById('bottom').style.filter = 'Alpha(opacity='+count+')';
			}
			else if(count == 100){
				clearInterval(action);			
				setTimeout("loadImage()",delay);
			}
		}
