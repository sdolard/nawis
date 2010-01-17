/**
*
*/
Ext.namespace('KSMod.Preferences.Ui.Log');

/**
*
*/
KSMod.Preferences.LIVEGRID_VIEW_DEFAULT_NEAR_LIMIT = 100;
KSMod.Preferences.LIVEGRID_STORE_BUFFER_SIZE = 300;
KSMod.Preferences.NOP_SLEEPING_DELAY = function(){
    return 60000;
};

KSMod.Preferences.NOP_WORKING_DELAY = function(){
    return 5000;
};

KSMod.Preferences.currentNopDelay = KSMod.Preferences.NOP_SLEEPING_DELAY();


/**
*
*/
KSMod.Preferences.Ui.Log.autoRefreshPeriod = 0; // value is mseconds, 0 equals to desactivated
/**
*
*/
KSMod.Preferences.Ui.showWindow = function(){		 
    // sharedDirProxy
    var sharedDirProxy = new Ext.data.HttpProxy({
    		url: KSLib.Path.root('api/cfg/shareddir')
    });
    
    var sharedDirReader = new Ext.data.JsonReader({
    		idProperty: 'id',
    		root: 'data',
    		totalProperty: 'totalcount', 
    		fields: [{
    				name: 'id'
    		},{
    			name: 'path'
    		}, {
    			name: 'shared'
    		}, {
    			name: 'recursive'
    		}, {
    			name: 'exists'
    		}]
    });
    
    var sharedDirWriter = new Ext.data.JsonWriter({
    		encode: false
    });
    
    
    // Directory store and reader
    var sharedDirStore = new Ext.data.Store({
    		id: 'sharedDirectory',
    		restful: true,
    		proxy: sharedDirProxy,
    		reader: sharedDirReader,
    		writer: sharedDirWriter,
    		sortInfo: {
    			field: 'path',
    			direction: 'ASC'
    		}				
    });
    
    
    // Renderer for "Exists" columns 
    function ExistsRendererColumn(val, p, record){
    	var tips = "";
    	if (!record.data.exists) {
            tips = '<img src="' +
            KSLib.Path.res('exclamation') +
            '"' +
            'ext:qtip="' +
            "This directory do not exists" +
            '" width="16" height="16">';
        }
        return tips + record.data.path;
    }
    
    // Shared dir column model
    var sharedDirColumns = [
    	{
    		id: 'path',
    		header: "Path",
    		dataIndex: 'path',
    		width: 220,
    		editor: {
                xtype: 'textfield',
                allowBlank: false
            },
            renderer: ExistsRendererColumn
        },
        {
        	xtype: 'booleancolumn',
        	header: "Shared",
        	dataIndex: 'shared',
        	width: 70,
        	trueText: 'Shared',
            falseText: 'Private',
            align: 'center',
            editor: {
                xtype: 'checkbox'
            }
            
        }, 
        {
        	xtype: 'booleancolumn',
        	header: "Recursive",
        	dataIndex: 'recursive',
        	width: 70,
        	trueText: 'Recursive',
            falseText: 'Not recursive',
            align: 'center',
            editor: {
                xtype: 'checkbox'
            }
    }];
    
    sharedDirStore.load();
    
    // use RowEditor for editing
    var rowEditor = new Ext.ux.grid.RowEditor({
    		saveText: 'Update'
    });
    
    
    // Add shared dir action
    var actionAddSharedDir = new Ext.Action({
    		text: 'Add',
    		handler: function(){
    			var dir = new sharedDirStore.recordType({
    					path: '',
    					shared: true,
    					recursive: true
    			});
    			rowEditor.stopEditing();
    			sharedDirStore.insert(0, dir);
    			rowEditor.startEditing(0);
    		},
    		disabled: false,
    		iconCls: 'ks-action-add-icon'
    });
    
    // Remove shared dir action
    var actionRemoveSharedDir = new Ext.Action({
    		text: 'Remove',
    		handler: function(){
    			var selections = sharedDirGrid.selModel.getSelections();
    			if (selections.length === 0) {
    				return;
    			}
    			var dirRecord = selections[0];
    			if (dirRecord === undefined) {
    				return;
    			}
    			Ext.Msg.confirm("Confirm", "Remove selected directory from shared list?", function(btn, text){
    					if (btn !== 'yes') {
    						return;
    					}
    					sharedDirStore.remove(dirRecord);
    			}, this);
    		},
    		disabled: false,
    		iconCls: 'ks-action-delete-icon'
    });
    
    
    /*
    * Search mask
    */
    var waitMask = KSLib.Ajax.newWaitMask("Please wait...", sharedDirStore);
    
    /*
    * Ajax request
    */
    var ajaxConnection = KSLib.Ajax.newAjaxConnection(waitMask);
    
    // Look for modification
    var actionLookForModification = new Ext.Action({
    		text: 'Look for new files now',
    		handler: function(){
    			ajaxConnection.request({
    					url: KSLib.Path.root('api/file/updatedb')
    			});
    			actionLookForModification.disable();
    			KSMod.Main.Ui.setStatusAsWorking();
    		},
    		iconCls: 'ks-action-refresh-icon'
    });
    
    
    // Shared directory grid
    var sharedDirGrid = new Ext.grid.GridPanel({
    		title: "Shared directories",
    		store: sharedDirStore,
    		columns: sharedDirColumns,
    		autoExpandColumn: 'path',
    		selModel: new Ext.grid.RowSelectionModel({
    				singleSelect: true
    		}),
    		plugins: [rowEditor],
    		width: 450,
    		height: 150,
    		tbar: [actionAddSharedDir, actionRemoveSharedDir, actionLookForModification],
    		viewConfig: {
    			forceFit: true
    		}
    		
    });
    
    
    
    // Preference form panel
    var preferenceFormPanel = new Ext.FormPanel({
    		layout: 'fit',
    		labelWidth: 75, // label settings here cascade unless overridden       
    		border: false,
    		defaultType: 'textfield',
    		items: [sharedDirGrid]
    });
    
    
    
    var preferenceWindows = new Ext.Window({
    		title: 'Preferences',
    		iconCls: 'ks-window-preference-icon',
    		modal: true,
    		layout: 'fit',
    		width: 500,
    		height: 300,
    		plain: true,
    		border: false,
    		items: preferenceFormPanel,
    		buttons: [{
    				text: 'Ok',
    				handler: function(){
    					preferenceWindows.close();
    					KSMod.Main.Ui.setStatusAsWorking();
    				}
    		}]
    });
    
    preferenceWindows.show(Ext.get('preferenceToolbarButton'));
};

/**
*
*/
KSMod.Preferences.Ui.load = function(){
    // Shared directories
    
    if (KSMod.Preferences.Ui.loaded !== undefined) {
        return;
    }
    KSMod.Preferences.Ui.loaded = true;
    
    // View preference action
    var viewPreferenceAction = new Ext.Action({
    		text: 'Preferences',
    		handler: function(){
    			KSMod.Preferences.Ui.showWindow();
    		},
    		iconCls: 'ks-action-view-preference-icon'
    });
    
    KSMod.Preferences.Ui.viewPreferenceAction = viewPreferenceAction;
    return KSMod.Preferences.Ui;
};
