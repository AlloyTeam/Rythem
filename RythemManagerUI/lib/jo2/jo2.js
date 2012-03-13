;(function(){
	if(!window.jo2) window.jo2 = {};

	/**
	 * create namespace object
	 * @param {String} namespace a string with format like: xx.xxx.xx
	 * @return {Object} the namespace object
	 */
	jo2.ns = function(namespace){
		var items = namespace.split('.');
		for(var i=0, target=window; i<items.length; i++){
			if(!target[items[i]]){
				target[items[i]] = {};
			}
			target = target[items[i]];
		}
		return target;
	};

	/**
	 * print current callstack
	 * @return {Array} callstack
	 */
	jo2.getCallStack = function(){
		var stack = new Error().stack.split('\n');
		var items = [];
		//first line is "Error"
		//second line is "Object.getStack(here)"
		//we need to ignore the first two lines here
		for(var i=2; i<stack.length; i++){
			var item = stack[i].match(/^\s*at\s*(.*)$/);
			if(item && item[1].length){
				items.push(item[1]);
			}
		}
		return items;
	};

	/**
	 * create a Class
	 * how this works ?
	 * 1. xxx = function(){ ... }
	 * 2. xxx.prototype = { ... }
	 * 3. xxx.prototype.__proto__ = SuperClass.prototype
	 * @param {Object} members public members of the class, member 'initialize' would become the constructor
	 * @param {Function} Superclass optional superclass to inherit from
	 * @return {Function} class object
	 */
	jo2.extend = function(members, Superclass){
		//initialize the constructor
		var constructor = function(){
			var fn = members.initialize || Superclass || new Function();
			fn.apply(this, arguments);
		};

		//assign class members to prototype
		constructor.prototype = members;

		//connect the class to its super class
		if(Superclass){
			constructor.prototype.__proto__ = Superclass.prototype;
			constructor.prototype.__defineGetter__('superclass', function(){
				return this.__proto__.__proto__;
			});
		}

		return constructor;
	};

})();