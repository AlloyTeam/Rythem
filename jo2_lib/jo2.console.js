;(function(){
	
	// Console /////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * create a new console instance, and insert it into the parent node
     * @param {Element} parent parent node of the console element, if omit, the console would be insert into the body tag
     */
	var Console = function(parent){
		//init the dom element for the console
		parent = parent || document.querySelector('body');
		var el = document.createElement('article');
		el.className = 'jo2console vgroup';
		el.innerHTML = '\
			<header class="hgroup">\
				<div class="controls"></div>\
				<nav class="tabs hgroup">\
					<li data-role="tab" data-name="element" class="tab activate">Elements</li>\
					<li data-role="tab" data-name="console" class="tab">Console</li>\
				</nav>\
				<div class="search"></div>\
			</header>\
			<div class="panels">\
				<div data-role="panel" data-name="element" class="panel element">element</div>\
				<div data-role="panel" data-name="console" class="panel console activate">\
					<div class="history"></div>\
					<div class="input" contentEditable></div>\
				</div>\
			</div>\
			<footer>\
			</footer>\
		';
		el.__instance__ = this;
		el.delegate('.tab', 'click', onTabClick);
		parent.appendChild(el);
		
		this.el = el;
        this.elementPanel = el.querySelector('.panel.element');
        this.consolePanel = el.querySelector('.panel.console');
		this.consoleInput = el.querySelector('.panel.console .input');
		this.currentTab = 'console';
	}

	Console.prototype = {
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
				//do some other stuff
				switch(name){
					case 'element': this.activateElementTab(); break;
					case 'console': this.activateConsoleTab(); break;
				}
			}
		},
		activateElementTab: function(){
		
		},
		activateConsoleTab: function(){
			this.consoleInput.focus();
		},
		refreshDomTree: function(node){
			node = node || window.document.documentElement;
			//document.doctype;
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
	
	// Private Event Handlers //////////////////////////////////////////////////////////////////////////////////////////

    /**
     * tab click event handler, activate the selected tab
     * @param {MouseEvent} e
     * @param {Element} el
     */
	function onTabClick(e, el){
		var instance = this.__instance__;
        instance.activateTab(el.getData('name'));
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(!window.jo2) window.jo2 = {};
	jo2.Console = Console;

})();