;(function(){

	if(!window.jo2) window.jo2 = {};

	// Dom Inspector ///////////////////////////////////////////////////////////////////////////////////////////////////

	jo2.DomInspector = function(parent){
		var el = this.init(parent);
	};

	jo2.DomInspector.getDomTree = function(node){
		node = node || document.documentElement;
		var attributes = [];
		var children = [];
		var i;
		for(i=0; i<node.attributes.length; i++){
			var a = node.attributes[i];
			attributes.push(a.name + '="' + a.value + '"');
		}
		for(i=0; i<node.childElementCount; i++){
			children.push(jo2.DomInspector.getDomTree(node.children[i]));
		}
		return {
			node: node,
			tagName: node.nodeName.toLowerCase(),
			attributes: attributes,
			children: children
		};
	};

	jo2.DomInspector.prototype = {
		init: function(parent){

		}
	};

	// Console /////////////////////////////////////////////////////////////////////////////////////////////////////////

	jo2.Console = function(parent){
		var el = this.init(parent);
	};

	jo2.Console.prototype = {
		init: function(parent){

		}
	};
	
	// Inspector ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * create a new console instance, and insert it into the parent node
     * @param {Element} parent parent node of the console element, if omit, the console would be insert into the body tag
     * @param {String} activateTab default is console
     */
	jo2.Inspector = function(parent, activateTab){
		//init the dom element for the console
		var el = this.init(parent);
		this.el = el;
        this.domInspectorPanel = el.querySelector('.panel.element');
        this.consolePanel = el.querySelector('.panel.console');

		this.domInspector = new jo2.DomInspector(this.domInspectorPanel);
		this.console = new jo2.Console(this.consolePanel);

		activateTab = activateTab || 'console';
		this.activateTab(activateTab);
		this.currentTab = activateTab;
	};

	jo2.Inspector.prototype = {
		init: function(parent){
			parent = parent || document.querySelector('body');
			var el = document.createElement('article');
			el.className = 'jo2console vgroup';
			el.innerHTML = '\
				<header class="hgroup">\
					<div class="controls"></div>\
					<nav class="tabs hgroup">\
						<li data-role="tab" data-name="element" class="tab">Elements</li>\
						<li data-role="tab" data-name="console" class="tab">Console</li>\
					</nav>\
					<div class="search"></div>\
				</header>\
				<div class="panels">\
					<div data-role="panel" data-name="element" class="panel element"></div>\
					<div data-role="panel" data-name="console" class="panel console"></div>\
				</div>\
				<footer>\
				</footer>\
			';
			el.__instance__ = this;
			el.delegate('.tab', 'click', this.onTabClick);
			parent.appendChild(el);
			return el;
		},
		show: function(){
		
		},
		hide: function(){
		
		},
		restore: function(){
		
		},
		min: function(){
		
		},
		info: function(){

		},
		log: function(){

		},
		warn: function(){

		},
		error: function(){

		},
		activateTab: function(name){
			if(name != this.currentTab){
				//update tab and panel status
				updateTabStatus(this, name);
				updatePanelStatus(this, name);
				this.currentTab = name;
			}
		},
		onTabClick: function(e, el){
			var instance = this.__instance__;
			instance.activateTab(el.getData('name'));
		}
	};
	
	// Private Helper Methods //////////////////////////////////////////////////////////////////////////////////////////

    /**
     * update tab status
     * @param {Console} instance
     * @param {String} name
     */
	function updateTabStatus(instance, name){
		var currentTab = instance.el.querySelector('.tabs .tab.activate');
		var tab = instance.el.querySelector('.tabs .tab[data-name="' + name + '"]');
		if(currentTab){
			currentTab.classList.remove('activate');
		}
		if(tab){
			tab.classList.add('activate');
		}
	}

    /**
     * update panel status
     * @param {Console} instance
     * @param {String} name
     */
	function updatePanelStatus(instance, name){
		var currentPanel = instance.el.querySelector('.panels .panel.activate');
		var panel = instance.el.querySelector('.panels .panel[data-name="' + name + '"]');
		if(currentPanel){
			currentPanel.classList.remove('activate');
		}
		if(panel){
			panel.classList.add('activate');
		}
	}

})();