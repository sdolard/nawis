/**
*
*/
Ext.namespace('KSMod.User.Ui');

/**
*
*/
KSMod.User.Ui.load = function(){
    if (KSMod.User.Ui.loaded !== undefined) {
        return;
    }
    KSMod.User.Ui.loaded = true;
    
    /**
    * Global var
    */
    var emptySearchText = 'Filter...';
    
    /**
    * Search reader
    */
    var userReader = new Ext.data.JsonReader({
    				idProperty: 'id',
    				root: 'data',
    				totalProperty: 'totalcount', 
    				fields: [{
    								name: 'lastName'
    				}, {
    						name: 'firstName'
    				}, {
    						name: 'email'
    				}, {
    						name: 'passwordRequested'
    				}, {
    						name: 'level'
    				}, {
    						name: 'registered'
        		}]
    });
    
    /**
    * Create the request Data Store
    */
    var userStore = new Ext.ux.grid.livegrid.Store({
    				autoLoad: true,
    				bufferSize: KSMod.Preferences.LIVEGRID_STORE_BUFFER_SIZE,
    				reader: userReader,
    				sortInfo: {
    						field: 'lastName',
    						direction: 'DESC'
    				},
    				remoteSort: true,
    				proxy: new Ext.data.HttpProxy({
    								url: KSLib.Path.root('api/user'),
    								method: 'GET'
    				}),
    				listeners: {
    						beforeload: function(store, options){
    								var KSModUserSearchTextField = Ext.getCmp("KSModUserSearchTextField");
    								if (KSModUserSearchTextField === undefined || KSModUserSearchTextField === null) {
    										return;
    								}
    								var searchUserValue = KSModUserSearchTextField.value;
    								if (searchUserValue === emptySearchText) {
    										searchUserValue = "";
    								}
    								options.params.search = searchUserValue;
    						},
    						remove: function(s, r, i){
    								Ext.Ajax.request({
    												waitMsg: 'Please wait...',
    												url: KSLib.Path.root('api/user/') + r.id,
    												method: 'DELETE',
    												success: function(response){
    														userStore.reload();
    												},
    												failure: function(response){
    														Ext.MessageBox.alert('Error', 'Could not connect to the server.');
    												}
    								});
    						}
    				}
    });
    
    /**
    * Grid View
    */
    var userView = new Ext.ux.grid.livegrid.GridView({
    				nearLimit: KSMod.Preferences.LIVEGRID_VIEW_DEFAULT_NEAR_LIMIT,
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
    });
    
    
    
    function userNameRenderer(val, p, record){
        return record.data.firstName + " " + record.data.lastName;
    }
    
    /**
    * Create the resultGrid
    */
    var rowSelectionModel = new Ext.ux.grid.livegrid.RowSelectionModel();
    var userGrid = new Ext.ux.grid.livegrid.GridPanel({
    				id: 'userGrid',
    				//title:'userGrid',
    				border: false,
    				region: 'center',
    				store: userStore,
    				cm: new Ext.grid.ColumnModel([new Ext.grid.RowNumberer({
    										header: '#',
    										width: 30
    				}), {
    						header: "User name",
    						width: 200,
    						sortable: true,
    						dataIndex: 'lastName',
    						renderer: userNameRenderer
    				}, {
    						header: "Email address",
    						width: 140,
    						sortable: true,
    						dataIndex: 'email'
    				}, {
    						header: "Password requested",
    						width: 140,
    						sortable: true,
    						dataIndex: 'passwordRequested'
    				}, {
    						header: "level",
    						width: 120,
    						sortable: true,
    						dataIndex: 'level'
    				}, {
    						id: 'registeredCol',
    						header: "Accredited",
    						width: 70,
    						sortable: true,
    						dataIndex: 'registered'
    				}]),
    				stripeRows: true,
    				autoExpandColumn: 'registeredCol',
    				loadMask: true,
    				selModel: rowSelectionModel,
    				view: userView
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
    				id: 'KSModUserSearchTextField',
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
    												target: 'KSModUserSearchTextField',
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
    * Search user Button
    */
    var searchUserButton = new Ext.Button({
    				id: 'searchUserButton',
    				text: 'Refresh',
    				style: 'padding-left:2px',
    				scope: this,
    				icon: KSLib.Path.res('database_refresh'),
    				cls: 'x-btn-text-icon',
    				listeners: {
    						click: function(b, e){
    								search();
    						}
    				}
    });
    
    
    /** 
    * Delete user action
    */
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
    						Ext.Msg.confirm("Confirm", "Delete selected user from list? He could no more connect to nawis.", function(btn, text){
    										if (btn !== 'yes') {
    												return;
    										}
    										userStore.remove(userRecord);
    						}, this);
    				},
    				disabled: false,
    				iconCls: 'ks-action-delete-icon'
    });
    
    /** 
    * Add user action
    */
    var actionAddUser = new Ext.Action({
    				text: 'Add',
    				handler: function(){
    				},
    				disabled: false,
    				iconCls: 'ks-action-add-icon'
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
    				items: [searchUserTextField, ' ', searchUserButton, ' ', actionAddUser, actionDeleteUser, '->', searchDisplayedText]
    });
    
    KSMod.User.Ui.toolBar = userToolBar;
    KSMod.User.Ui.mainPanel = userGrid;
    return KSMod.User.Ui;
};

