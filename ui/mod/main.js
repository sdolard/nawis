
Ext.BLANK_IMAGE_URL = 'lib/ext-3.1.0/resources/images/default/s.gif';

/**
*
*/
Ext.namespace('KSMod.Main.Ui');

// Model dependencies
KSMod.Main.jsLoadTags = [{
		name: "soundmanager2",
		path: "lib/soundmanagerv2/soundmanagerv295a-20090717/script/"
}];

KSMod.Main.jsLoader = new JSLoad(KSMod.Main.jsLoadTags, "./");




/**
*
*/
KSMod.Main.Ui.categoryStore = new Ext.data.ArrayStore({
		fields: [
			{name: 'name'}
		]
});


/**
*
*/
KSMod.Main.Ui.load = function(){
	if (KSMod.Main.Ui.loaded !== undefined) {
		return;
	}
	KSMod.Main.Ui.loaded = true;
	
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
	KSMod.Main.Ui.categoryStore.loadData(categoryData);
	
	/*
	* Preference components
	*/
	KSMod.Preferences.Ui.load();
	
	/*
	* About components
	*/
	KSMod.About.Ui.load();
	
	
	// nawis label
	var nawisLabel = new Ext.form.Label({
			text: KSVersion
	});
	
	// Logout action
	var logoutAction = new Ext.Action({
			text: 'Logout',
			handler: function(){
				Ext.Ajax.request({
						url: KSLib.Path.root('api/auth'),
						method: 'DELETE',
						callback: function(options, success, response){
							KSLib.Session.clear();
							window.open('/ui', "_self");
						}
				});
			},
			iconCls: 'ks-action-logout-icon'
	});
	
	/*
	* MusicPlayer Components
	*/
	KSMod.MusicPlayer.Ui.load();
	
	/*
	* Search Components
	*/
	KSMod.Search.Ui.load();
	
	
	/*
	* Music Components
	*/
	KSMod.Music.Ui.load();
	
	
	if (!KSLib.Session.isLevelSet("admin")) {
		KSMod.Preferences.Ui.viewPreferenceAction.setHidden(true);
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
					tbar: KSMod.Music.Ui.toolBar,
					items: KSMod.Music.Ui.mainPanel
			}, {
				layout: 'fit',
				title: 'Search',
				id: 'searchTab',
				tbar: KSMod.Search.Ui.toolBar,
				items: KSMod.Search.Ui.mainPanel
			}]
	});
	
	/*
	* Component only loaded for local connection on server
	*/
	if (KSLib.Session.isLevelSet("admin")) {
		/*
		* User Components are only loaded for local connection on server
		*/
		/* TODO: not for first version
		KSMod.User.Ui.load();
		var userTab = new Ext.form.FormPanel({
				layout: 'fit',
				title: 'Users',
				id: 'userTab',
				items: KSMod.User.Ui.mainPanel,
				tbar: KSMod.User.Ui.toolBar
		});
		applicationTabs.add(userTab);*/
		
		
		/*
		* Duplicated tab
		*/
		KSMod.Duplicated.Ui.load();
		var duplicatedTab = new Ext.form.FormPanel({
				layout: 'fit',
				title: 'Duplicated',
				id: 'duplicatedTab',
				items: KSMod.Duplicated.Ui.mainPanel,
				tbar: KSMod.Duplicated.Ui.toolBar
		});
		applicationTabs.add(duplicatedTab);
		
		
		/*
		* Log Components are only loaded for local connection on server
		*/
		KSMod.Log.Ui.load();
		var logTab = new Ext.form.FormPanel({
				layout: 'fit',
				title: 'Log',
				id: 'logTab',
				items: KSMod.Log.Ui.mainPanel,
				tbar: KSMod.Log.Ui.toolBar
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
			items:[KSMod.MusicPlayer.Ui.mainPanel, picturePanel]
	});
	
	/*
	* Application ViewPort
	*/
	var applicationViewPort = new Ext.Viewport({
			layout: 'border',
			items: [new Ext.Toolbar({
						region: 'north',
						height: 27,
						items: [nawisLabel, '->', KSMod.About.Ui.accessButton, KSMod.Preferences.Ui.viewPreferenceAction, logoutAction]
			}), 
			mainRightPanel,
			applicationTabs, statusBar]
	});
	// Must be done, i don't know why for now
	mainRightPanel.doLayout();
	
	
	// Picture preview init
	KSMod.PicturePreview.Ui.load();
	KSMod.PicturePreview.Ui.init('picturePanelPreview');
	
	
	setTimeout(function(){
			Ext.get('loading').remove();
			Ext.get('loading-mask').fadeOut({
					remove: true
			});
	}, 250);
	
	KSMod.Main.Ui.setStatusAsWorking = function(){
		if (!KSMod.Main.Ui.loaded) {
			return;
		}
		KSMod.Preferences.currentNopDelay = KSMod.Preferences.NOP_WORKING_DELAY();
		statusBar.showBusy('nawis server is working...');
	};
	
	// Nop
	var nop = function(){
		Ext.Ajax.request({
				url: KSLib.Path.root('api/nop'),
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
						KSMod.Preferences.currentNopDelay = KSMod.Preferences.NOP_SLEEPING_DELAY();
						return;
					}
					
					KSMod.Preferences.currentNopDelay = KSMod.Preferences.NOP_WORKING_DELAY();
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
					throw new Error("Unmanaged server job type state: " + data.status);
				}
		});
		nop.defer(KSMod.Preferences.currentNopDelay, this);
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
		KSMod.Main.Ui.load();
		
		// TODO: test if flash is available, at least 8 ?
		
		// Must be called as last !!!
		KSMod.Main.jsLoader.load(["soundmanager2"], function(){
				soundManager.flashVersion = 9;
				soundManager.url = 'lib/soundmanagerv2/soundmanagerv295a-20090717/swf'; // directory where SM2 .SWFs live
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
