/**
*
*/
Ext.namespace('NMod.User.Ui');

/**
*
*/
NMod.User.Ui.load = function(){
    if (NMod.User.Ui.loaded !== undefined) {
        return;
    }
    NMod.User.Ui.loaded = true;
    
    /**
    * Global var
    */
    var emptySearchText = 'Filter...';
    var fullLevel = '';
    
    Ext.Ajax.request({
    		url: NLib.Path.root('api/cfg/level'),
    		method: 'GET',
    		callback: function(options, success, response){
    			var data = Ext.util.JSON.decode(response.responseText);	
    			if (data.totalcount === 0) {
    				return;
    			}
    			var levels = [];
    			for (var i = 0; i < data.totalcount; i++){
    				levels.push(data.data[i].level);
    			}
    			fullLevel = levels.join(' '); 
    		}
    });
	
    /**
    * Create the request Data Store
    */
    var userStore = new Ext.ux.grid.livegrid.Store({
    		restful: true,
    		autoLoad: true,
    		bufferSize: NMod.Preferences.LIVEGRID_STORE_BUFFER_SIZE,
    		sortInfo: {
    			field: 'email',
    			direction: 'DESC'
    		},
    		remoteSort: true,
    		proxy: new Ext.data.HttpProxy({
    				url: NLib.Path.root('api/user'),
    				method: 'GET',
    				listeners: {
    					exception:  function(proxy, type, action, options, res) {
    						if (type === 'response' && action === 'create') {
    							rowEditor.stopEditing();
    							Ext.Msg.show({
    									title:'Creation error',
    									msg: 'An error occured when creating a new user. Email already used?',
    									buttons: Ext.Msg.OK,
    									fn: search,
    									icon: Ext.MessageBox.WARNING
    							});
    						}
    					},
    					beforeload: function ( DataProxy , params ) {
    						var NModUserSearchTextField = Ext.getCmp("NModUserSearchTextField");
    						if (NModUserSearchTextField === undefined || NModUserSearchTextField === null) {
    							return;
    						}
    						var searchUserValue = NModUserSearchTextField.getValue();
    						if (searchUserValue === emptySearchText) {
    							searchUserValue = "";
    						}
    						searchUserValue = NLib.Convert.toParamValue(searchUserValue);	
    						params.search = searchUserValue;
    						
    					}
    				}
    		}),
    		reader: new Ext.data.JsonReader({
    				idProperty: 'id',
    				root: 'data',
    				totalProperty: 'totalcount', 
    				fields: [
    					{
    						name: 'email'
    					}, {
    						name: 'name'
    					},  {
    						name: 'level'
    					},{
    						name: 'enabled'
    					}, {
    						name: 'password'
    					}
    				]
    		}),
    		writer: new Ext.data.JsonWriter({
    				encode: false
    		})
    });
    
    
    // use RowEditor for editing
    var rowEditor = new Ext.ux.grid.RowEditor({
    		saveText: 'Update'
    });
    
    
    // Add user action
    var actionAddUser = new Ext.Action({
    		text: 'Add',
    		handler: function(){
    			rowEditor.stopEditing();
    			var user = new userStore.recordType({
    					email: 'undefined',
    					name: 'undefined',
    					password: 'undefined',
    					level: fullLevel,
    					enabled: false
    			});
    			userStore.insert(0, user);
    			rowEditor.startEditing(0);
    		},
    		disabled: false,
    		iconCls: 'ks-action-add-icon'
    });
    
    // Remove shared dir action
    var actionDeleteUser = new Ext.Action({
    		text: 'Delete',
    		handler: function(){
    			var selections = userGrid.selModel.getSelections();
    			if (selections.length === 0) {
    				return;
    			}
    			var userRecord = selections[0];
    			if (userRecord === undefined) {
    				return;
    			}
    			Ext.Msg.confirm("Confirm", "Remove selected user?", function(btn, text){
    					if (btn !== 'yes') {
    						return;
    					}
    					userStore.remove(userRecord);
    			}, this);
    		},
    		disabled: false,
    		iconCls: 'ks-action-delete-icon'
    });
    
    // Remove shared dir action
    var actionUpdateUserPassword = new Ext.Action({
    		text: 'Update user password',
    		handler: function(){
    			var selections = userGrid.selModel.getSelections();
    			if (selections.length === 0) {
    				return;
    			}
    			var userRecord = selections[0];
    			if (userRecord === undefined) {
    				return;
    			}
    			Ext.MessageBox.prompt(
    				'Define a password', 
    				'User password', 
    				function(btn, text){
    					if (btn ==='ok') {
    						userRecord.set('password', text);
    						userRecord.commit();
    					}
    				}); 
    		},
    		disabled: false,
    		iconCls: 'ks-action-update-password-icon'
    });
    
    
    
    /**
    * Search user Button
    */
    var searchUserButton = new Ext.Button({
    		id: 'searchUserButton',
    		text: 'Refresh',
    		style: 'padding-left:2px',
    		scope: this,
    		icon: NLib.Path.res('database_refresh'),
    		cls: 'x-btn-text-icon',
    		listeners: {
    			click: function(b, e){
    				search();
    			}
    		}
    });
    
    /**
    * Search function
    */
    function search(){
        userStore.load();
    }
    
    
    /** 
    * Search user TextField
    */
    var searchUserTextField = new Ext.form.TriggerField({
    		id: 'NModUserSearchTextField',
    		width: 200,
    		emptyText: emptySearchText,
    		hideLabel: true,
    		triggerClass: 'x-form-clear-trigger',
    		onTriggerClick: function(e){
    			this.reset();
    			this.focus();
    		},
    		listeners: {
    			render: function(t){
    				var searchUserTextFieldToolTip = new Ext.ToolTip({
    						target: 'NModUserSearchTextField',
    						html: 'Enter here what you are looking for.'
    				});
    			},
    			specialkey: function(f, e){
    				if (e.getKey() === e.ENTER || e.getKey() === e.RETURN) {
    					search();
    				}
    			}
    		}
    });
    
    
    /**
    * Used in top toolbar
    */
    var searchDisplayedText = new Ext.Toolbar.TextItem({
    		cls: 'x-paging-info'
    });
    function updateSearchDisplayedText(rowIndex, visibleRows, totalCount){
        var msg = totalCount === 0 ? 'No results' : String.format('Displaying user from {0} to {1} / total: {2}', rowIndex + 1, rowIndex + visibleRows, totalCount);
        searchDisplayedText.setText(msg, false);
    }
    
    var userToolBar = new Ext.Toolbar({
    		id: 'userToolBar',
    		items: [searchUserTextField, ' ', searchUserButton, ' ', actionAddUser, actionDeleteUser, actionUpdateUserPassword,  '->', searchDisplayedText]
    });
    
    
    /**
    * Create the resultGrid
    */
    var userGrid = new Ext.ux.grid.livegrid.GridPanel({
    		id: 'userGrid',
    		border: false,
    		clicksToEdit: 2,
    		region: 'center',
    		store: userStore,
    		cm: new Ext.grid.ColumnModel([
    				{
    					id: 'email',
    					header: "Email",
    					dataIndex: 'email',
    					width: 300,
    					sortable: true,
    					editor: {
    						xtype: 'textfield',
    						allowBlank: false
    					}
    				},{
    					header: "Name",
    					dataIndex: 'name',
    					sortable: true,
    					width: 300,
    					editor: {
    						xtype: 'textfield',
    						allowBlank: false
    					}
    				},{
    					header: "Levels",
    					dataIndex: 'level',
    					sortable: true,
    					width: 300,
    					editor: {
    						xtype: 'textfield',
    						allowBlank: true
    					}
    				}, {
    					xtype: 'booleancolumn',
    					header: "State",
    					dataIndex: 'enabled',
    					sortable: true,
    					width: 100,
    					trueText: 'enabled',
    					falseText: 'disabled',
    					align: 'center',
    					editor: {
    						xtype: 'checkbox'
    					}		
    				}
    		]),
    		plugins: [rowEditor],
    		stripeRows: true,
    		loadMask: true,
    		selModel: new Ext.ux.grid.livegrid.RowSelectionModel(),
    		view: new Ext.ux.grid.livegrid.GridView({
    				nearLimit: NMod.Preferences.LIVEGRID_VIEW_DEFAULT_NEAR_LIMIT,
    				listeners: {
    					cursormove: function(view, rowIndex, visibleRows, totalCount){
    						updateSearchDisplayedText(rowIndex, visibleRows, totalCount);
    					},
    					rowsinserted: function(view, start, end){
    						updateSearchDisplayedText(view.rowIndex, Math.min(view.ds.totalLength, view.visibleRows - view.rowClipped), view.ds.totalLength);
    					},
    					rowremoved: function(view, index, record){
    						updateSearchDisplayedText(view.rowIndex, Math.min(view.ds.totalLength, view.visibleRows - view.rowClipped), view.ds.totalLength);
    					},
    					beforebuffer: function(view, store, rowIndex, visibleRows, totalCount, options){
    						updateSearchDisplayedText(rowIndex, visibleRows, totalCount);
    					},
    					buffer: function(view, store, rowIndex, visibleRows, totalCount){
    						updateSearchDisplayedText(rowIndex, visibleRows, totalCount);
    					}
    				}
    		})
    });
    
    
    NMod.User.Ui.toolBar = userToolBar;
    NMod.User.Ui.mainPanel = userGrid;
    return NMod.User.Ui;
};

