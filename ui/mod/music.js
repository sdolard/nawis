/**
* Music module
*/
Ext.namespace('NMod.Music.Ui');


/**
* Load module function
*/
NMod.Music.Ui.load = function() {

	var EMPTY_VALUE = '&lt;Not defined&gt;';
	var imageFlowInstance;
	
	if (NMod.Music.Ui.loaded !== undefined) {
		return;
	}
	NMod.Music.Ui.loaded = true;
	
	/**
	* Global var
	*/
	var emptySearchText = 'What you search...';
	
	/**
	* Year reader
	*/
	var yearReader = new Ext.data.JsonReader({
			idProperty: 'year', //OK
			totalProperty: 'totalcount',
			root: 'data'
	}, [{
			name: 'year'
	}]);
	
	
	/**
	* Create the request Data yearStore
	*/
	var yearStore = new Ext.ux.grid.livegrid.Store({
			autoLoad: true,
			bufferSize: NMod.Preferences.LIVEGRID_STORE_BUFFER_SIZE,
			reader: yearReader,
			sortInfo: {
				field: 'year',
				direction: 'DESC'
			},
			remoteSort: true,
			proxy: new Ext.data.HttpProxy({
					url: NLib.Path.root('api/music/year'),
					method: 'GET'
			}),
			listeners: {
				load: function(store){
					if (store.getTotalCount() === 0)	
					{
						genreStore.removeAll();
						artistStore.removeAll();
						albumStore.removeAll();
						titleStore.removeAll();
						return;
					}
					
					if (!yearGridRowSelectionModel.hasSelection()) {
						yearGridRowSelectionModel.selectFirstRow();
						return;
					}
					var r = yearStore.getById(yearGridRowSelectionModel.getSelected().id);
					if (r === undefined) {
						yearGridRowSelectionModel.selectFirstRow();
						return;
					}
				},
				beforeload: function(store, options){
					setBaseParams(options.params, false, false, false, false);
					setSearchParam(store, 'api/music/year');
				}
			}
	});
	
	
	/**
	* yearGridRowSelectionModel
	*/
	var yearGridRowSelectedRecord = undefined; // To manage a load event bug...
	var yearGridRowSelectionModel = new Ext.ux.grid.livegrid.RowSelectionModel({
			singleSelect: true,
			listeners: {
				rowselect: function(t, n, r){
					if (yearGridRowSelectedRecord === r) {
						return;
					}
					yearGridRowSelectedRecord = r;
					genreStore.load();
				}
			}
	});
	
	/**
	* yearView
	*/
	var yearView = new Ext.ux.grid.livegrid.GridView({
			nearLimit: NMod.Preferences.LIVEGRID_VIEW_DEFAULT_NEAR_LIMIT
	});
	
	/** 
	* Create the resultGrid
	*/
	var yearGrid = new Ext.ux.grid.livegrid.GridPanel({
			id: 'NModMusicYearGrid',
			width: 100,
			margins: '2 2 2 2',
			store: yearStore,
			cm: new Ext.grid.ColumnModel([{
						id: 'yearCol',
						header: "Year",
						width: 20,
						sortable: true,
						dataIndex: 'year',
						renderer: function(val, p, record){
							if (record.data.year === -1) {
								return String.format('All ({0})', yearStore.getTotalCount() - 1);
							}
							if (record.data.year === 0) {
								return EMPTY_VALUE;
							}
							return record.data.year;
						}
			}]),
			stripeRows: true,
			autoExpandColumn: 'yearCol',
			selModel: yearGridRowSelectionModel,
			view: yearView,
			loadMask: true
	});
	
	
	/**
	* Genre reader
	*/
	var genreReader = new Ext.data.JsonReader({
			idProperty: 'id', 
			totalProperty: 'totalcount',
			root: 'data'
	}, [{
			name: 'genre'
	}]);
	
	
	/**
	* Create the request Data genreStore
	*/
	var genreStore = new Ext.ux.grid.livegrid.Store({
			autoLoad: false,
			bufferSize: NMod.Preferences.LIVEGRID_STORE_BUFFER_SIZE,
			reader: genreReader,
			sortInfo: {
				field: 'genre',
				direction: 'ASC'
			},
			remoteSort: true,
			proxy: new Ext.data.HttpProxy({
					url: NLib.Path.root('api/music/genre'),
					method: 'GET'
			}),
			listeners: {
				load: function(){
					if (!genreGridRowSelectionModel.hasSelection()) {
						genreGridRowSelectionModel.selectFirstRow();
						return;
					}
					var r = genreStore.getById(genreGridRowSelectionModel.getSelected().id);
					if (r === undefined) {
						genreGridRowSelectionModel.selectFirstRow();
						return;
					}
				},
				beforeload: function(store, options){
					setBaseParams(options.params, true, false, false, false);
					setSearchParam(store, 'api/music/genre');
					
				}
			}
	});
	
	
	/**
	* genreGridRowSelectionModel
	*/
	var genreGridRowSelectedRecord = undefined; // To manage a load event bug...
	var genreGridRowSelectionModel = new Ext.ux.grid.livegrid.RowSelectionModel({
			singleSelect: true,
			listeners: {
				rowselect: function(t, n, r){
					if (genreGridRowSelectedRecord === r) {
						return;
					}
					genreGridRowSelectedRecord = r;
					artistStore.load();
				}
			}
	});
	
	/**
	* genreView
	*/
	var genreView = new Ext.ux.grid.livegrid.GridView({
			nearLimit: NMod.Preferences.LIVEGRID_VIEW_DEFAULT_NEAR_LIMIT,
			loadMask: {
				msg: 'Please wait...'
			}
	});
	
	/** 
	* Create the resultGrid
	*/
	var genreGrid = new Ext.ux.grid.livegrid.GridPanel({
			id: 'NModMusicGenreGrid',
			store: genreStore,
			flex: 2,
			margins: '2 2 2 0',
			cm: new Ext.grid.ColumnModel([{
						id: 'genreCol',
						header: "Genre",
						width: 20,
						sortable: true,
						dataIndex: 'genre',
						renderer: function(val, p, record){
							if (record.data.genre === "genre-all") {
								return String.format('All ({0})', genreStore.getTotalCount() - 1);
							}
							if (record.data.genre === "") {
								return EMPTY_VALUE;
							}
							return record.data.genre;
						}
			}]),
			stripeRows: true,
			autoExpandColumn: 'genreCol',
			selModel: genreGridRowSelectionModel,
			view: genreView,
			loadMask: true
	});
	
	
	/**
	* Artist reader
	*/		
	var artistReader = new Ext.data.JsonReader({
			idProperty: 'id',
			totalProperty: 'totalcount',
			root: 'data'
	}, [{
			name: 'artist'
	}]);
	
	
	/**
	* Create the request Data artistStore
	*/
	var artistStore = new Ext.ux.grid.livegrid.Store({
			autoLoad: false,
			bufferSize: NMod.Preferences.LIVEGRID_STORE_BUFFER_SIZE,
			reader: artistReader,
			sortInfo: {
				field: 'artist',
				direction: 'ASC'
			},
			remoteSort: true,
			proxy: new Ext.data.HttpProxy({
					url: NLib.Path.root('api/music/artist'),
					method: 'GET'
			}),
			listeners: {
				load: function(){
					if (!artistGridRowSelectionModel.hasSelection()) {
						artistGridRowSelectionModel.selectFirstRow();
						return;
					}
					var r = artistStore.getById(artistGridRowSelectionModel.getSelected().id);
					if (r === undefined) {
						artistGridRowSelectionModel.selectFirstRow();
						return;
					}
				},
				beforeload: function(store, options){
					setBaseParams(options.params, true, true, false, false);
					setSearchParam(store, 'api/music/artist');
				}
			}
	});
	
	
	/**
	* artistGridRowSelectionModel
	*/
	var artistGridRowSelectedRecord = undefined; // To manage a load event bug...
	var artistGridRowSelectionModel = new Ext.ux.grid.livegrid.RowSelectionModel({
			singleSelect: true,
			listeners: {
				rowselect: function(t, n, r){
					if (artistGridRowSelectedRecord === r) {
						return;
					}
					artistGridRowSelectedRecord = r;
					albumStore.load();
				}
			}
	});
	
	/**
	* artistView
	*/
	var artistView = new Ext.ux.grid.livegrid.GridView({
			nearLimit: NMod.Preferences.LIVEGRID_VIEW_DEFAULT_NEAR_LIMIT,
			loadMask: {
				msg: 'Please wait...'
			}
	});
	
	/** 
	* Create the resultGrid
	*/
	var artistGrid = new Ext.ux.grid.livegrid.GridPanel({
			id: 'NModMusicArtistGrid',
			store: artistStore,
			flex: 2,
			margins: '2 2 2 0',
			cm: new Ext.grid.ColumnModel([{
						id: 'artistCol',
						header: "Artist",
						width: 20,
						sortable: true,
						dataIndex: 'artist',
						renderer: function(val, p, record){
							if (record.data.artist === "artist-all") {
								return String.format('All ({0})', artistStore.getTotalCount() - 1);
							}
							if (record.data.artist === "") {
								return EMPTY_VALUE;
							}
							return record.data.artist;
						}
			}]),
			stripeRows: true,
			autoExpandColumn: 'artistCol',
			selModel: artistGridRowSelectionModel,
			view: artistView,
			loadMask: true
	});
	
	/**
	* Album reader
	*/
	
	var albumReader = new Ext.data.JsonReader({
			idProperty: 'id',
			totalProperty: 'totalcount',
			root: 'data'
	}, [
		{
			name: 'album'
		},{
			name: 'mainArtist'
		},{
			name: 'frontCoverID3PictureFileHash'
		},{
			name: 'frontCoverPictureFileHash'
		}
	]);
	
	
	/**
	* Create the request Data albumStore
	*/
	var albumStore = new Ext.ux.grid.livegrid.Store({
			autoLoad: false,
			bufferSize: NMod.Preferences.LIVEGRID_STORE_BUFFER_SIZE,
			reader: albumReader,
			sortInfo: {
				field: 'album',
				direction: 'ASC'
			},
			remoteSort: true,
			proxy: new Ext.data.HttpProxy({
					url: NLib.Path.root('api/music/album'),
					method: 'GET'
			}),
			listeners: {
				load: function(){
					if (!albumGridRowSelectionModel.hasSelection()) {
						albumGridRowSelectionModel.selectFirstRow();
						return;
					}
					var r = albumStore.getById(albumGridRowSelectionModel.getSelected().id);
					if (r === undefined) {
						albumGridRowSelectionModel.selectFirstRow();
						return;
					}
				},
				beforeload: function(store, options){
					setBaseParams(options.params, true, true, true, false);
					setSearchParam(store, 'api/music/album');
				}
			}
	});
	
	
	/**
	* albumGridRowSelectionModel
	*/
	var albumGridRowSelectedRecord = undefined; // To manage a load event bug...
	var albumGridRowSelectionModel = new Ext.ux.grid.livegrid.RowSelectionModel({
			singleSelect: true,
			listeners: {
				rowselect: function(t, n, r){
					if (albumGridRowSelectedRecord === r) {
						return;
					}
					albumGridRowSelectedRecord = r;
					titleStore.load();
				}
			}
	});
	
	/**
	* albumView
	*/
	var albumView = new Ext.ux.grid.livegrid.GridView({
			nearLimit: NMod.Preferences.LIVEGRID_VIEW_DEFAULT_NEAR_LIMIT,
			loadMask: {
				msg: 'Please wait...'
			}
	});
	
	/** 
	* Create the resultGrid
	*/
	function albumSelectRenderer(val, p, record){
		// Preview
		var preview = '<img src="{0}" height="16" alt="" title="">';
		var previewP = Ext.BLANK_IMAGE_URL;
		if (record.data.frontCoverID3PictureFileHash.length) {
			previewP = NLib.Path.root('api/music/id3picture/' + record.data.frontCoverID3PictureFileHash);
		} else if (record.data.frontCoverPictureFileHash.length) {
			previewP = NLib.Path.root('api/picture/thumb/' + record.data.frontCoverPictureFileHash);
		}
		preview = String.format(preview, previewP);
		
		// table
		var table = '<table><tr><td>{0}</td><td>{1}</td></tr></table>';
		if (Ext.isIE) {
            table = '<table><tr><td width="32">{0}</td><td width="100%">{1}</td></tr></table>';
        }
        var album =  record.data.album;
		if (record.data.album === "album-all") {
			album = String.format('All ({0})', albumStore.getTotalCount() - 1);
		} else if (record.data.album === "") {
			album = EMPTY_VALUE;
		}
		if (record.data.mainArtist.length > 0){
			album += ' - ' + record.data.mainArtist;
		}
		return String.format(table, preview, album);
	}
	
	var albumGrid = new Ext.ux.grid.livegrid.GridPanel({
			id: 'NModMusicAlbumGrid',
			store: albumStore,
			flex: 2,
			margins: '2 2 2 0',
			cm: new Ext.grid.ColumnModel([{
						id: 'albumCol',
						header: "Album",
						width: 20,
						sortable: true,
						dataIndex: 'album',
						renderer: albumSelectRenderer
			}]),
			stripeRows: true,
			autoExpandColumn: 'albumCol',
			selModel: albumGridRowSelectionModel,
			view: albumView,
			loadMask: true
	});
	
	
	/**
	* Title reader
	*/
	var titleReader = new Ext.data.JsonReader({
			idProperty: 'id',
			totalProperty: 'totalcount',
			root: 'data'
			
	}, [{
			name: 'fileName' /*sortType:'string'*/
	}, {
		name: 'size',
		type: 'int'
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
		name: 'duration',
		type: 'int'
	}, {
		name: 'genre'
	}, {
		name: 'title'
	}, {
		name: 'trackNumber',
		type: 'int'
	}, {
		name: 'year',
		type: 'int'
	}, {
		name: 'hasID3Picture',
		type: 'bool'
	},{
		name: 'frontCoverID3PictureFileHash'
	},{
		name: 'frontCoverPictureFileHash'
	}]);
	
	/**
	* Create the request Data titleStore
	*/
	var titleStore = new Ext.ux.grid.livegrid.Store({
			autoLoad: false,
			bufferSize: NMod.Preferences.LIVEGRID_STORE_BUFFER_SIZE,
			reader: titleReader,
			sortInfo: {
				field: 'album',
				direction: 'ASC'
			},
			remoteSort: true,
			proxy: new Ext.data.HttpProxy({
					url: NLib.Path.root('api/music/title'),
					method: 'GET'
			}),
			listeners: {
				beforeload: function(store, options){
					setBaseParams(options.params, true, true, true, true);
					setSearchParam(store, 'api/music/title');
				}
			}
	});
	
	/**
	* @param {object} params
	* @param {boolean} year
	* @param {boolean} genre
	* @param {boolean} artist
	* @param {boolean} album
	*/
	function setBaseParams(params, year, genre, artist, album){
		
		// Year
		if (year !== undefined && year) {
			var yearRecord = yearGridRowSelectionModel.getSelected();
			if (yearRecord !== undefined) {
				if (yearRecord.data.year === -1) {
					// nothing
				}
				else {
					
					params.year = yearRecord.data.year;
				}
			}
		}
		
		// Genre
		if (genre !== undefined && genre) {
			var genreRecord = genreGridRowSelectionModel.getSelected();
			if (genreRecord !== undefined) {
				if (genreRecord.data.genre === "genre-all") {
					// nothing
				} else {
					
					params.genre = genreRecord.data.genre;
				}
			}
		}
		
		// Artist
		if (artist !== undefined && artist) {
			var selectRecord = artistGridRowSelectionModel.getSelected();
			if (selectRecord !== undefined) {
				if (selectRecord.data.artist === "artist-all") {
					// nothing
				} else {
					
					params.artist = selectRecord.data.artist;
				}
			}
		}
		
		// Album
		if (album !== undefined && album) {
			var albumRecord = albumGridRowSelectionModel.getSelected();
			if (albumRecord !== undefined) {
				if (albumRecord.data.album === "album-all") {
					// nothing
				} else {
					
					params.album = albumRecord.data.album;
				}
			}
		}
		
	}
	
	// Research
	function setSearchParam(store, url){
		var el = document.getElementById("nmod-music-search-text-field");
		if (el !== null) {
			var searchValue = document.getElementById("nmod-music-search-text-field").value;
			if (searchValue !== emptySearchText) {
				searchValue = NLib.Convert.toParamValue(searchValue);
				store.proxy.setUrl(NLib.Path.root(url + "?search=" + searchValue), true);
				return;
			}
			store.proxy.setUrl(NLib.Path.root(url), true);
		}
	}
	
	
	/**
	* titleGridRowSelectionModel
	*/
	var titleGridRowSelectionModel = new Ext.ux.grid.livegrid.RowSelectionModel({
			singleSelect: true,
			listeners: {
				rowselect: function(t, n, r){
				}
			}
	});
	
	/**
	* titleView
	*/
	var titleView = new Ext.ux.grid.livegrid.GridView({
			nearLimit: NMod.Preferences.LIVEGRID_VIEW_DEFAULT_NEAR_LIMIT,
			forceFit: true,
			loadMask: {
				msg: 'Please wait...'
			},
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
	* Title Renderer
	*/
	function titleRenderer(val, p, record){
		if (Ext.isEmpty(record.data.title)) {
			return '<lt;No title@gt;> - ' + record.data.fileName;
		}
		return record.data.title;
	}
	
	/**
	* Duration Renderer
	*/
	function durationRenderer(val, p, record){
		if (record.data.duration === 0) {
			return '';
		}
		return NLib.Convert.millisecondsToHuman(record.data.duration * 1000);
	}
	
	/**
	* Track Renderer
	*/
	function trackRenderer(val, p, record){
		if (record.data.trackNumber === 0) {
			return '';
		}
		return record.data.trackNumber;
	}
	
	/**
	* Year Renderer
	*/
	function yearRenderer(val, p, record){
		if (record.data.year === 0) {
			return '';
		}
		return record.data.year;
	}
	
	/**
	* Album Renderer
	*/
	function albumRenderer(val, p, record){
		// Preview
		var preview = '<img src="{0}" height="16" alt="" title="">';
		var previewP = Ext.BLANK_IMAGE_URL;
		if (record.data.frontCoverID3PictureFileHash.length) {
			previewP = NLib.Path.root('api/music/id3picture/' + record.data.frontCoverID3PictureFileHash);
		} else if (record.data.frontCoverPictureFileHash.length) {
			previewP = NLib.Path.root('api/picture/thumb/' + record.data.frontCoverPictureFileHash);
		} else if (record.data.hasID3Picture) {
			previewP = NLib.Path.root('api/music/id3picture/' + record.data.hash);
		}
		preview = String.format(preview, previewP);
		
		// table
		var table = '<table><tr><td>{0}</td><td>{1}</td></tr></table>';
		if (Ext.isIE) {
            table = '<table><tr><td width="32">{0}</td><td width="100%">{1}</td></tr></table>';
        }
        
		return String.format(table, preview, record.data.album);
	}
	
	
	
	/** 
	* Create the resultGrid
	*/
	var titleGrid = new Ext.ux.grid.livegrid.GridPanel({
			id: 'NModMusicTitleGrid',
			store: titleStore,
			region: 'center', // position for region
			layout: 'fit',
			border: false,
			cm: new Ext.grid.ColumnModel([{
						id: 'titleCol',
						header: "Title",
						width: 180,
						sortable: true,
						dataIndex: 'title',
						renderer: titleRenderer
			}, {
				header: "Album",
				width: 140,
				sortable: true,
				dataIndex: 'album',
				renderer: albumRenderer
			}, {
				header: "Artist",
				width: 70,
				sortable: true,
				dataIndex: 'artist'
			}, {
				header: 'Duration',
				width: 60,
				sortable: true,
				dataIndex: 'duration',
				renderer: durationRenderer
			}, {
				header: 'Genre',
				width: 70,
				sortable: true,
				dataIndex: 'genre'
			}, {
				header: 'Track',
				width: 50,
				sortable: true,
				dataIndex: 'trackNumber',
				renderer: trackRenderer
			}, {
				header: 'Year',
				width: 50,
				sortable: true,
				dataIndex: 'year',
				renderer: yearRenderer
			}]),
			stripeRows: true,
			autoExpandColumn: 'titleCol',
			selModel: titleGridRowSelectionModel,
			view: titleView,
			loadMask: true,
			listeners: {
				dblclick: function(e){
					var t = e.getTarget();
					var v = this.getView();
					var rowIdx = v.findRowIndex(t);
					var record = this.getStore().getAt(rowIdx);
					NMod.MusicPlayer.Ui.play(record);
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
	*
	*/
	var musicCard = {
			layout: 'card',
			id: 'music-card',
			height: 250,
			region: 'north',
			activeItem: 0,
			deferredRender: true,
			border: false,
			split: true, // enable resizing
			minSize: 75, // defaults to 50
			maxSize: 250, // defaults to 50
			border: false,
			items: [
				{
					xtype: 'panel',
					id: 'music-card-search-item',
					layout: 'hbox',
					border: false,
					layoutConfig: {
						align: 'stretch',
						pack: 'start'
					},
					items: [yearGrid, genreGrid, artistGrid, albumGrid]
				},
				{
					layout: 'hbox',
					id: 'music-card-coverflow-item',
					border: false,
					layoutConfig: {
						align: 'stretch',
						pack: 'start'
					},
					defaults: {
						border: false
					},
					items:[{
							flex: 1,
							bodyCssClass: 'hbox-tabpanel-coverflow'
					},
					{
						id: 'tabpanel-coverflow',
						width: 800,bodyCssClass: 'hbox-tabpanel-coverflow',
						html: '<div id="musicCoverFlowContainer" ><div id="musicCoverFlow" class="imageflow"></div></div>'
					},{
						flex: 1,
						bodyCssClass: 'hbox-tabpanel-coverflow'
					}]
				}
				
			]
    };
    
    
    function applyAlbumCover(){
    	if (imageFlowInstance) {
    		return;
    	}
    	var dh = Ext.DomHelper; 
    	var html = '<img src="{0}" longdesc="{1}" width="200" height="200" alt="{2}"/>';
    	var tpl = new Ext.DomHelper.createTemplate(html);
    	var params = {};
    	setBaseParams(params, true, true, true, false);
    	
    	//for
    	
    	Ext.Ajax.request({
    			url: NLib.Path.root('api/music/album?limit=-1'),
    			method: 'GET',
    			success: function (response){
    				var data = Ext.util.JSON.decode(response.responseText);
    				Ext.each(data.data, function(album){
    						if (album == 'album-all'){
    							return true;
    						}
    						var cover = NLib.Path.root('ui/res/nocover.jpg');
    						if (album.frontCoverPictureFileHash){
    							cover = NLib.Path.root(String.format('api/picture/resize/{0}?width=200&height=200', album.frontCoverPictureFileHash));
    						} else if (album.frontCoverID3PictureFileHash) {
    							cover = NLib.Path.root('api/music/id3picture/' + album.frontCoverID3PictureFileHash);
    						}
    						var albumTitle = album.album;
    						if (album.mainArtist.length > 0){
    							albumTitle += ' - ' + album.mainArtist;
    						}
    						tpl.append('musicCoverFlow', [cover, albumTitle, albumTitle]);
    						
    				});
    				
    				imageFlowInstance = new ImageFlow();
    				imageFlowInstance.init({
    						ImageFlowID: 'musicCoverFlow' , 
    						reflections: false, 
    						buttons: true,
    						circular: true,
    						glideToStartID: false,
    						imageCursor: 'pointer',
    						preloadImages: true,
    						reflectionP: 0.0,
    						aspectRatio: 3.0, 
    						imagesM: 0.8, 
    						xStep: 70, 
    						percentLandscape: 66, 
    						percentOther: 50, 
    						imageFocusMax: 4,
    						imageFocusM: 1.3,
    						onClick: function(a){
    							//a.target.url;
    						}
    				});
    			}
    	});
    }
    
	
	/**
	*
	*/
	var mainPanel = new Ext.Panel({
			id: 'music-main-panel',
			width: 700,
			height: 500,
			layout: 'border',
			border: false,
			items: [musicCard, titleGrid]
	});
	
	
	/** 
	* Search TextField
	*/
	var searchTextField = new Ext.form.TriggerField({
			id: 'nmod-music-search-text-field',
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
					var searchTextField = new Ext.ToolTip({
							target: 'nmod-music-search-text-field',
							html: 'Enter here what you are looking for. An artist name, album... You can set many words. Ex: 2006 ACDC'
					});
				},
				specialkey: function(f, e){
					if (e.getKey() === e.ENTER || e.getKey() === e.RETURN) {
						searchAction.execute();
					}
				}
			}
	});
	
	/**
	* Search action
	*/
	var searchAction = new Ext.Action({
			text: 'Search',
			handler: function(){
				yearStore.load();
			},
			iconCls: 'ks-action-search-music-icon'
	});
	
	
	/**
	* Used in top toolbar
	*/
	var searchDisplayedText = new Ext.Toolbar.TextItem({
			cls: 'x-paging-info'
	});
	function updateSearchDisplayedText(rowIndex, visibleRows, totalCount){
		var msg = totalCount === 0 ? 'No results' : String.format('Displaying title from {0} to {1} / total: {2}', rowIndex + 1, rowIndex + visibleRows, totalCount);
		searchDisplayedText.setText(msg, false);
	}
	
	/**
	* Search Toolbar
	*/
	var searchToolBar = new Ext.Toolbar({
			id: 'NModMusicSearchToolBar',
			items: [searchTextField, ' ', searchAction, '-',{
					text: 'Cover flow',
					iconCls: 'ks-action-search-coverflow-icon',
					enableToggle: true,
					toggleHandler: function(button, state){
						Ext.getCmp('nmod-music-search-text-field').setDisabled(state);
						searchAction.setDisabled(state);
						var cardCt = Ext.getCmp('music-card');
						if(state){
							cardCt.getLayout().setActiveItem('music-card-coverflow-item');
							cardCt.setHeight(250);
							
							Ext.getCmp('application-view-port').doLayout();
							applyAlbumCover();
							return;
						}
						cardCt.getLayout().setActiveItem('music-card-search-item');
						
					},
					pressed: false
			}, '->', searchDisplayedText]
	});
	
	function getNextMusicRecordCallback(){
		while (this.selectNext()) {
			if (!NMod.MusicPlayer.Ui.isMusicRecordValid(this.getSelected())) {
				continue;
			}
			return this.getSelected();
		}
		return undefined;
	}
	NMod.MusicPlayer.Ui.setNextCallBack(getNextMusicRecordCallback, titleGridRowSelectionModel);
	
	function getPreviousMusicRecordCallback(){
		while (this.selectPrevious()) {
			if (!NMod.MusicPlayer.Ui.isMusicRecordValid(this.getSelected())) {
				continue;
			}
			return this.getSelected();
		}
		return undefined;
	}
	NMod.MusicPlayer.Ui.setPreviousCallBack(getPreviousMusicRecordCallback, titleGridRowSelectionModel);
	
	
	// Public variables
	NMod.Music.Ui.toolBar = searchToolBar;
	NMod.Music.Ui.mainPanel = mainPanel;
	return NMod.Music.Ui;
};

