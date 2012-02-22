;(function(){

	// Helper Methods //////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * convert string '??px' into a number
	 * @param value string with a 'px' suffix
	 * @return number value
	 */
	function pixel2number(value){
		return Number(value.substr(0, value.length - 2));
	};

	/**
	 * round number x to specify accuracy
	 * @param x
	 * @param precision
	 */
	function roundTo(x, precision){
		var p = Math.pow(10, precision || 6);
		return Math.round(x * p)/p;
	};

	/**
	 * bubble up from element el to the document treetop, invoke callback method for every node on the path
	 * @param el starting point
	 * @param callback callback method to invoke on every node on the path, expect one argument: the node element
	 * @param offsetParentOnly true:offsetParent, false:parentNode
	 */
	function bubbleUp(el, callback, offsetParentOnly){
		var parentName = offsetParentOnly ? 'offsetParent' : 'parentNode';
		while(el && el!=document){
			callback(el);
			el = el[parentName];
		}
	};
	/**
	 * bubble up from element el to the document treetop and cumulate values return from the callback method
	 * @param el starting point
	 * @param callback callback method to invoke on every node on the path, expect one argument: the node element, and should return a number value
	 * @param offsetParentOnly
	 */
	function bubbleUpAndCumulate(el, callback, offsetParentOnly){
		var result = 0;
		bubbleUp(el, function(el){
			result += callback(el);
		}, offsetParentOnly);
		return result;
	};
	function bubbleUpAndMultiple(el, callback, offsetParentOnly){
		var result = 1;
		bubbleUp(el, function(el){
			result *= callback(el);
		}, offsetParentOnly);
		return result;
	};

	// Extend Element Prototype ////////////////////////////////////////////////////////////////////////////////////////

	Element.prototype.getData = function(name){
		return this.getAttribute('data-' + name);
	};
	Element.prototype.setData = function(name, value){
		this.setAttribute('data-' + name, value);
	};
	Element.prototype.hasData = function(name){
		return this.hasAttribute('data-' + name);
	};
	Element.prototype.getStyle = function(name, pseudoElt){
		return window.getComputedStyle(this, pseudoElt).getPropertyValue(name);
	};
	Element.prototype.setStyle = function(name, value){
		this.style[name] = value;
	};

	//padding
	Element.prototype.__defineGetter__('paddingLeft', function(){
		return pixel2number(this.getStyle('padding-left'));
	});
	Element.prototype.__defineGetter__('paddingRight', function(){
		return pixel2number(this.getStyle('padding-right'));
	});
	Element.prototype.__defineGetter__('paddingTop', function(){
		return pixel2number(this.getStyle('padding-top'));
	});
	Element.prototype.__defineGetter__('paddingBottom', function(){
		return pixel2number(this.getStyle('padding-bottom'));
	});
	Element.prototype.__defineGetter__('hPadding', function(){
		return this.paddingLeft + this.paddingRight;
	});
	Element.prototype.__defineGetter__('vPadding', function(){
		return this.paddingTop + this.paddingBottom;
	});
	Element.prototype.__defineSetter__('paddingLeft', function(value){
		this.setStyle('padding-left', value + 'px');
	});
	Element.prototype.__defineSetter__('paddingRight', function(value){
		this.setStyle('padding-right', value + 'px');
	});
	Element.prototype.__defineSetter__('paddingTop', function(value){
		this.setStyle('padding-top', value + 'px');
	});
	Element.prototype.__defineSetter__('paddingBottom', function(value){
		this.setStyle('padding-bottom', value + 'px');
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
		return pixel2number(this.getStyle('margin-left'));
	});
	Element.prototype.__defineGetter__('marginRight', function(){
		return pixel2number(this.getStyle('margin-right'));
	});
	Element.prototype.__defineGetter__('marginTop', function(){
		return pixel2number(this.getStyle('margin-top'));
	});
	Element.prototype.__defineGetter__('marginBottom', function(){
		return pixel2number(this.getStyle('margin-bottom'));
	});
	Element.prototype.__defineGetter__('hMargin', function(){
		return this.marginLeft + this.marginRight;
	});
	Element.prototype.__defineGetter__('vMargin', function(){
		return this.marginTop + this.marginBottom;
	});
	Element.prototype.__defineSetter__('marginLeft', function(value){
		this.setStyle('margin-left', value + 'px');
	});
	Element.prototype.__defineSetter__('marginRight', function(value){
		this.setStyle('margin-right', value + 'px');
	});
	Element.prototype.__defineSetter__('marginTop', function(value){
		this.setStyle('margin-top', value + 'px');
	});
	Element.prototype.__defineSetter__('marginBottom', function(value){
		this.setStyle('margin-bottom', value + 'px');
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
		return pixel2number(this.getStyle('border-left-width'));
	});
	Element.prototype.__defineGetter__('borderRightWidth', function(){
		return pixel2number(this.getStyle('border-right-width'));
	});
	Element.prototype.__defineGetter__('borderTopWidth', function(){
		return pixel2number(this.getStyle('border-top-width'));
	});
	Element.prototype.__defineGetter__('borderBottomWidth', function(){
		return pixel2number(this.getStyle('border-bottom-width'));
	});
	Element.prototype.__defineGetter__('hBorderWidth', function(){
		return this.borderLeftWidth + this.borderRightWidth;
	});
	Element.prototype.__defineGetter__('vBorderWidth', function(){
		return this.borderTopWidth + this.borderBottomWidth;
	});
	Element.prototype.__defineSetter__('borderLeftWidth', function(value){
		this.setStyle('border-left-width', value + 'px');
	});
	Element.prototype.__defineSetter__('borderRightWidth', function(value){
		this.setStyle('border-right-width', value + 'px');
	});
	Element.prototype.__defineSetter__('borderTopWidth', function(value){
		this.setStyle('border-top-width', value + 'px');
	});
	Element.prototype.__defineSetter__('borderBottomWidth', function(value){
		this.setStyle('border-bottom-width', value + 'px');
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
		return pixel2number(this.getStyle('width'));
	});
	Element.prototype.__defineGetter__('innerHeight', function(){
		return pixel2number(this.getStyle('height'));
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
		this.setStyle('width', value + 'px');
	});
	Element.prototype.__defineSetter__('innerHeight', function(value){
		this.setStyle('height', value + 'px');
	});
	Element.prototype.__defineSetter__('width', function(value){
		this.setStyle('width', (value - this.hPadding - this.hBorderWidth) + 'px');
	});
	Element.prototype.__defineSetter__('height', function(value){
		this.setStyle('height', (value - this.vPadding - this.vBorderWidth) + 'px');
	});
	Element.prototype.__defineSetter__('outerWidth', function(value){
		this.setStyle('width', (value - this.hPadding - this.hBorderWidth - this.hMargin) + 'px');
	});
	Element.prototype.__defineSetter__('outerHeight', function(value){
		this.setStyle('height', (value - this.vPadding - this.vBorderWidth - this.vMargin) + 'px');
	});

	//position
	Element.prototype.__defineGetter__('left', function(){
		return this.offsetLeft;
	});
	Element.prototype.__defineGetter__('top', function(){
		return this.offsetTop;
	});
	Element.prototype.__defineGetter__('absoluteLeft', function(){
		return bubbleUpAndCumulate(this, function(el){
			return el.offsetLeft;
		}, true);
	});
	Element.prototype.__defineGetter__('absoluteTop', function(){
		return bubbleUpAndCumulate(this, function(el){
			return el.offsetTop;
		}, true);
	});
	Element.prototype.__defineSetter__('left', function(value){
		this.setStyle('left', (value - this.marginLeft) + 'px');
	});
	Element.prototype.__defineSetter__('top', function(value){
		this.setStyle('top', (value - this.marginTop) + 'px');
	});

	//2D transforms
	Element.prototype.get2DTransformMatrix = function(){
		var transform = this.getStyle('-webkit-transform');
		if(transform.indexOf('matrix(') != -1){
			var data = transform.substring(7, transform.length - 1).split(', ');
			return {
				a: parseFloat(data[0]),
				b: parseFloat(data[1]),
				c: parseFloat(data[2]),
				d: parseFloat(data[3]),
				tx: parseFloat(data[4]),
				ty: parseFloat(data[5])
			};
		}
		else{
			return {a:1, b:0, c:0, d:1, tx:0, ty:0};
		}
	};
	Element.prototype.get2DTransform = function(precision){
		var matrix = this.get2DTransformMatrix();
		var radian = -Math.atan(matrix.c / matrix.a);
		var scale = matrix.a / Math.cos(radian);
		return {
			tx: matrix.tx,
			ty: matrix.ty,
			rotate: roundTo(radian * 180 / Math.PI, precision || 6),
			scale: roundTo(scale, precision || 6)
		};
	};
	Element.prototype.set2DTransform = function(tx, ty, rotate, scale){
		this._2dTranslateX = tx || this._2dTranslateX || 0;
		this._2dTranslateY = ty || this._2dTranslateY || 0;
		this._2dRotate = rotate || this._2dRotate || 0;
		this._2dScale = scale || this._2dScale || 1;
		this.updateCSSTransform();
	};
	Element.prototype.updateCSSTransform = function(){
		var translate2d = 'translate(' + (this._2dTranslateX || 0) + 'px, ' + (this._2dTranslateY || 0) + 'px)';
		var rotate2d = 'rotate(' + (this._2dRotate || 0) + 'deg)';
		var scale2d = 'scale(' + (this._2dScale || 1) + ')';
		this.setStyle('-webkit-transform', [translate2d, rotate2d, scale2d].join(' '));
	};
	Element.prototype.__defineGetter__('translateX', function(){
		return this.get2DTransformMatrix().tx;
	});
	Element.prototype.__defineGetter__('translateY', function(){
		return this.get2DTransformMatrix().ty;
	});
	Element.prototype.__defineGetter__('rotate', function(){
		return this.get2DTransform().rotate;
	});
	Element.prototype.__defineGetter__('scale', function(){
		return this.get2DTransform().scale;
	});
	Element.prototype.__defineGetter__('absoluteTranslateX', function(){
		return bubbleUpAndCumulate(this, function(el){
			return el.offsetLeft + el.translateX;
		});
	});
	Element.prototype.__defineGetter__('absoluteTranslateY', function(){
		return bubbleUpAndCumulate(this, function(el){
			return el.offsetTop + el.translateY;
		});
	});
	Element.prototype.__defineGetter__('absoluteRotate', function(){
		return bubbleUpAndCumulate(this, function(el){
			return el.rotate;
		});
	});
	Element.prototype.__defineGetter__('absoluteScale', function(){
		return bubbleUpAndMultiple(this, function(el){
			return el.scale;
		});
	});
	Element.prototype.__defineSetter__('translateX', function(value){
		this._2dTranslateX = value;
		this.updateCSSTransform();
	});
	Element.prototype.__defineSetter__('translateY', function(value){
		this._2dTranslateY = value;
		this.updateCSSTransform();
	});
	Element.prototype.__defineSetter__('rotate', function(value){
		this._2dRotate = value;
		this.updateCSSTransform();
	});
	Element.prototype.__defineSetter__('scale', function(value){
		this._2dScale = value;
		this.updateCSSTransform();
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
	 * @param [useCapture=false]
	 * @param [live=false] update selected element collection on the fly ?
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
	};
	//update live nodelists when new node is inserted
	Element.prototype.onDOMNodeInserted = function(e){
		this._updateLiveNodeLists();
	};
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
	};
	//update live nodelists
	Element.prototype._updateLiveNodeLists = function(){
		for(var selector in this._liveNodeLists){
			this._liveNodeLists[selector] = this.querySelectorAll(selector);
		}
	};
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
	};

	Text.prototype.serialize = function(){
		return {};
	};
	Text.prototype.serializeToString = function(){
		return this.textContent;
	};

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
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//simply dom selection
	$ = function(){     return document.getElementById.apply(document, arguments);      };
	$$ = function(){    return document.querySelector.apply(document, arguments);       };
	$$$ = function(){   return document.querySelectorAll.apply(document, arguments);    };

})();