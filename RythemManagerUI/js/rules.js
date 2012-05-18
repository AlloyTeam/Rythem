"use strict";
;(function(){
function escapeToHtml(str){
    return str.replace(/&/g,"&amp;").replace(/\s/g,"&nbsp;").replace(/\>/g,"&gt;").replace(/</g,"/&lt;");
}
function unescapeFromHtml(html){
 var s = html.replace(/&amp;/g,"&").replace(/&nbsp;/," ").replace(/&gt;/g,">").replace(/&lt;/,"<");
 return s;
}
var oldConfigs;
function updateConfigs(){
    //已废弃
}
    /**
     * 项目里的每条规则，支持编辑和启用/禁用，会自动同步相关的数据
     * @param {Object} ruleConfig
     */
    function Rule(ruleConfig){
        var me = this;

        //create the rule element
        var el = document.createElement('div');
        el.className = 'rule hgroup';
        el.innerHTML =
            '<input class="ruleEnableCB" type="checkbox" checked>\
            <select>\
                <option value="1" disabled>Complex Host</option>\
                <option value="2">Simple Host</option>\
                <option value="3" disabled>Remote Content</option>\
                <option value="4">Local File</option>\
                <option value="5">Local File Merge</option>\
                <option value="6">Local Directory</option>\
            </select>\
            <div class="pattern editable" contentEditable>' + escapeToHtml(ruleConfig.rule.pattern) + '</div>\
            <div class="replace editable" contentEditable>' + escapeToHtml(ruleConfig.rule.replace) + '</div>\
			<div class="button selectFile hidden">-F</div>\
			<div class="button selectDir hidden">-D</div>\
            <div class="button remove">x</div> \
        ';

        //selected the rule type
        var select = el.querySelector('select');
        var option = el.querySelector('option[value="' + ruleConfig.type + '"]');
        if(option) option.setAttribute('selected', 'selected');

        var checkbox = el.querySelector('input');
        var patternField = el.querySelector('.pattern');
        var replaceField = el.querySelector('.replace');
		var selectFileField = el.querySelector(".selectFile");
		var selectDirField = el.querySelector(".selectDir");

        this.__config = ruleConfig;
        this.__el = el;
        this.__typeSelect = select;
        this.__checkbox = checkbox;
        this.__patternField = patternField;
        this.__replaceField = replaceField;
		this.__selectFileField = selectFileField;
		this.__selectDirField = selectDirField;

        this.updateButtonState(ruleConfig.type);

        select.addEventListener(	'change', 	function(e){ me.onTypeChange(e); });
        checkbox.addEventListener(	'change', 	function(e){ me.onCheckboxChange(e); });
        //el.delegate('.editable', 	'dblclick', function(e, el){ me.onFieldsDoubleClick(e, el); });
        el.delegate('.editable', 	'focus', function(e, el){ me.onFieldsFocus(e, el); },true);
        el.delegate('.editable', 	'keydown', 	function(e, el){ me.onFieldsEditing(e, el); });
        el.delegate('.editable', 	'blur', 	function(e, el){ me.onFieldsBlur(e, el); },true);
        el.delegate('.selectFile', 	'click', 	function(e, el){ me.onSelectFile(e, el);});
        el.delegate('.selectDir', 	'click', 	function(e, el){ me.onSelectDir(e, el);});
		ruleConfig.enable = (ruleConfig.enable!=0)
        this.setEnable(ruleConfig.enable);
    }
    Rule.prototype = {
        /**
         * 获取规则是否启用
         */
        getEnable: function(){
            return this.__config.enable;
        },
        /**
         * 设置规则是否启用
         * @param {Boolean} enable
         * @param {Boolean} [excludeCheckbox=false] 是否忽略掉勾选框
         */
        setEnable: function(enable, excludeCheckbox){
            this.__config.enable = enable;
            if(enable){
                this.__el.classList.remove('disabled');
            }
            else{
                this.__el.classList.add('disabled');
            }
            this.__checkbox.checked = enable;
            this.__typeSelect.disabled = !enable;
            if(!excludeCheckbox){
                this.__checkbox.disabled = !enable;
            }
        },
        setLineEnable: function(enable){
            this.__config.lineEnable = enable;
            if(enable){
                this.__el.classList.remove('disabled');
            }
            else{
                this.__el.classList.add('disabled');
            }
            this.__typeSelect.disabled = !enable;
            this.__checkbox.disabled = !enable
        },

        /**
         * 获取dom元素
         * @return {HTMLDivElement}
         */
        getEl: function(){
            return this.__el;
        },
        /**
         * 获取配置数据
         * @return {Object}
         */
        getConfig: function(){
            return this.__config;
        },
        /**
         * 获取规则类型
         * @return {int}
         */
        getType: function(){
            return this.__config.type;
        },
        /**
         * 获取规则匹配规则
         * @return {String}
         */
        getPattern: function(){
            return this.__config.rule.pattern;
        },
        /**
         * 获取规则替换值
         * @return {String}
         */
        getReplace: function(){
            return this.__config.rule.replace;
        },
        /**
         * 修改规则类型时触发
         * @param {Event} e
         */
        onTypeChange: function(e){
            var options = this.__typeSelect.options;
            var index = this.__typeSelect.selectedIndex;
			var type = Number(options[index].value);
            this.__config.type = type;
            this.updateButtonState(type);
            if(window.App){
                window.App.doAction(4,JSON.stringify(this.__config),this.__config.groupId);
            }
        },
        /**
         * 勾选/取消勾选规则时触发
         * @param {Event} e
         */
        onCheckboxChange: function(e){
            var enabled = this.__checkbox.checked;
            this.setEnable(enabled, true);
			if(window.App){
				window.App.doAction(4,JSON.stringify(this.__config),this.__config.groupId);
			}
        },
        /**
         * 双击匹配/替换值时编辑对应值
         * @param {Event} e
         * @param {Element} fieldEl
         */
        onFieldsDoubleClick: function(e, fieldEl){
            startEdit(fieldEl);
        },
		onFieldsFocus: function(e, fieldEl){
			startEdit(fieldEl);
		},
        /**
         * 按下ENTER时结束编辑并更新相关数据
         * @param {Event} e
         * @param {Element} fieldEl
         */
        onFieldsEditing: function(e, fieldEl){
            if(e.keyCode == 13){
                //prevent browser create the <br> tag and stop editing
                e.preventDefault();
                stopEdit(fieldEl);
                /*
				if(this.__patternField == e.srcElement){
					this.__config.rule.pattern = unescapeFromHtml(e.srcElement.innerHTML);
				}else if(this.__replaceField == e.srcElement){
					this.__config.rule.replace = unescapeFromHtml(e.srcElement.innerHTML);
				}
				console.info(this.__config);
				if(window.App){
					window.App.doAction(4,JSON.stringify(this.__config),this.__config.groupId);
				}
                */
            }
        },
        onFieldsBlur: function(e, fieldEl){
			
            //prevent browser create the <br> tag and stop editing
            //console.info("blur before(html):",e.srcElement.innerHTML);
            e.srcElement.innerText = e.srcElement.innerText.replace(/^\s*|\s*$|\n|\r/g,"");
            //console.info("blur afeter(text)",e.srcElement.innerText);
            if(this.__patternField == e.srcElement){
                if(this.__config.rule.pattern == e.srcElement.innerText){
                    return;
                }
                this.__config.rule.pattern = e.srcElement.innerText;
            }else if(this.__replaceField == e.srcElement){
                if(this.__config.rule.replace == e.srcElement.innerText){
                    return;
                }
                this.__config.rule.replace = e.srcElement.innerText;
            }else{
                return;
            }
            //console.info(this.__config);
            if(window.App){
                window.App.doAction(4,JSON.stringify(this.__config),this.__config.groupId);
            }
        },
		onSelectFile: function(e,fieldEl){
			var s = window.App.getFile();
            if(s===""){
               return;
            }
            this.__replaceField.innerText = s;
			this.__config.rule.replace = s;
			if(window.App){
				window.App.doAction(4,JSON.stringify(this.__config),this.__config.groupId);
			}
		},
		onSelectDir: function(e,fieldEl){
            var s = window.App.getDir();
            if(s===""){
               return;
            }
            this.__replaceField.innerText = s;
			this.__config.rule.replace = s;
			if(window.App){
				window.App.doAction(4,JSON.stringify(this.__config),this.__config.groupId);
			}
        },
        updateButtonState:function(type){
            if(type === 4 || type === 5){
                this.__selectFileField.classList.remove("hidden");
                this.__selectDirField.classList.add("hidden");
            }else if(type === 6){
                this.__selectDirField.classList.remove("hidden");
                this.__selectFileField.classList.add("hidden");
            }else{
                this.__selectFileField.classList.add("hidden");
                this.__selectDirField.classList.add("hidden");
            }
        }
    };

    /**
     * 表示一个规则组（项目）
     * @param {String} groupName
     * @param {Object} groupConfig
     */
    function RuleGroup(groupName, groupConfig){
        var me = this;

        //create the group element
        var el = document.createElement('details');
        el.className = 'group';
        el.innerHTML =
            '<summary class="hgroup">\
                <input class="groupEnableCB" type="checkbox">\
                <span class="groupTitle editable" contentEditable>' + groupName + '</span>\
                <div class="button addRule">+</div>\
                <div class="button removeGroup">x</div>\
            </summary>\
            <div class="desc hgroup">\
                <div class="type">type</div>\
                <div class="pattern">pattern</div>\
                <div class="replace">replace</div>\
            </div>\
            <div class="rules"></div>\
        ';
        el.setAttribute('groupId',groupConfig.id);

        var checkbox 	= el.querySelector('summary > input.groupEnableCB');
        this.__groupName = groupName;
		this.__groupId = groupConfig.id;
        this.__config = groupConfig;
        this.__el = el;
        this.__checkbox = checkbox;
        this.__rulesEl = el.querySelector('.rules');
        this.__rules = [];

        if(groupConfig.rules.length && groupConfig.enable){
            this.expand();
        }
        this.setEnable(groupConfig.enable);

        checkbox.addEventListener('change', function(e){ me.onCheckboxChange(e); });

        el.delegate('.groupTitle', 			'click', 	function(e){ e.preventDefault(); });//prevent group open/collapse when click the group title
        //el.delegate('.groupTitle', 			'dblclick', function(e, el){ me.onGroupTitleDoubleClick(e, el); });//start editing group name
		el.delegate('.groupTitle', 			'focus', 	function(e, el){ me.onGroupTitleFocus(e, el); },true);//focus select all 
        el.delegate('.groupTitle', 			'keydown', 	function(e, el){ me.onGroupTitleEditing(e, el); });//edit group name
        el.delegate('.groupTitle', 			'blur', 	function(e, el){ me.onGroupTitleBlur(e, el); },true);// group name blur check and update
        el.delegate('.button.removeGroup', 	'click', 	function(e){ me.onRemoveGroupButtonClick(e); });//remove this group
        el.delegate('.button.addRule', 		'click', 	function(e){ me.onAddRuleButtonClick(e); });//add rule to this group
        el.delegate('.button.remove', 		'click', 	function(e, el){ me.onRuleRemoveButtonClick(e, el); }, false, true);//remove rule from this group

        //add rules to the group
        var i, length = groupConfig.rules.length;
        for(i=0; i<length; i++){
            if(groupConfig.rules[i].type != 1){ //ignore complex host type
                this.addRule(groupConfig.rules[i],true);
            }
        }
    }
    RuleGroup.prototype = {
        /**
         * 添加一条规则
         * @param {Object} ruleConfig
         */
        addRule: function(ruleConfig,donExpand){
            //console.info("addRule",ruleConfig);
			ruleConfig.groupId = this.__groupId
            var r = new Rule(ruleConfig);
            r.setLineEnable(this.getEnable());
            this.__rulesEl.appendChild(r.getEl());
            this.__rules.push(r);
            if(!donExpand){
                this.expand();
            }
        },
        /**
         * 删除一条规则
         * @param {int} index
         */
        removeRuleAt: function(index){
            var r = this.__rules[index];
            if(r){
                this.__rulesEl.removeChild(r.getEl());
                this.__rules.splice(index, 1);
                return true;
            }
            return false;
        },
        /**
         * 展开分组
         */
        expand: function(){
            this.__el.setAttribute('open', 'open');
        },
        /**
         * 关闭分组
         */
        collapse: function(){
            this.__el.removeAttribute('open');
        },
        /**
         * 获取元素
         * @return {HTMLDivElement}
         */
        getEl: function(){
            return this.__el;
        },
        getEnable: function(){
            return this.__config.enable;
        },
        setEnable: function(enable){
            this.__config.enable = enable;
            if(enable){
                this.__el.classList.remove('disabled');
            }
            else{
                this.__el.classList.add('disabled');
            }
            this.__checkbox.checked = enable;
            for(var i=0; i<this.__rules.length; i++){
                this.__rules[i].setLineEnable(enable);
            }
        },
        getConfig: function(){
            return this.__config;
        },
        getGroupName: function(){
            return this.__groupName;
        },
        onCheckboxChange: function(e){
            this.setEnable(this.__checkbox.checked);
              if(window.App){
                  var o = {
                      'name':this.__groupName,
                      'enable':this.getEnable()
                  };
                  window.App.doAction(3,JSON.stringify(o),this.__groupId);
              }
        },
        onAddRuleButtonClick: function(e){
            e.preventDefault();
            currentGroupName = this.__groupName;
			currentGroupId = this.__groupId;
            showAddRulePanel();
        },
        onRemoveGroupButtonClick: function(e){
            e.preventDefault();
            removeGroup(this.__groupId);
        },
        onRuleRemoveButtonClick: function(e, el){
            var ruleNode = el.parentNode;
            var index = this.__rulesEl.childNodes.indexOf(ruleNode);
            if(index != -1){
                removeRule(this.__groupId, index);
            }
        },
        onGroupTitleDoubleClick: function(e, titleEl){
            startEdit(titleEl);
        },
        onGroupTitleEditing: function(e, titleEl){
            if(e.keyCode == 13){
                //prevent browser create the <br> tag and stop editing
                e.preventDefault();
                stopEdit(titleEl);
            }
        },
		onGroupTitleFocus: function(e, titleEl){
			startEdit(titleEl);
		},
        onGroupTitleBlur: function(e, titleEl){
          titleEl.innerText = titleEl.innerText.replace(/^\s*|\s*$|\n|\r/g,"");
          var newGroupName = titleEl.innerText;
          if(newGroupName == this.__groupName){
              return;
          }
          if(updateGroup(this.__groupId, this.getEnable(), newGroupName)){
              //update group info and configs
              //if(currentGroupName == this.__groupName){
              //    currentGroupName = newGroupName;
              //}
              currentGroupId = this.__groupId;
              this.__groupName = newGroupName;
              //console.info(this);
              if(window.App){
                  var o = {
                      'name':newGroupName,
                      'enable':this.getEnable()
                  };
                  window.App.doAction(3,JSON.stringify(o),this.__groupId);
              }
          }
          else{
              alert('you already have a group named ' + newGroupName);
              titleEl.innerText = this.__groupName;
          }
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    var addLocalGroupBtn;
    var importLocalGroupBtn;
	var addRemoteGroupBtn;
    var panelMask, addGroupPanel, addRulePanel;
    var currentGroupName;
	var currentGroupId;
    var groupsContainer;

    var groups = [];
    var configs = [];
	var TEST_CONFIGS = {
	    "version": 1.0,
	    "groups": [
	        {
                "id":5,
	            "name": "group1",
                "enable": false,
	            "rules": [{
                              "id":4,
	                "name": "simple address example",
	                "type": 2,
	                "enable": false,
	                "rule": {
	                    "pattern": "http://abc.com",
	                    "replace": "172.168.0.1"
	                }
                }, {
	                "name": "remote content example",
	                "type": 3,
	                "enable": true,
	                "rule": {
	                    "pattern": "http://abc.com/a.html",
	                    "replace": "http://123.com/just_a_test/somedir/b.html"
	                }
	            }, {
                              "id":3,
	                "name": "local file example",
	                "type": 4,
	                "enable": true,
	                "rule": {
	                    "pattern": "http://abc.com/b.html",
	                    "replace": "files.qzmin"
	                }
	            }, {
                    "id":2,
	                "name": "local files example",
	                "type": 5,
	                "enable": true,
	                "rule": {
	                    "pattern": "http://abc.com/c.html",
	                    "replace": "C:/a.html.qzmin"
	                }
	            }, {
                    "id":1,
	                "name": "local directory example",
	                "type": 6,
	                "enable": true,
	                "rule": {
	                    "pattern": "http://abc.com/mydir/",
	                    "replace": "C:/replacement/"
	            	}
				}]
	        }
	    ]
	};

    function cleanup(){
        for(var i=0; i<groups.length; i++){
            groupsContainer.removeChild(groups[i].getEl());
        }
        groups = [];
        configs = [];
    }
    function createGroups(configurations){
        cleanup();
        configs = configurations;
        //console.info(configs);
        for(var i=0; i<configs.length; i++){
            addGroup(configs[i].name, configs[i], true);
        }
    }
    function addNewGroup(groupNameOrRemoteUrl,type){
        var newGroups,groupsObj;
        switch(type){
        case "importRemote":
			if(window.App){
                newGroups = window.App.doAction(2,groupNameOrRemoteUrl);
				if(newGroups!=""){
					try{
                        groupsObj = eval('('+newGroups+')');
						if(!groupsObj['groups']){
							return true;
						}
						for(var i=0;i<groupsObj['groups'].length;++i){
							addGroup(groupsObj['groups'][i].name,groupsObj['groups'][i],true);
						}
					}catch(e){
						return true;
					}
				}
				return true;
			}
            break;
        case "importLocal":
            if(window.App){
                newGroups = window.App.doAction(7,groupNameOrRemoteUrl);
                if(newGroups!=""){
                    try{
                        groupsObj = eval('('+newGroups+')');
                        if(!groupsObj['groups']){
                            return true;
                        }
                        for(var i=0;i<groupsObj['groups'].length;++i){
                            addGroup(groupsObj['groups'][i].name,groupsObj['groups'][i],true);
                        }
                    }catch(e){
                        return true;
                    }
                }
                return true;
            }
            break;
        default:
		  var c = {"name":groupNameOrRemoteUrl, "enable":true, "rules":[]};
		  configs.push(c);

		  //create the group element

		  if(window.App){
			  var newGroup = window.App.doAction(0,JSON.stringify(c));
			  if(newGroup == ""){
				alert("add group failed");
				return true;
			  }
			  var newGroup = eval('('+newGroup+')');
			  c.id=newGroup.id;
		  }		  
          var group = new RuleGroup(groupNameOrRemoteUrl, c);
          groupsContainer.appendChild(group.getEl());
          groups.push(group);
          return true;
          break;
        }
      }

    function addGroup(groupName, groupConfig, ignoreConflict){
        var existed = getGroupIndex(groupConfig.id) != -1;
        if(!ignoreConflict && existed){
            alert('you already have a group named ' + groupName);
            return false;
        }
        else{
            //create the default group config
            var c = groupConfig || {"name":groupName, "enable":true, "rules":[]};
            if(!existed){
                configs.push(c);
            }

            //create the group element
            var group = new RuleGroup(groupName, c);
            groupsContainer.appendChild(group.getEl());
            groups.push(group);

			updateConfigs();
            return true;
        }
    }
    function getGroupIndex(groupId){
        for(var i=0; i<configs.length; i++){
            if(configs[i].id == groupId){
                return i;
            }
        }
        return -1;
    }
    function updateGroup(groupId, enable, newName){
        //if(getGroupIndex(newName) != -1){
        //    return false;
        //}
        //else{
            var i = getGroupIndex(groupId);
            //rename group in configs
            configs[i].enable = enable;
            configs[i].name = newName;
            return true;
        //}
    }
    function removeGroup(groupId){
        var i = getGroupIndex(groupId);
        if(i != -1){
            var group = groups[i];
            groupsContainer.removeChild(group.getEl());
            groups.splice(i, 1);
            configs.splice(i, 1);
			if(window.App){
				window.App.doAction(5,groupId);
			}
        }
    }
	function addNewRule(groupId,type,pattern,replace){
        var i = getGroupIndex(groupId);
        if(i != -1){
            var group = groups[i];
            var c = {
                "type": type,
                "enable": true,
                "rule": {
                    "pattern": pattern,
                    "replace": replace
                }
            };
			if(window.App){
				var newRule = window.App.doAction(1,JSON.stringify(c),groupId);
				if(newRule==""){
					alert("add rule failed");
					return true;
				}
				newRule = eval('('+newRule+')');
				c.id = newRule.id;
			}
            group.addRule(c);
            configs[i].rules.push(c);
			updateConfigs();
        }
        else{
            alert('group ' + groupName + ' does not exist!');
        }		
	}
    function removeRule(groupId, index){
        var i = getGroupIndex(groupId);
        if(i != -1){
            var group = groups[i];
            group.removeRuleAt(index);
            var rule = configs[i].rules.splice(index, 1)[0];
            //console.info(rule);
            if(window.App){
                window.App.doAction(6,rule.id,rule.groupId);
            }
        }
        else{
            alert('group ' + groupName + ' does not exist!');
        }
    }

    //show and hide panels
    function showPanel(panel){
        panelMask.classList.remove('hidden');
        panel.classList.remove('hidden');
        var field = panel.querySelector('.field');
        if(field) field.focus();
    }
    function hidePanel(panel){
        panelMask.classList.add('hidden');
        panel.classList.add('hidden');
    }
    function showAddGroupPanel(type){
		addGroupPanel.nameField.value = '';
		addGroupPanel.type = type;
        if(type == "importLocal"){
            addGroupPanel.selectFileBtn.classList.remove('hidden');
        }else{
            addGroupPanel.selectFileBtn.classList.add('hidden');
        }

		showPanel(addGroupPanel);
    }
    function showAddRulePanel(){
        addRulePanel.patternField.value = '';
        addRulePanel.replaceField.value = '';
        showPanel(addRulePanel);
    }

    //start/stop editing the contenteditable element
    function startEdit(el){
        //el.setAttribute('contenteditable', true);
        //el.focus();
        var range = document.createRange();
        range.selectNodeContents(el);
        document.getSelection().addRange(range);
    }
    function stopEdit(el){
        el.blur();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    function init(){

        //get ui components
        addLocalGroupBtn 	= document.getElementById('addLocalGroupBtn');
        importLocalGroupBtn = document.getElementById('importLocalGroupBtn');
        addRemoteGroupBtn 	= document.getElementById('addRemoteGroupBtn');
		
        panelMask 		= document.getElementById('panelMask');
        addGroupPanel 	= document.getElementById('addGroupPanel');
        addRulePanel 	= document.getElementById('addRulePanel');
        groupsContainer = document.querySelector('.groups');

        addGroupPanel.confirmBtn 	= addGroupPanel.querySelector('.button.confirm');
        addGroupPanel.cancelBtn 	= addGroupPanel.querySelector('.button.cancel');
        addGroupPanel.selectFileBtn = addGroupPanel.querySelector('.button.selectFile');
        addGroupPanel.nameField		= document.getElementById('newGroupName');
        addRulePanel.confirmBtn 	= addRulePanel.querySelector('.button.confirm');
        addRulePanel.cancelBtn 		= addRulePanel.querySelector('.button.cancel');
        addRulePanel.typeField		= document.getElementById('newRuleType');
        addRulePanel.patternField	= document.getElementById('newRulePattern');
        addRulePanel.replaceField	= document.getElementById('newRuleReplace');
        addRulePanel.selectFileBtn  = addRulePanel.querySelector('.button.selectFile');
        addRulePanel.selectDirBtn  = addRulePanel.querySelector('.button.selectDir');

        //bind some events
        addLocalGroupBtn.addEventListener('click', function(e){
            showAddGroupPanel("local");
        });
        importLocalGroupBtn.addEventListener('click',function(e){
            showAddGroupPanel("importLocal");
        })
		addRemoteGroupBtn.addEventListener('click', function(e){
            showAddGroupPanel("importRemote");
        });

        addRulePanel.selectFileBtn.addEventListener('click', function(e){
            var s = window.App.getFile();
            if(s===""){
               return;
            }
            addRulePanel.replaceField.value = s;
        });
        addRulePanel.selectDirBtn.addEventListener('click', function(e){
            var s = window.App.getDir();
            if(s===""){
               return;
            }
            addRulePanel.replaceField.value = s;
        });
        addRulePanel.typeField.addEventListener("change",function(e){
            var type = Number(e.target.value);
            if(type === 4 || type === 5){
                addRulePanel.selectDirBtn.classList.add("hidden");
                addRulePanel.selectFileBtn.classList.remove("hidden");
            }else if(type === 6){
                addRulePanel.selectDirBtn.classList.remove("hidden");
                addRulePanel.selectFileBtn.classList.add("hidden");
            }else{
                addRulePanel.selectFileBtn.classList.add("hidden");
                addRulePanel.selectDirBtn.classList.add("hidden");
            }
        });
        addGroupPanel.selectFileBtn.addEventListener('click', function(e){
             if(window.App){
                 var fileName = window.App.getFile();
                 if(fileName!=""){
                    addGroupPanel.nameField.value = fileName;
                 }
             }else{
                 addGroupPanel.nameField.value = "C:\\fakePath\\fake.file";
             }
        });
        addGroupPanel.confirmBtn.addEventListener('click', function(e){
            var groupName = addGroupPanel.nameField.value;
            if(groupName.length && addNewGroup(groupName,addGroupPanel.type)){
                hidePanel(addGroupPanel);
            }
        });
        addGroupPanel.cancelBtn.addEventListener('click', function(e){
            hidePanel(addGroupPanel);
        });
        addRulePanel.confirmBtn.addEventListener('click', function(e){
            var type = Number(addRulePanel.typeField.value);
            var pattern = addRulePanel.patternField.value;
            var replace = addRulePanel.replaceField.value;
            if(pattern.length && replace.length){
                addNewRule(currentGroupId, type, pattern, replace);
                hidePanel(addRulePanel);
            }
        });
        addRulePanel.cancelBtn.addEventListener('click', function(e){
            hidePanel(addRulePanel);
        });
		if(window.App){
			window.App.ruleChanged.connect(window.callbackFromApp);
			var rawConfig = window.App.getConfigs();
			rawConfig = rawConfig.replace(/&quot;/g,"\'");
			rawConfig = eval('(' + rawConfig + ')');
            var tmp = {'groups':[]};// for new RuleManager
            for(var i in rawConfig){
                if(rawConfig.hasOwnProperty(i)){
                    Array.prototype.push.apply(tmp.groups,rawConfig[i].groups);
                }
            }
            rawConfig = tmp;
			createGroups(rawConfig.groups);
		}
		else{
			createGroups(TEST_CONFIGS.groups);
		}
    }

    window.refreshRulesCallback = function(groupsConfigs){
        createGroups(groupsConfigs || []);
    }
    window.callbackFromApp = function(action,data){
		switch(action){
        case 'newRule':
            var el = document.elementFromPoint(data.pos.x,data.pos.y);
            console.info(data.pos.x,data.pos.y,data.url)
            while(el.tagName!="BODY"){
                console.info(el.tagName);
                if(el.tagName == "DETAILS")break;
                el = el.parentNode;
            }
            var groupId;
            if(el.tagName != "DETAILS"){
                //TODO: add new group?
                if(groups.length !=0){
                   groupId = groups[0].__groupId;
                }else{
                    alert("please creat an group first");
                    return;
                }
            }else{
                groupId = el.getAttribute('groupId');
            }
            currentGroupId = groupId;
            showAddRulePanel();
            document.getElementById('newRulePattern').value = data.url;

		}
	}

    document.addEventListener('DOMContentLoaded', init);

})();
