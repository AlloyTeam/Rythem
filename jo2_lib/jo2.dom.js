;(function(){

	// Helper Methods //////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * convert string '??px' into a number
	 * @param value string with a 'px' suffix
	 * @return number value
	 */
	function pixel2number(value){
		return Number(value.substr(0, value.length - 2));
	}

	/**
	 * get computed value of the specify style
	 * @param el target element
	 * @param name style name
	 * @param pseudoElt
	 */
	function getStyle(el, name, pseudoElt){
		return window.getComputedStyle(el, pseudoElt).getPropertyValue(name);
	}

	/**
	 * set specify style to target element
	 * @param el target element
	 * @param name style name
	 * @param value style value
	 */
	function setStyle(el, name, value){
		el.style[name] = value;
	}

	/**
	 * accumulate specify attr value from all offsetParent of specify target element
	 * @param el target element
	 * @return {Object} {left, top}
	 */
	function tracebackToRoot(el){
		//TODO how about -webkit-transition & -webkit-transform?
		var left=0, top=0;
		while(el && el!=document){
			left += el.offsetLeft;
			top += el.offsetTop;
			el = el.offsetParent;
		}
		return {left:left, top:top};
	}

	// Extend Element Prototype ////////////////////////////////////////////////////////////////////////////////////////
	
	//attribute
	Element.prototype.getData = function(name){
		return this.getAttribute('data-' + name);
	}

	//padding
    Element.prototype.__defineGetter__('paddingLeft', function(){
		return pixel2number(getStyle(this, 'padding-left'));
	});
	Element.prototype.__defineGetter__('paddingRight', function(){
		return pixel2number(getStyle(this, 'padding-right'));
	});
	Element.prototype.__defineGetter__('paddingTop', function(){
		return pixel2number(getStyle(this, 'padding-top'));
	});
	Element.prototype.__defineGetter__('paddingBottom', function(){
		return pixel2number(getStyle(this, 'padding-bottom'));
	});
	Element.prototype.__defineGetter__('hPadding', function(){
		return this.paddingLeft + this.paddingRight;
	});
	Element.prototype.__defineGetter__('vPadding', function(){
		return this.paddingTop + this.paddingBottom;
	});
	Element.prototype.__defineSetter__('paddingLeft', function(value){
		setStyle(this, 'padding-left', value + 'px');
	});
	Element.prototype.__defineSetter__('paddingRight', function(value){
		setStyle(this, 'padding-right', value + 'px');
	});
	Element.prototype.__defineSetter__('paddingTop', function(value){
		setStyle(this, 'padding-top', value + 'px');
	});
	Element.prototype.__defineSetter__('paddingBottom', function(value){
		setStyle(this, 'padding-bottom', value + 'px');
	});
	Element.prototype.__defineSetter__('padding', function(value){
		this.paddingLeft = this.paddingRight = this.paddingTop = this.paddingBottom = value;
	});
	Element.prototype.__defineSetter__('hPadding', function(value){
		this.paddingLeft = this.paddingRight = value/2;
	});
	Element.prototype.__defineSetter__('vPadding', function(value){
		this.paddingTop = this.paddingBottom = value/2;
	});

	//margin
	Element.prototype.__defineGetter__('marginLeft', function(){
		return pixel2number(getStyle(this, 'margin-left'));
	});
	Element.prototype.__defineGetter__('marginRight', function(){
		return pixel2number(getStyle(this, 'margin-right'));
	});
	Element.prototype.__defineGetter__('marginTop', function(){
		return pixel2number(getStyle(this, 'margin-top'));
	});
	Element.prototype.__defineGetter__('marginBottom', function(){
		return pixel2number(getStyle(this, 'margin-bottom'));
	});
	Element.prototype.__defineGetter__('hMargin', function(){
		return this.marginLeft + this.marginRight;
	});
	Element.prototype.__defineGetter__('vMargin', function(){
		return this.marginTop + this.marginBottom;
	});
	Element.prototype.__defineSetter__('marginLeft', function(value){
		setStyle(this, 'margin-left', value + 'px');
	});
	Element.prototype.__defineSetter__('marginRight', function(value){
		setStyle(this, 'margin-right', value + 'px');
	});
	Element.prototype.__defineSetter__('marginTop', function(value){
		setStyle(this, 'margin-top', value + 'px');
	});
	Element.prototype.__defineSetter__('marginBottom', function(value){
		setStyle(this, 'margin-bottom', value + 'px');
	});
	Element.prototype.__defineSetter__('margin', function(value){
		this.marginLeft = this.marginRight = this.marginTop = this.marginBottom = value;
	});
	Element.prototype.__defineSetter__('hMargin', function(value){
		this.marginLeft = this.marginRight = value/2;
	});
	Element.prototype.__defineSetter__('vMargin', function(value){
		this.marginTop = this.marginBottom = value/2;
	});

	//border
	Element.prototype.__defineGetter__('borderLeftWidth', function(){
		return pixel2number(getStyle(this, 'border-left-width'));
	});
	Element.prototype.__defineGetter__('borderRightWidth', function(){
		return pixel2number(getStyle(this, 'border-right-width'));
	});
	Element.prototype.__defineGetter__('borderTopWidth', function(){
		return pixel2number(getStyle(this, 'border-top-width'));
	});
	Element.prototype.__defineGetter__('borderBottomWidth', function(){
		return pixel2number(getStyle(this, 'border-bottom-width'));
	});
	Element.prototype.__defineGetter__('hBorderWidth', function(){
		return this.borderLeftWidth + this.borderRightWidth;
	});
	Element.prototype.__defineGetter__('vBorderWidth', function(){
		return this.borderTopWidth + this.borderBottomWidth;
	});
	Element.prototype.__defineSetter__('borderLeftWidth', function(value){
		setStyle(this, 'border-left-width', value + 'px');
	});
	Element.prototype.__defineSetter__('borderRightWidth', function(value){
		setStyle(this, 'border-right-width', value + 'px');
	});
	Element.prototype.__defineSetter__('borderTopWidth', function(value){
		setStyle(this, 'border-top-width', value + 'px');
	});
	Element.prototype.__defineSetter__('borderBottomWidth', function(value){
		setStyle(this, 'border-bottom-width', value + 'px');
	});
	Element.prototype.__defineSetter__('borderWidth', function(value){
		this.borderLeftWidth = this.borderRightWidth = this.borderTopWidth = this.borderBottomWidth = value;
	});
	Element.prototype.__defineSetter__('hBorderWidth', function(value){
		this.borderLeftWidth = this.borderRightWidth = value/2;
	});
	Element.prototype.__defineSetter__('vBorderWidth', function(value){
		this.borderTopWidth = this.borderBottomWidth = value/2;
	});

	//size
	Element.prototype.__defineGetter__('innerWidth', function(){
		return pixel2number(getStyle(this, 'width'));
	});
	Element.prototype.__defineGetter__('innerHeight', function(){
		return pixel2number(getStyle(this, 'height'));
	});
	Element.prototype.__defineGetter__('width', function(){
		return this.offsetWidth;
	});
	Element.prototype.__defineGetter__('height', function(){
		return this.offsetHeight;
	});
	Element.prototype.__defineGetter__('outerWidth', function(){
		return this.width + this.hMargin;
	});
	Element.prototype.__defineGetter__('outerHeight', function(){
		return this.height + this.vMargin;
	});
	Element.prototype.__defineSetter__('innerWidth', function(value){
		setStyle(this, 'width', value + 'px');
	});
	Element.prototype.__defineSetter__('innerHeight', function(value){
		setStyle(this, 'height', value + 'px');
	});
	Element.prototype.__defineSetter__('width', function(value){
		setStyle(this, 'width', (value - this.hPadding - this.hBorderWidth) + 'px');
	});
	Element.prototype.__defineSetter__('height', function(value){
		setStyle(this, 'height', (value - this.vPadding - this.vBorderWidth) + 'px');
	});
	Element.prototype.__defineSetter__('outerWidth', function(value){
		setStyle(this, 'width', (value - this.hPadding - this.hBorderWidth - this.hMargin) + 'px');
	});
	Element.prototype.__defineSetter__('outerHeight', function(value){
		setStyle(this, 'height', (value - this.vPadding - this.vBorderWidth - this.vMargin) + 'px');
	});

	//position
	Element.prototype.__defineGetter__('left', function(){
		return this.offsetLeft;
	});
	Element.prototype.__defineGetter__('top', function(){
		return this.offsetTop;
	});
	Element.prototype.__defineGetter__('absoluteLeft', function(){
		return tracebackToRoot(this).left;
	});
	Element.prototype.__defineGetter__('absoluteTop', function(){
		return tracebackToRoot(this).top;
	});
	Element.prototype.__defineSetter__('left', function(value){
		setStyle(this, 'left', (value - this.marginLeft) + 'px');
	});
	Element.prototype.__defineSetter__('top', function(value){
		setStyle(this, 'top', (value - this.marginTop) + 'px');
	});

	//content
	Element.prototype.__defineGetter__('html', function(){
		return this.innerHTML;
	});
	Element.prototype.__defineGetter__('text', function(){
		return this.textContent;
	});
	Element.prototype.__defineSetter__('html', function(value){
		this.innerHTML = value;
	});
	Element.prototype.__defineSetter__('text', function(value){
		this.textContent = value;
	});

	/**
	 * event delegate
	 * @param selector css selector
	 * @param type
	 * @param listener
	 * @param useCapture
	 * @param live {Boolean} update selected element collection on the fly ?
	 */
	Element.prototype.delegate = function(selector, type, listener, useCapture, live){
		/**
		 * _delegators = {
		 * 		'click':[
		 * 			{
		 * 				fn:listener,
		 * 				live:true/false,
		 * 				selector:selector,
		 * 				nodes:nodelist
		 * 			},
		 * 			{
		 * 				...
		 * 			},
		 * 			...
		 * 		],
		 * 		'clickCapture':[
		 * 			...
		 * 		],
		 * 		...
		 * }
		 * _liveNodeLists = {
		 * 		'selector1':nodelist1,
		 * 		'selector2':nodelist2,
		 * 		...
		 * }
		 */
		if(!this._delegators){
			this._delegators = {};
		}
		var eventName = useCapture ? type + 'Capture' : type;
		if(!this._delegators[eventName]){
			this._delegators[eventName] = [];
			this.addEventListener(type, this._delegateEventHandler, useCapture);
		}
		if(live){
			if(!this._liveNodeLists){
				this._liveNodeLists = {};
				this.addEventListener('DOMNodeInserted', this.onDOMNodeInserted);
				this.addEventListener('DOMNodeRemoved', this.onDOMNodeRemoved);
			}
			if(!this._liveNodeLists[selector]){
				this._liveNodeLists[selector] = null;
				this._updateLiveNodeLists();
			}
		}
		this._delegators[eventName].push({
			fn: listener,
			live: live,
			selector: selector,
			nodes: live ? null : this.querySelectorAll(selector)
		});
	}
	//update live nodelists when new node is inserted
	Element.prototype.onDOMNodeInserted = function(e){
		this._updateLiveNodeLists();
	}
	//update live nodelists when nodes are removed
	Element.prototype.onDOMNodeRemoved = function(e){
		var scope = this;
		setTimeout(function(){
			scope._updateLiveNodeLists();
		}, 0);
		/**
		 * Note: why we need setTimeout here ?
		 * because, according to w3c's standard, DOMNodeRemoved event is fired *BEFORE* the node is actually being removed,
		 * and I found no event which would be fire after the node is removed, so have to use this stupid way :(
		 * @see http://www.w3.org/TR/DOM-Level-2-Events/events.html#Events-MutationEvent
		 */
	}
	//update live nodelists
	Element.prototype._updateLiveNodeLists = function(){
		for(var selector in this._liveNodeLists){
			this._liveNodeLists[selector] = this.querySelectorAll(selector);
		}
	}
	//handle delegate events
	Element.prototype._delegateEventHandler = function(e){
		var listeners,
			bubbleListener = this._delegators[e.type] || [],
			captureListener = this._delegators[e.type + 'Capture'] || [];
		switch(e.eventPhase){
			case 1: listeners = captureListener; break; 						//CAPTURE PHASE
			case 2: listeners = captureListener.concat(bubbleListener); break; 	//AT_TARGET PHASE
			case 3: listeners = bubbleListener; break; 							//BUBBLE PHASE
		}
		var el = e.target, i, ln = listeners.length;
		while(el && el != this){
			for(i=0; i<ln; i++){
				var item = listeners[i];
				var nodes = item.live ? this._liveNodeLists[item.selector] : item.nodes;
				if(nodes.indexOf(el) != -1){
					item.fn.apply(this, [e, el]);
				}
			}
			el = el.parentNode;
		}
	}

    // Extend NodeList Prototype ///////////////////////////////////////////////////////////////////////////////////////

	/**
	 * locate specify element in a nodelist
	 * @param element
	 * @return index of the element in this nodelist, -1 if not found
	 */
    NodeList.prototype.indexOf = function(element){
		for(var i=0, item; item = this.item(i); i++){
            if(item == element) return i;
        }
        return -1;
    }

})();