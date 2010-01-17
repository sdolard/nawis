/**
*
*/
Ext.namespace('KSMod.Log.Ui');

/**
*
*/
KSMod.Log.Ui.load = function(){
    if (KSMod.Log.Ui.loaded !== undefined) {
        return;
    }
    KSMod.Log.Ui.loaded = true;
    
    /*
    * Global var
    */
    var emptySearchText = 'Filter...';
    
    /*
    * Search reader
    */
    var searchReader = new Ext.data.JsonReader({
    				idProperty: 'id',
    				root: 'data',
    				totalProperty: 'totalcount', 
    				fields: [{
    								name: 'date'
    				}, {
    						name: 'log'
    				}]
    });
    
    /*
    * Create the request Data Store
    */
    var logStore = new Ext.ux.grid.livegrid.Store({
    				autoLoad: false,
    				bufferSize: KSMod.Preferences.LIVEGRID_STORE_BUFFER_SIZE,
    				reader: searchReader,
    				sortInfo: {
    						field: 'date',
    						direction: 'DESC'
    				},
    				remoteSort: true,
    				proxy: new Ext.data.HttpProxy({
    								url: KSLib.Path.root('api/log'),
    								method: 'GET'
    				}),
    				listeners: {
    						beforeload: function(store, options){
    								var searchLogValue = document.getElementById("KSModLogSearchTextField").value;
    								if (searchLogValue === emptySearchText) {
    										searchLogValue = "";
    								}
    								options.params.search = searchLogValue;
    						}
    				}
    });
    
    /*
    * Grid View
    */
    var logView = new Ext.ux.grid.livegrid.GridView({
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
    
    
    /* 
    * Create the resultGrid
    */
    var rowSelectionModel = new Ext.ux.grid.livegrid.RowSelectionModel();
    var logGrid = new Ext.ux.grid.livegrid.GridPanel({
    				id: 'logGrid',
    				border: false,
    				//title:'logGrid',
    				region: 'center',
    				store: logStore,
    				cm: new Ext.grid.ColumnModel([new Ext.grid.RowNumberer({
    										header: '#',
    										width: 30
    				}), {
    						header: "Date",
    						width: 140,
    						sortable: true,
    						dataIndex: 'date'
    				}, {
    						id: 'logCol',
    						header: "Log",
    						width: 20,
    						sortable: true,
    						dataIndex: 'log'
    				}]),
    				stripeRows: true,
    				autoExpandColumn: 'logCol',
    				loadMask: true,
    				selModel: rowSelectionModel,
    				view: logView
    });
    
    
    /* 
    * Search function
    */
    function search(){
        logStore.load();
    }
    
    
    /* 
    * Search log TextField
    */
    var searchLogTextField = new Ext.form.TriggerField({
    				id: 'KSModLogSearchTextField',
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
    								var searchLogTextFieldToolTip = new Ext.ToolTip({
    												target: 'KSModLogSearchTextField',
    												html: 'Enter here what you are looking for. You can set many words. Ex: holydays 2008'
    								});
    						},
    						specialkey: function(f, e){
    								if (e.getKey() === e.ENTER || e.getKey() === e.RETURN) {
    										search();
    								}
    						}
    				}
    });
    
    /* 
    * Search Log Button
    */
    var searchLogButton = new Ext.Button({
    				id: 'searchLogButton',
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
    
    
    /* 
    * Clear Log Button
    */
    var clearLogButton = new Ext.Button({
    				id: 'clearLogButton',
    				text: 'Clear logs',
    				style: 'padding-left:2px',
    				scope: this,
    				icon: KSLib.Path.res('database_delete'),
    				cls: 'x-btn-text-icon',
    				listeners: {
    						click: function(b, e){
    								clearLogConfirm();
    						}
    				}
    });
    
    /*
    * Clear log function
    */
    function clearLog(){
    		Ext.Ajax.request({
    						url: KSLib.Path.root('api/log'),
    						method: 'DELETE',
        				callback: function(o, s, r){
        						search();
        				}
        });
    }
    
    /*
    * Clear log confirmation message
    */
    function clearLogConfirm(){
        Ext.Msg.show({
        				title: 'Clear logs?',
        				msg: 'You will clear all logs. This action is definitive. Continue?',
        				buttons: {
        						yes: 'Clear logs',
        						no: 'Cancel'
        				},
        				fn: function(b, t){
        						if (b === 'yes') {
        								clearLog();
        						}
        				},
        				animEl: 'elId',
        				icon: Ext.MessageBox.QUESTION
        });
    }
    
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
    
    var logToolBar = new Ext.Toolbar({
    				id: 'logToolBar',
    				items: [searchLogTextField, ' ', searchLogButton, ' ', clearLogButton, '->', searchDisplayedText]
    });
    
    /*
    * Logs Auto refresh
    */
    var task = {
        run: function(){
            search();
        },
        interval: KSMod.Preferences.Ui.Log.autoRefreshPeriod
    };
    if (KSMod.Preferences.Ui.Log.autoRefreshPeriod > 0) {
        Ext.TaskMgr.start(task);
    }
    
    KSMod.Log.Ui.toolBar = logToolBar;
    KSMod.Log.Ui.mainPanel = logGrid;
    return KSMod.Log.Ui;
};

