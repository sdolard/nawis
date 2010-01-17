/**
*
*/
Ext.namespace('KSMod.Duplicated.Ui');

/**
*
*/
KSMod.Duplicated.Ui.load = function(){
		
    if (KSMod.Duplicated.Ui.loaded !== undefined) {
        return;
    }
    KSMod.Duplicated.Ui.loaded = true;
    
    /*
    * Global var
    */
    var emptySearchText = 'What you search...';
    var emptyCategoryText = 'All file type';
    
    /*
    * Search reader
    */
    var duplicatedSearchReader = new Ext.data.JsonReader({
    				idProperty: 'id',
    				root: 'data',
    				totalProperty: 'totalcount', 
    				fields: [{
    								name: 'absoluteFilePath'
    				}, {
    						name: 'originalAbsoluteFilePath'
    				}, {
    						name: 'added'
    				}, {
    						name: 'category'
    				}, {
    						name: 'size'
    				}]
    });
    
    
    /*
    * Create the request Data Store
    */
    duplicatedSearchStore = new Ext.ux.grid.livegrid.Store({
    				autoLoad: false,
    				bufferSize: KSMod.Preferences.LIVEGRID_STORE_BUFFER_SIZE,
    				reader: duplicatedSearchReader,
    				sortInfo: {
    						field: 'size',
    						direction: 'DESC'
    				},
    				remoteSort: true,
    				proxy: new Ext.data.HttpProxy({
    								url: KSLib.Path.root('api/duplicated'),
    								method: 'GET'
    				}),
    				listeners: {
    						beforeload: function(store, options){
    								var searchValue = document.getElementById("KSModDuplicatedSearchTextField").value;
    								if (searchValue === emptySearchText) {
    										searchValue = "";
    								}
    								
    								var categoryValue = document.getElementById("duplicatedCategoryComboBox").value;
    								if (categoryValue === "all") {
    										categoryValue = "file";
    								}
    								if (categoryValue === emptyCategoryText) {
    										categoryValue = "file";
    								}
    								store.proxy.setUrl(KSLib.Path.root('api/duplicated/' + categoryValue), true);
    								store.proxy.setApi({
    												read: KSLib.Path.root('api/duplicated/' + categoryValue)
    								});
    								options.params.search = searchValue;
    						}
    				}
    });
    
    /*
    * Grid View
    */
    var duplicatedSearchView = new Ext.ux.grid.livegrid.GridView({
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
    * Result Grid renderer
    */
    function rendererToHumanByte(val){
        return KSLib.Convert.byteToHuman(parseInt(val, 10));
    }
    function directoryToTag(val){
        return val.substr(1).split("/").join(", ");
    }
    
    function fileNameRenderer(val, p, record){
        var deleteAbsoluteFileControl = '<img src="' +
        KSLib.Path.res('bin') +
        '"ext:qtip="Put in recycle bin" width="16" height="16"' +
        'class="control_deleteAbsoluteFilePath">';
        
        var deleteOriginalAbsoluteFileControl = '<img src="' +
        KSLib.Path.res('bin') +
        '"ext:qtip="Put in recycle bin" width="16" height="16"' +
        'class="control_deleteOriginalAbsoluteFilePath">';
        
        if (Ext.isIE7) {
            return '<div class="duplicatedFilesControlBtn"><table><tr><td width="16">' + deleteAbsoluteFileControl +
            '</td><td width="100%">' +
            record.data.absoluteFilePath +
            '</td><tr><td width="16">' +
            deleteOriginalAbsoluteFileControl +
            '</td><td width="100%">' +
            record.data.originalAbsoluteFilePath +
            '</td></table></div>';
        }
        else {
            return '<div class="duplicatedFilesControlBtn"><table><tr><td>' + deleteAbsoluteFileControl +
            '</td><td>' +
            record.data.absoluteFilePath +
            '</td><tr><td>' +
            deleteOriginalAbsoluteFileControl +
            '</td><td>' +
            record.data.originalAbsoluteFilePath +
            '</td></table></div>';
        }
    }
    
    
    /* 
    * Create the resultGrid
    */
    var rowSelectionModel = new Ext.ux.grid.livegrid.RowSelectionModel();
    var duplicatedResultGrid = new Ext.ux.grid.livegrid.GridPanel({
    				id: 'duplicatedResultGrid',
    				border: false,
    				region: 'center',
    				store: duplicatedSearchStore,
    				cm: new Ext.grid.ColumnModel([new Ext.grid.RowNumberer({
    										header: '#',
    										width: 30
    				}), {
    						id: 'filenameCol',
    						header: "File name",
    						width: 20,
    						sortable: true,
    						renderer: fileNameRenderer,
    						dataIndex: 'absoluteFilePath'
    				}, {
    						header: "Added",
    						width: 140,
    						sortable: true,
    						dataIndex: 'added'
    				}, {
    						header: "Category",
    						width: 70,
    						sortable: true,
    						dataIndex: 'category'
    				}, {
    						header: "Size",
    						width: 70,
    						sortable: true,
    						renderer: rendererToHumanByte,
    						dataIndex: 'size'
    				}]),
    				stripeRows: true,
    				autoExpandColumn: 'filenameCol',
    				loadMask: true,
    				selModel: rowSelectionModel,
    				view: duplicatedSearchView,
    				listeners: {
    						click: function(e){
    								var btn = e.getTarget('.duplicatedFilesControlBtn');
    								
    								if (btn) {
    										var t = e.getTarget();
    										var v = this.getView();
    										var rowIdx = v.findRowIndex(t);
    										var record = this.getStore().getAt(rowIdx);
    										var control = t.className.split('_')[1];
    										switch (control) {
                        case 'deleteAbsoluteFilePath':
                            Ext.Msg.alert('Put absolute file path in recycle bin - ' + record.id);
                            break;
                        case 'deleteOriginalAbsoluteFilePath':
                            Ext.Msg.alert('Delete original absolute file path in recycle bin - ' + record.id);
                            break;
                        }
                    }
                }
            }
    });
    
    
    
    /* 
    * Search function
    */
    function search(){
        duplicatedSearchStore.load();
    }
    
    
    /** 
    * Duplicated Search TextField
    */
    var duplicatedSearchTextField = new Ext.form.TriggerField({
    				id: 'KSModDuplicatedSearchTextField',
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
    								var duplicatedSearchTextFieldToolTip = new Ext.ToolTip({
    												target: 'KSModDuplicatedSearchTextField',
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
    * Category combo box
    */
    var duplicatedCategoryComboBox = new Ext.form.ComboBox({
    				id: 'duplicatedCategoryComboBox',
    				store: KSMod.Main.Ui.categoryStore,
    				displayField: 'name',
    				typeAhead: true,
    				mode: 'local',
    				forceSelection: true,
    				triggerAction: 'all',
    				emptyText: emptyCategoryText,
    				selectOnFocus: true,
    				editable: false,
    				listeners: {
    						render: function(t){
    								var duplicatedCategoryComboBoxToolTip = new Ext.ToolTip({
    												target: 'duplicatedCategoryComboBox',
    												html: 'Select a file type'
    								});
    						},
    						select: function(combo, record){
    								search(record.data.name);
    						}
    				}
    });
    
    /** 
    * Search Button
    */
    var duplicatedSearchButton = new Ext.Button({
    				id: 'duplicatedSearchButton',
    				text: 'Search',
    				style: 'padding-left:2px',
    				scope: this,
    				icon: KSLib.Path.res('drive_magnify'),
    				cls: 'x-btn-text-icon',
    				listeners: {
    						click: function(b, e){
    								search();
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
    
    var duplicatedSearchToolBar = new Ext.Toolbar({
    				id: 'duplicatedSearchToolBar',
    				items: [duplicatedSearchTextField, ' ', duplicatedCategoryComboBox, ' ', duplicatedSearchButton, '->', searchDisplayedText]
    });
    
    KSMod.Duplicated.Ui.toolBar = duplicatedSearchToolBar;
    KSMod.Duplicated.Ui.mainPanel = duplicatedResultGrid;
    return KSMod.Duplicated.Ui;
};

