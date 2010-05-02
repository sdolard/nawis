
Ext.BLANK_IMAGE_URL = 'lib/ext-3.2.1/resources/images/default/s.gif';

/**
*
*/
Ext.namespace('NMod.Main.Ui');

// Model dependencies
NMod.Main.jsLoadTags = [{
		name: "soundmanager2",
		path: "lib/soundmanagerv2/soundmanagerv295b-20100323/script/"
}];

NMod.Main.jsLoader = new JSLoad(NMod.Main.jsLoadTags, "./");




/**
*
*/
NMod.Main.Ui.categoryStore = new Ext.data.ArrayStore({
		fields: [
			{name: 'name'}
		]
});


/**
*
*/
NMod.Main.Ui.load = function(){
	if (NMod.Main.Ui.loaded !== undefined) {
		return;
	}
	NMod.Main.Ui.loaded = true;
	
	/**
	* Category store
	*/
	var categoryData = [ 
		["all"],
		["archive"],
		["document"],
		["music"],
		["other"],
		["picture"],
		["movie"]
	];
	NMod.Main.Ui.categoryStore.loadData(categoryData);
	
	/*
	* Preference components
	*/
	NMod.Preferences.Ui.load();
	
	/*
	* About components
	*/
	NMod.About.Ui.load();
	
	
	// nawis label
	var nawisLabel = new Ext.form.Label({
			text: NVersion
	});
	
	// Logout action
	var logoutAction = new Ext.Action({
			text: 'Logout',
			handler: function(){
				Ext.Ajax.request({
						url: NLib.Path.root('api/auth'),
						method: 'DELETE',
						callback: function(options, success, response){
							NLib.Session.clear();
							window.open('/ui', "_self");
						}
				});
			},
			iconCls: 'ks-action-logout-icon'
	});
	
	/*
	* MusicPlayer Components
	*/
	NMod.MusicPlayer.Ui.load();
	
	/*
	* Search Components
	*/
	NMod.Search.Ui.load();
	
	
	/*
	* Music Components
	*/
	NMod.Music.Ui.load();
	
	
	if (!NLib.Session.isLevelSet("admin")) {
		NMod.Preferences.Ui.viewPreferenceAction.setHidden(true);
	}
	
	
	
	/*
	* Application Tabs
	*/
	var applicationTabs = new Ext.TabPanel({
			id: 'applicationTabs',
			region: 'center',
			activeTab: 0,
			margins: '2 5 0 0',
			plain: true,
			items: [{
					layout: 'fit',
					title: 'Music',
					id: 'musicTab',
					tbar: NMod.Music.Ui.toolBar,
					items: NMod.Music.Ui.mainPanel
			}, {
				layout: 'fit',
				title: 'Search',
				id: 'searchTab',
				tbar: NMod.Search.Ui.toolBar,
				items: NMod.Search.Ui.mainPanel
			}]
	});
	
	/*
	* Component only loaded for local connection on server
	*/
	if (NLib.Session.isLevelSet("admin")) {
		/*
		* User Components are only loaded for local connection on server
		*/
		/* TODO: not for first version
		NMod.User.Ui.load();
		var userTab = new Ext.form.FormPanel({
				layout: 'fit',
				title: 'Users',
				id: 'userTab',
				items: NMod.User.Ui.mainPanel,
				tbar: NMod.User.Ui.toolBar
		});
		applicationTabs.add(userTab);*/
		
		
		/*
		* Duplicated tab
		*/
		NMod.Duplicated.Ui.load();
		var duplicatedTab = new Ext.form.FormPanel({
				layout: 'fit',
				title: 'Duplicated',
				id: 'duplicatedTab',
				items: NMod.Duplicated.Ui.mainPanel,
				tbar: NMod.Duplicated.Ui.toolBar
		});
		applicationTabs.add(duplicatedTab);
		
		
		/*
		* Log Components are only loaded for local connection on server
		*/
		NMod.Log.Ui.load();
		var logTab = new Ext.form.FormPanel({
				layout: 'fit',
				title: 'Log',
				id: 'logTab',
				items: NMod.Log.Ui.mainPanel,
				tbar: NMod.Log.Ui.toolBar
		});
		
		applicationTabs.add(logTab);
	}
	
	
	// Picture panel 
	var picturePanel = new Ext.Panel({
			frame: true,
			id: 'pictures-panel',
			title: 'Picture preview',
			autoHeight: true,
			html: '<div id="picturePanelPreview"></div>'
	});
	
	
	var statusBar = new Ext.ux.StatusBar({
			region: 'south',
			height: 27,
			clear: false
	});
	
	var mainRightPanel  = new Ext.Container({                    
			id: 'main-right-panel',
			region: 'east',
			border: false,
			width: 275,
			margins: '2 0 0 0',
			layout: {
				type:'vbox',
				align:'stretch'
			},
			defaults:{
				margins:'0 0 5 0'
			},
			items:[NMod.MusicPlayer.Ui.mainPanel, picturePanel]
	});
	
	/*
	* Application ViewPort
	*/
	var applicationViewPort = new Ext.Viewport({
			layout: 'border',
			items: [new Ext.Toolbar({
						region: 'north',
						height: 27,
						items: [nawisLabel, '->', NMod.About.Ui.accessButton, NMod.Preferences.Ui.viewPreferenceAction, logoutAction]
			}), 
			mainRightPanel,
			applicationTabs, statusBar]
	});
	// Must be done, i don't know why for now
	mainRightPanel.doLayout();
	
	
	// Picture preview init
	NMod.PicturePreview.Ui.load();
	NMod.PicturePreview.Ui.init('picturePanelPreview');
	
	
	setTimeout(function(){
			Ext.get('loading').remove();
			Ext.get('loading-mask').fadeOut({
					remove: true
			});
	}, 250);
	
	NMod.Main.Ui.setStatusAsWorking = function(){
		if (!NMod.Main.Ui.loaded) {
			return;
		}
		NMod.Preferences.currentNopDelay = NMod.Preferences.NOP_WORKING_DELAY();
		statusBar.showBusy('nawis server is working...');
	};
	
	// Nop
	var nop = function(){
		Ext.Ajax.request({
				url: NLib.Path.root('api/nop'),
				method: 'GET',
				callback: function(options, success, response){
					if (!success) {
						window.open('/ui', "_self");
						return;
					}
					var data = Ext.util.JSON.decode(response.responseText);
					
					if (data.status === "JT_NONE") {
						statusBar.setStatus({
								text: 'nawis server is ready',
								iconCls: 'x-status-valid',
								clear: {
									wait: 5000,
									anim: false,
									useDefaults: false
								}
						});
						NMod.Preferences.currentNopDelay = NMod.Preferences.NOP_SLEEPING_DELAY();
						return;
					}
					
					NMod.Preferences.currentNopDelay = NMod.Preferences.NOP_WORKING_DELAY();
					if (data.status === "JT_WATCH_FILES") {
						statusBar.showBusy('nawis server is working... (looking for new files)');
						return;
					}
					if (data.status === "JT_DB_UPDATE") {
						statusBar.showBusy('nawis server is working... (database update pending)');
						return;
					}
					if (data.status === "JT_HASH") {
						statusBar.showBusy('nawis server is working... (hashing files)');
						return;
					}
					if (data.status === "JT_GET_METADATA") {
						statusBar.showBusy('nawis server is working... (getting file metadata)');
						return;
					}
					if (data.status === "JT_BUILD_MUSIC_DB") {
						statusBar.showBusy('nawis server is working... (building music db)');
						return;
					}
					throw new Error("Unmanaged server job type state: " + data.status);
				}
		});
		nop.defer(NMod.Preferences.currentNopDelay, this);
	};
	nop.defer(1000, this);
	
};


/**
*
*/
Ext.onReady(function(){
		// Init the singleton. Any tag-based quick tips will start working.
		Ext.QuickTips.init();
		
		// turn on validation errors beside the field globally
		Ext.form.Field.prototype.msgTarget = 'side';
		
		// Default Ext.form.TextField behaviors
		Ext.form.TextField.prototype.selectOnFocus = true;
		
		// Must be called as last
		NMod.Main.Ui.load();
		
		// TODO: test if flash is available, at least 8 ?
		
		// Must be called as last !!!
		NMod.Main.jsLoader.load(["soundmanager2"], function(){
				soundManager.flashVersion = 9;
				soundManager.url = 'lib/soundmanagerv2/soundmanagerv295b-20100323/swf'; // directory where SM2 .SWFs live
				soundManager.waitForWindowLoad = true;
				// Note that SounndManager will determine and append the appropriate .SWF file to the URL.
				// disable debug mode after development/testing..
				soundManager.debugMode = false;
				soundManager.consoleOnly = true;
				soundManager.onload = function(){
				};
				soundManager.useHighPerformance = true;
				soundManager.useMovieStar = false;
				soundManager.allowFullScreen = false;
				soundManager.go();
		});
});
