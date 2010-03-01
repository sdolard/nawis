/**
*
*/
Ext.namespace('KSMod.Search.Ui');

/**
*
*/
KSMod.Search.Ui.load = function(){
	if (KSMod.Search.Ui.loaded !== undefined) {
		return;
	}
	KSMod.Search.Ui.loaded = true;
	
	function isPictureRecord(r){
		if (r === undefined) {
			return false;
		}
		return r.data.category === 'picture';
	}
	
	
	/**
	* Global var
	*/
	var emptySearchText = 'What you search...';
	var emptyCategoryText = 'All file type';
	
	
	/**
	* Search reader
	*/
	var searchReader = new Ext.data.JsonReader({
			idProperty: 'id',
			root: 'data',
			totalProperty: 'totalcount', 
			fields: [{
					name: 'fileName'
			}, {
				name: 'added' 
			}, {
				name: 'lastModified'
			}, {
				name: 'category' 
			}, {
				name: 'relativePath'
			}, {
				name: 'size'
			}, {
				name: 'hash'
			}, {
				name: 'album'
			}, {
				name: 'artist'
			}, {
				name: 'comment'
			}, {
				name: 'copyright'
			}, {
				name: 'dateTimeOriginal'
			}, {
				name: 'duration'
			}, {
				name: 'genre'
			}, {
				name: 'height'
			}, {
				name: 'make'
			}, {
				name: 'model'
			}, {
				name: 'title'
			}, {
				name: 'trackNumber'
			}, {
				name: 'width'
			}, {
				name: 'year'
			}, {
				name: 'hasID3Picture',
				type: 'bool'
			}]
	});
	
	
	/**
	* Create the request Data Store
	*/
	var searchStore = new Ext.ux.grid.livegrid.Store({
			autoLoad: false,
			bufferSize: KSMod.Preferences.LIVEGRID_STORE_BUFFER_SIZE,
			reader: searchReader,
			sortInfo: {
				field: 'added',
				direction: 'DESC'
			},
			remoteSort: true,
			proxy: new Ext.data.HttpProxy({
					url: "", // Done in beforeload event
					method: 'GET'
			}),
			listeners: {
				beforeload: function(store, options){
					var searchValue = document.getElementById("KSModSearchSearchTextField").value;
					if (searchValue === emptySearchText) {
						searchValue = "";
					}
					
					var categoryValue = document.getElementById("searchCategoryComboBox").value;
					if (categoryValue === "all") {
						categoryValue = "file";
					}
					if (categoryValue === emptyCategoryText) {
						categoryValue = "file";
					}
					searchValue = KSLib.Convert.toParamValue(searchValue);
					store.proxy.setUrl(KSLib.Path.root('api/search/' + categoryValue + "?search=" + searchValue), true);
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
		var msg = totalCount === 0 ? 'No results' : String.format('Displaying files from {0} to {1} / total: {2}', rowIndex + 1, rowIndex + visibleRows, totalCount);
		searchDisplayedText.setText(msg, false);
	}
	
	/**
	* Grid View
	*/
	var searchView = new Ext.ux.grid.livegrid.GridView({
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
	
	
	/**
	* Result Grid renderer
	*/
	function rendererToHumanByte(val){
		return KSLib.Convert.byteToHuman(parseInt(val, 10));
	}
	function relativePathToTag(path){
		return path.substr(1).split("/").join(", ");
	}
	function rendererControls(val, p, record){
		return '<div class="controlBtn">' +
		'<img src="' +
		KSLib.Path.res('add') +
		'"' +
		'ext:qtip="' +
		"Download" +
		'" width="16" height="16"' +
		'class="control_edit">' +
		'<img src="' +
		KSLib.Path.res('drive_go') +
		'" ext:qtip="' +
		"Preview" +
		'" width="16" height="16"' +
		'class="control_go"></div>' +
		'<div class="controlLink">' +
		'<a href="javascript:;" class="control_play" ext:qtip="tool tip">Play</a>' +
		'<a href="../api/download/' +
		record.data.hash +
		'" class="control_download"> Download</a>' +
		'</div>';
	}
	function fileNameRenderer(val, p, record){
		var preview = '<div class="search-result-preview-img"><img src="';
		var link_target = '';
		switch (record.data.category) {
		case "archive":
			preview += KSLib.Path.res("compress");
			break;
		case "document":
			preview += KSLib.Path.res("page_white");
			break;
		case "music":
			if (record.data.hasID3Picture) {
				preview += KSLib.Path.root('api/music/id3picture/' + record.data.hash);
			} else {
				preview += KSLib.Path.res("music");
			}
			break;
		case "other":
			preview += KSLib.Path.res("page_white_text");
			break;
		case "picture":
			preview += KSLib.Path.root('api/picture/thumb/' + record.data.hash);
			link_target = ' target="_blank" ';
			break;
		case "movie":
			preview += KSLib.Path.res("film");
			break;
		default:
			// no preview
		}
		preview += '" height="24" alt="Thumb" title="Thumb"></div>&nbsp;';
		var test = "";
		var targetName = record.data.title.length === 0 ? record.data.fileName : record.data.title;
		text = '<a href="' + KSLib.Path.root('api/download/' + record.data.hash) + '"' + link_target + '>' + targetName + '</a>';
		/*if (record.data.album.length !== 0) {
		text += ' - ' + record.data.album;
		}
		if (record.data.artist.length !== 0) {
		text += ' - ' + record.data.artist;
		}
		if (record.data.duration !== 0) {
		text += ' - ' + KSLib.Convert.millisecondsToHuman(record.data.duration * 1000);
		}
		if (record.data.year !== 0) {
		text += ' - ' + record.data.year;
		}*/
		
		text += '<br>Tag: ' + relativePathToTag(record.data.relativePath) + ' ' + record.data.comment;
		/*if (record.data.genre.length !== 0) {
		text += '<br>Genre: ' + record.data.genre;
		}
		if (record.data.width !== 0 && record.data.height !== 0) {
		text += '<br>Dimension: ' + record.data.width + 'x' + record.data.height;
		}*/
		/*{
		name: 'make'
		},{
		name: 'model'
		},{
		name: 'trackNumber',
		type: 'int'
		}*/
		if (Ext.isIE) {
			return '<table><tr><td width="32">' + preview + '</td><td width="100%">' + text + '</td></tr></table>';
		}
		else {
			return '<table><tr><td>' + preview + '</td><td>' + text + '</td></tr></table>';
		}
	}
	
	
	/**
	* ResultGridRowSelectionModel
	*/
	var resultGridRowSelectionModel = new Ext.ux.grid.livegrid.RowSelectionModel({
			singleSelect: true,
			listeners: {
				rowselect: function(t, n, r){
					/*if (isFlashMediaRecord(r)) {
					disabledPlayerAction(false);
					selectedRecord = r;
					}
					else {
					disabledPlayerAction(true);
					}*/
					if (isPictureRecord(r)) {
						KSMod.PicturePreview.Ui.overwrite({
								hash: r.data.hash,
								sizeString: KSLib.Convert.byteToHuman(parseInt(r.data.size, 10)),
								name: r.data.fileName
						});
					}
					else {
						//clearPictureTemplate();
					}
				}
			}
	});
	
	/** 
	* Create the resultGrid
	*/
	var resultGrid = new Ext.ux.grid.livegrid.GridPanel({
			id: 'KSModSearchResultGrid',
			border: false,
			store: searchStore,
			cm: new Ext.grid.ColumnModel([new Ext.grid.RowNumberer({
						header: '#',
						width: 35
			}),        /* {
				header: "Controls",
				width: 60,
				sortable: false,
				renderer: rendererControls,
				dataIndex: 'fileName'
				}, */
				{
					id: 'filenameCol',
					header: "File name",
					width: 20,
					sortable: true,
					dataIndex: 'fileName',
					renderer: fileNameRenderer
				}, {
					header: "Added",
					width: 140,
					sortable: true,
					dataIndex: 'added'
				}, {
					header: "Last modification",
					width: 140,
					sortable: true,
					dataIndex: 'lastModified'
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
			selModel: resultGridRowSelectionModel,
			view: searchView,
			listeners: {
				click: function(e){
					var btn = e.getTarget('.controlBtn');
					if (!btn) {
						btn = e.getTarget('.controlLink');
					}
					if (btn) {
						var t = e.getTarget();
						var v = this.getView();
						var rowIdx = v.findRowIndex(t);
						var record = this.getStore().getAt(rowIdx);
						var control = t.className.split('_')[1];
						switch (control) {
						case 'play':
							break;
						case 'download':
							Ext.Msg.alert('Download this record - ' + record.id);
							break;
						case 'edit':
							Ext.Msg.alert('Edit this record - ' + record.id);
							break;
						case 'go':
							Ext.Msg.alert('Go to this record - ' + record.id);
							break;
						}
					}
				},
				dblclick: function(e){
					var t = e.getTarget();
					var v = this.getView();
					var rowIdx = v.findRowIndex(t);
					var record = this.getStore().getAt(rowIdx);
					if (isFlashMediaRecord(record)) {
						var mySMSound = soundManager.getSoundById('mySound');
						if (mySMSound !== undefined) {
							mySMSound.stop();
						}
						playPauseAction.execute();
					}
				}
			}
	});
	
	
	
	
	/**
	* Search function
	*/
	function search(){
		searchStore.load();
	}
	
	/** 
	* Search TextField
	*/
	var searchTextField = new Ext.form.TriggerField({
			id: 'KSModSearchSearchTextField',
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
					var searchTextFieldToolTip = new Ext.ToolTip({
							target: 'KSModSearchSearchTextField',
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
	var searchComboBox = new Ext.form.ComboBox({
	id: 'searchComboBox',
	store: ds,
	displayField:'title',
	typeAhead: false,
	loadingText: 'Searching...',
	//width: 570,
	pageSize:10,
	hideTrigger:true,
	tpl: resultTpl,
	//applyTo: 'search',
	itemSelector: 'div.search-item',
	onSelect: function(record){ // override default onSelect to do redirect
	window.location =
	String.format('http://extjs.com/forum/showthread.php?t={0}&p={1}', record.data.topicId, record.id);
	}
	});
	
	var ds = new Ext.data.Store({
	proxy: new Ext.data.ScriptTagProxy({
	url: 'http://extjs.com/forum/topics-remote.php'
	}),
	reader: new Ext.data.JsonReader({
	root: 'topics',
	totalProperty: 'totalCount',
	id: 'post_id'
	}, [
	{name: 'title', mapping: 'topic_title'},
	{name: 'topicId', mapping: 'topic_id'},
	{name: 'author', mapping: 'author'},
	{name: 'lastPost', mapping: 'post_time', type: 'date', dateFormat: 'timestamp'},
	{name: 'excerpt', mapping: 'post_text'}
	])
	});
	// Custom rendering Template
	var resultTpl = new Ext.XTemplate(
	'<tpl for="."><div class="search-item">',
	'<h3><span>{lastPost:date("M j, Y")}<br />by {author}</span>{title}</h3>',
	'{excerpt}',
	'</div></tpl>'
	);
	
	
	*/
	/**
	* Category combo box
	*/
	var searchCategoryComboBox = new Ext.form.ComboBox({
			id: 'searchCategoryComboBox',
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
					var searchCategoryComboBoxToolTip = new Ext.ToolTip({
							target: 'searchCategoryComboBox',
							html: 'Select a file type'
					});
				},
				select: function(combo, record){
					search(record.data.name);
				}
			}
	});
	
	
	/**
	* Search action
	*/
	var searchAction = new Ext.Action({
			text: 'Search',
			handler: function(){
				search();
			},
			iconCls: 'ks-action-search-icon'
	});
	
	
	/**
	* Toolbar
	*/
	var searchToolBar = new Ext.Toolbar({
			id: 'searchToolBar',
			items: [searchTextField, ' ', searchCategoryComboBox, ' ', searchAction, '->', searchDisplayedText]
	});
	
	
	
	/**
	* Global Status bar
	*/
	KSMod.Search.Ui.toolBar = searchToolBar;
	KSMod.Search.Ui.mainPanel = resultGrid;
	
	return KSMod.Search.Ui;
};

