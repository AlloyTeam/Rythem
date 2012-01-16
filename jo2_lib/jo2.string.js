;(function(){

    String.prototype.escapeHTML = function(){
        return this.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
    };

    String.prototype.unescapeHTML = function(){
        return this.stripTags().replace(/&lt;/g,'<').replace(/&gt;/g,'>').replace(/&amp;/g,'&');
    };

})();