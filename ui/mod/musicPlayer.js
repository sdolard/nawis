/**
* @author sebastiend
*/
/**
*
*/
Ext.namespace('KSMod.MusicPlayer.Ui');

/**
*
*/
KSMod.MusicPlayer.Ui.load = function(){
    if (KSMod.MusicPlayer.Ui.loaded !== undefined) {
        return;
    }
    KSMod.MusicPlayer.Ui.loaded = true;
    
    var currentMusic = undefined;
    var nextCallback = undefined;
    var nextCallbackScope = undefined;
    var previousCallback = undefined;
    var previousCallbackScope = undefined;
    var SLIDER_MAX_VALUE = 100;
    var DEFAULT_VOLUME_VALUE = 80;
    var sliderDragging = false;
    var playerTimer = new Date();
    var loaderTimer = new Date();
    
    /**
    * Play/pause action
    */
    var playPauseAction = new Ext.Action({
    		text: '',
    		handler: function(){
    			// Toggle Pause
    			var mySMSound = soundManager.getSoundById('mySound');
    			if (mySMSound !== undefined) {
    				
    				// If playing sound is same as currentMusic
    				if (mySMSound.url === KSLib.Path.root('api/download/' + currentMusic.hash)) {
    					mySMSound.togglePause();
    					return;
    				}
    				else {
    					stopAction.execute();
    				}
    			}
    			
    			if (currentMusic === undefined) {
    				return;
    			}
    			
    			// Play
    			mySMSound = soundManager.createSound({
    					id: 'mySound',
    					url: KSLib.Path.root('api/download/' + currentMusic.hash),
    					volume: DEFAULT_VOLUME_VALUE
    			});
    			mySMSound.play({
    					whileplaying: function(){
    						if (sliderDragging) {
    							return;
    						}
    						// Timer to unload browser
    						var now = new Date();
    						if (now.getTime() - playerTimer.getTime() < 1000) {
    							return;
    						}
    						playerTimer = now;
    						
    						playerPosSlider.setValue(this.position * SLIDER_MAX_VALUE / this.durationEstimate);
    						playerPos = KSLib.Convert.millisecondsToHuman(this.position);
    						playerDuration = KSLib.Convert.millisecondsToHuman(this.durationEstimate);
    						
    						if (this.loaded) {
    							playerTitle = 'Playing ' + currentMusic.title;
    						}
    						updatePlayerInfoLabel();
    					},
    					onfinish: function(){
    						playerPosSlider.setValue(0);
    						playPauseAction.setIconClass('ks-action-play-icon');
    						// Status bar is a plugin in extjs v3					               
    						playerTitle = currentMusic.title + " played";
    						updatePlayerInfoLabel();
    						
    						playNextAction.execute();
    					},
    					onstop: function(){
    						playerPosSlider.setValue(0);
    						playPauseAction.setIconClass('ks-action-play-icon');
    						// Status bar is a plugin in extjs v3					               
    						playerTitle = currentMusic.title + " stopped";
    						updatePlayerInfoLabel();
    					},
    					onload: function(success){
    						if (!success && this.readyState === 2) {
    							/*Numeric value indicating a sound's current load status
    							0 = uninitialised
    							1 = loading
    							2 = failed/error
    							3 = loaded/success
    							*/
    							playNextAction.execute();
    						}
    					},
    					onpause: function(){
    						playPauseAction.setIconClass('ks-action-play-icon');
    						playerTitle = currentMusic.title + " paused";
    						updatePlayerInfoLabel();
    					},
    					onplay: function(){
    						
    						playPauseAction.setIconClass('ks-action-pause-icon');
    					},
    					onresume: function(){
    						playPauseAction.setIconClass('ks-action-pause-icon');
    						if (this.loaded) {
    							playerTitle = 'Playing ' + currentMusic.title;
    							updatePlayerInfoLabel();
    						}
    					},
    					whileloading: function(){
    						// Timer to unload browser
    						var now = new Date();
    						if (now.getTime() - loaderTimer.getTime() < 1000) {
    							return;
    						}
    						loaderTimer = now;
    						
    						if (this.bytesLoaded !== this.bytesTotal) {
    							var loaded = this.bytesLoaded * 100 / this.bytesTotal;
    							
    							playerTitle = loaded.toFixed(0).toString() + '% - Loading ' + currentMusic.title + '...';
    							playerAlbum = currentMusic.album;
    							playerArtist = currentMusic.artist;
    						}
    						else {
    							if (this.paused){
    								playerTitle = currentMusic.title + " paused";
    								
    							}else {
    								playerTitle = 'Playing ' + currentMusic.title;
    							}
    						}
    						updatePlayerInfoLabel();
    					}
    			});
    			
    			if (currentMusic.hasID3Picture) { 
    				KSMod.PicturePreview.Ui.overwrite({
    						isMusic: true,
    						hash: currentMusic.hash,
    						sizeString: '',
    						name: currentMusic.album
    				});
    			} else {
    				KSMod.PicturePreview.Ui.clear();
    			}
    		},
    		iconCls: 'ks-action-play-icon',
    		disabled: true
    });
    
    
    /**
    * Stop action
    */
    var stopAction = new Ext.Action({
    		text: '',
    		handler: function(){
    			var mySMSound = soundManager.getSoundById('mySound');
    			if (mySMSound === undefined) {
    				return;
    			}
    			mySMSound.stop();
    			soundManager.destroySound("mySound");
    		},
    		iconCls: 'ks-action-stop-icon',
    		disabled: true
    });
    
    
    /**
    * Previous action
    */
    var playPreviousAction = new Ext.Action({
    		text: '',
    		handler: function(){
    			stopAction.execute();
    			if (previousCallback === undefined) {
    				return;
    			}
    			var r = previousCallback.call(previousCallbackScope);
    			currentMusic = fromMusicStoreRecord(r);
    			playPauseAction.execute();
    			
    		},
    		iconCls: 'ks-action-previous-icon',
    		disabled: true
    });
    
    /**
    * Next action
    */
    var playNextAction = new Ext.Action({
    		text: '',
    		handler: function(){
    			stopAction.execute();
    			if (nextCallback === undefined) {
    				return;
    			}
    			var r = nextCallback.call(nextCallbackScope);
    			currentMusic = fromMusicStoreRecord(r);
    			playPauseAction.execute();
    		},
    		iconCls: 'ks-action-next-icon',
    		disabled: true
    });
    
    /**
    * Player pos slider
    */
    var playerPosSlider = new Ext.Slider({
    		width: 263,
    		animate: false,
    		minValue: 0,
    		maxValue: SLIDER_MAX_VALUE,
    		disabled: true,
    		listeners: {
    			dragstart: function(slider, e){
    				sliderDragging = true;
    			},
    			dragend: function(slider, e){
    				var mySMSound = soundManager.getSoundById('mySound');
    				if (mySMSound !== undefined) {
    					mySMSound.setPosition(slider.getValue() * mySMSound.durationEstimate / 100);
    				}
    				sliderDragging = false;
    			}
    		}
    });
    
    
    /**
    * Player volume slider
    */
    var playerVolumeSlider = new Ext.Slider({
    		width: Ext.isOpera ? 120: 150,
    		animate: false,
    		minValue: 0,
    		maxValue: 100,
    		value: DEFAULT_VOLUME_VALUE,
    		disabled: true,
    		listeners: {
    			drag: function(slider, e){
    				var mySMSound = soundManager.getSoundById('mySound');
    				if (mySMSound !== undefined) {
    					mySMSound.setVolume(slider.getValue());
    				}
    			}
    		}
    });
    
    /**
    * Mute volume action
    */
    var muteVolumeAction = new Ext.Action({
    		text: '',
    		handler: function(){
    			var mySMSound = soundManager.getSoundById('mySound');
    			if (mySMSound !== undefined) {
    				mySMSound.setVolume(0);
    				playerVolumeSlider.setValue(0);
    			}
    		},
    		iconCls: 'ks-action-mute-sound-icon',
    		disabled: true
    });
    
    /**
    * Full volume action
    */
    var fullVolumeAction = new Ext.Action({
    		text: '',
    		handler: function(){
    			var mySMSound = soundManager.getSoundById('mySound');
    			if (mySMSound !== undefined) {
    				mySMSound.setVolume(100);
    				playerVolumeSlider.setValue(100);
    			}
    		},
    		iconCls: 'ks-action-sound-icon',
    		disabled: true
    });
    
    /**
    * Disabled player actions
    * @param {Object} disabled
    */
    function disabledPlayerAction(disabled){
        playPreviousAction.setDisabled(disabled);
        playPauseAction.setDisabled(disabled);
        playNextAction.setDisabled(disabled);
        playerPosSlider.setDisabled(disabled);
        playerVolumeSlider.setDisabled(disabled);
        muteVolumeAction.setDisabled(disabled);
        fullVolumeAction.setDisabled(disabled);
    }
    
    var playerInfoLabel = new Ext.form.Label();
    var playerTitle = '--';
    var playerPos = '--';
    var playerDuration = '--';
    var playerAlbum = '--';
    var playerArtist = '--';
    
    function updatePlayerInfoLabel(){
        playerInfoLabel.setText(playerTitle +
        	'<br>' +
        	playerAlbum +
        	'<br>' +
        	playerPos +
        	'/' +
        	playerDuration, false);
    }
    updatePlayerInfoLabel();
    
    
    var musicPlayerPanel = new Ext.FormPanel({
    		frame: true,
    		id: 'KSModMuscPlayerFormPanel',
    		title: 'Player',
    		autoHeight: true,
    		items: [playerInfoLabel, playerPosSlider],
    		bbarCfg: {
    			ctCls: 'x-box-layout-ct custom-class'
    		},
    		bbar: [playPreviousAction, playPauseAction, playNextAction, muteVolumeAction, playerVolumeSlider, fullVolumeAction]
    });
    
    
    function fromMusicStoreRecord(record){
        if (!isMusicRecordValid(record)) {
            return undefined;
        }
        
        return {
            title: record.data.title,
            album: record.data.album,
            artist: record.data.artist,
            hash: record.data.hash,
            hasID3Picture: record.data.hasID3Picture
            // TODO duration ?
        };
    }
    
    function play(musicStoreRecord){
        stopAction.execute();
        currentMusic = fromMusicStoreRecord(musicStoreRecord);
        playPauseAction.execute();
        disabledPlayerAction(false);
    }
    
    function stop(){
        stopAction.execute();
    }
    
    function togglePause(){
        playPauseAction.execute();
    }
    
    function isPlaying(){
        var mySMSound = soundManager.getSoundById('mySound');
        if (mySMSound === undefined) {
            return false;
        }
        // 0 = stopped/uninitialised
        // 1 = playing or buffering sound (play has been called, waiting for data etc.)	
        return mySMSound.playState === 1;
    }
    
    function isPaused(){
        var mySMSound = soundManager.getSoundById('mySound');
        if (mySMSound === undefined) {
            return false;
        }
        return mySMSound.paused;
    }
    
    function playNext(){
        playNextAction.execute();
    }
    
    function playPrevious(){
        playPreviousAction.execute();
    }
    
    function getCurrent(){
        return currentMusic;
    }
    
    function setNextCallBack(callback, callbackScope){
        nextCallback = callback;
        nextCallbackScope = callbackScope;
    }
    
    function setPreviousCallBack(callback, callbackScope){
        previousCallback = callback;
        previousCallbackScope = callbackScope;
    }
    
    function isMusicRecordValid(record){
        return record !== undefined &&
        record.data !== undefined &&
        record.data.title !== undefined &&
        record.data.album !== undefined &&
        record.data.artist !== undefined &&
        record.data.hash !== undefined &&
        record.data.hasID3Picture !== undefined;
    }
    
    
    /**
    * Global Status bar
    */
    KSMod.MusicPlayer.Ui.mainPanel = musicPlayerPanel;
    //KSMod.MusicPlayer.Ui.playList = musicPlayListPanel; // TODO :)
    KSMod.MusicPlayer.Ui.play = play;
    KSMod.MusicPlayer.Ui.stop = stop;
    KSMod.MusicPlayer.Ui.togglePause = togglePause;
    KSMod.MusicPlayer.Ui.isPlaying = isPlaying;
    KSMod.MusicPlayer.Ui.isPaused = isPaused;
    KSMod.MusicPlayer.Ui.playNext = playNext;
    KSMod.MusicPlayer.Ui.playPrevious = playPrevious;
    KSMod.MusicPlayer.Ui.getCurrent = getCurrent;
    KSMod.MusicPlayer.Ui.setNextCallBack = setNextCallBack;
    KSMod.MusicPlayer.Ui.setPreviousCallBack = setPreviousCallBack;
    KSMod.MusicPlayer.Ui.isMusicRecordValid = isMusicRecordValid;
    return KSMod.MusicPlayer.Ui;
};

