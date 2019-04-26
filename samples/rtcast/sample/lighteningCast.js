/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

px.import({
  scene: 'px:scene.1.js',
  keys: 'px:tools.keys.js',
  navBar: '../../../home/root/components/navBar.js',
  Optimus: '../../../home/root/rcvrcore/optimus.js'
}).then(function importsAreReady(imports) {
  let scene = imports.scene;
  let keys = imports.keys;
  let NavBar = imports.navBar;
  let root = scene.root;
  const optimus = imports.Optimus;
  optimus.setScene(scene);
  var ethIP = px.appQueryParams.ethIP; // ethernet Ip address
  if (ethIP === undefined) { ethIP = "NA"; }
  const launchedApps = {};
  let nextAppId = 1001;
  const actualH = 720
  const actualW = 1280

  var fromUI = 1;
  var current_Appname;
  var _launchParams = "";
  var applicationName ;
  var Paramset = 0;
  var LighteningCastObj = null;
//  const directory = px.getPackageBaseFilePath();
  const directory = "/home/root";
  var applicationConfigOverride = process.env.PXSCENE_APPS_CONFIG;

  const launchPage = () => {
    var applicationOverrideList = px.getFile(applicationConfigOverride);
    applicationOverrideList.then((data) => {
      console.log("Using " + applicationConfigOverride);
      loadPage(JSON.parse(data));
    }).catch(function importFailed(err) {
      console.log("Import failed for: " + applicationConfigOverride)
      const applicationDefaultList = px.getFile(`${directory}/appmanagerregistry.conf`);
      applicationDefaultList.then((data) => {
        loadPage(JSON.parse(data));
      }).catch(function importFailed(err) {
        console.error("Import failed for appmanagerregistry.conf: " + err)
      });
    });
  }

  launchPage();

  const loadPage = (data) => {
    const fullScreenW = scene.w;
    const fullScreenH = scene.h;
    const fullScreen = scene.create({
      t: "rect", parent: root, w: fullScreenW, h: fullScreenH, fillColor: 0x292929ff
    })

    const options = [
      {
        name: 'Options'
      }
    ]
    const navBar = new NavBar(scene, { parent: fullScreen, keys: keys, height: 50, options })

    const contentContainer = scene.create({
      t: "rect", parent: fullScreen, w: fullScreen.w, h: (fullScreen.h - navBar.container.h), fillColor: 0x292929ff,
      y: navBar.container.h, x: 0
    })

    const showMenu = (show) => {
      menuContainer.a = show
      menu.container.focus = true
    }

    var hideMenu1 = () => {
      console.log("Hiding Menu");
      fullScreen.a = 0;
    }
    var showMenu1 = () => {
      console.log("Showing Menu");
      fullScreen.a = 1;
      fullScreen.moveToFront();
    }

    function GridItem(params) {
      const data = params.data
      const iconUrl = `${directory}/images/${data.url}`
      const iconBackgroundUrl = `${directory}/images/square.svg`
      const containerH = params.h
      const containerW = params.w
      const containerX = params.x
      const containerY = params.y
      this.container = scene.create({
        id: params.id,
        t: "rect", parent: params.parent, w: params.w, h: params.h, fillColor: 0x00000000,
        x: params.x, y: params.y
      })

      const imageBackgroundW = params.w
      const imageBackgroundH = params.h - 50
      this.imageBackground = scene.create({
        id: "imageBackground",
        t: "image", parent: this.container, w: params.w, h: params.h - 50, url: iconBackgroundUrl,
        stretchX: 1, stretchY: 1
      })

      const imageH = this.imageBackground.h * 0.6
      const imageW = this.imageBackground.w * 0.6
      const imageX = (this.imageBackground.w - imageW) / 2
      const imageY = (this.imageBackground.h - imageH) / 2
      this.image = scene.create({
        t: "image", parent: this.container, url: iconUrl, w: imageH, h: imageW,
        stretchX: 1, stretchY: 1, x: imageX, y: imageY
      });
      const textW = params.w
      const textH = 50
      const textY = params.h - textH
      const textFontSize = 16
      this.text = scene.create({
        t: "textBox", parent: this.container, w: textW, h: textH, x: 0, y: textY,
        text: data.displayName, pixelSize: 16, alignHorizontal: scene.alignHorizontal.CENTER, alignVertical: scene.alignVertical.CENTER
      });

      this.updateSize = (changeW, changeH) => {
        this.container.h = containerH * changeH
        this.container.w = containerW * changeW
        this.container.x = containerX * changeW
        this.container.y = containerY * changeH

        this.imageBackground.h = imageBackgroundH * changeH
        this.imageBackground.w = imageBackgroundW * changeW

        this.image.h = imageH * changeH
        this.image.w = imageW * changeW
        this.image.x = (this.imageBackground.w - this.image.w) / 2
        this.image.y = (this.imageBackground.h - this.image.h) / 2

        this.text.h = textH * changeH
        this.text.w = textW * changeW
        this.text.pixelSize = textFontSize * changeH
        this.text.y = this.container.h - this.text.h
      }
    }

    const itemHeight = 213
    const itemWidth = 163
    const gapH = 30
    const gapv = 30
    function GridView(params) {
      this.currentIndex = 0
      this.previousIndex = 0
      const parent = params.parent
      this.data = params.data
      this.children = []
      this.itemWidth = params.itemWidth
      this.itemHeight = params.itemHeight
      const gridViewX = params.parent.x + 275
      const gridViewY = params.parent.y
      const gridViewW = parent.w
      const gridViewH = parent.h
      this.container = scene.create({
        t: "rect", parent, w: gridViewW, h: gridViewH, fillColor: 0x00000000, x: gridViewX, y: gridViewY
      })
      for (let index = 0; index < this.data.length; index++) {
        const item = this.data[index]
        const y = parseInt(index / params.noOfItemsInRow)
        const x = parseInt(index % params.noOfItemsInRow)
        const config = {
          x: (x * params.itemWidth) + (params.gapH * x),
          y: (y * params.itemHeight) + (params.gapV * y),
          h: params.itemHeight,
          w: params.itemWidth,
          data: item,
          parent: this.container,
          id: index
        }
        const gridItem = new GridItem(config)
        this.children.push(gridItem)
      }

      this.scroll = (keyCode) => {
        const currentItem = this.container.getObjectById(this.currentIndex)
        const requiredH = currentItem.y + (this.itemHeight + params.gapH)
        const navBarH = navBar.container.h
        if (keyCode === keys.UP) {
          if (this.container.y === 0 || this.container.y < 0 || this.container.y === navBarH) {
            this.container.y = 0
            return
          }
          this.container.y = + (currentItem.y + navBarH)
        }
        if (keyCode === keys.DOWN) {
          if (requiredH < this.container.h) return
          this.container.y = - (currentItem.y + navBarH)
        }
      }

      this.container.on("onKeyDown", (e) => {
        if (e.keyCode === keys.ENTER) {
          launchApp();
        }
        this.previousIndex = this.currentIndex
        if (e.keyCode === keys.RIGHT && this.currentIndex < this.data.length - 1) {
          this.setSelected(++this.currentIndex)
          if (this.currentIndex % params.noOfItemsInRow === 0) {
            this.scroll(keys.DOWN)
          }
        }
        if (e.keyCode === keys.LEFT && this.currentIndex > 0) {
          this.setSelected(--this.currentIndex)
          if (this.currentIndex % params.noOfItemsInRow === (params.noOfItemsInRow - 1)) {
            this.scroll(keys.UP)
          }
        }
        if (e.keyCode === keys.UP) {
          const index = this.currentIndex - 4
          if (index >= 0) {
            this.currentIndex = index
            this.setSelected(this.currentIndex)
              this.scroll(e.keyCode)
          }
        }
        if (e.keyCode === keys.DOWN) {
          const index = this.currentIndex + 4
          if (index < this.data.length) {
            this.currentIndex = index
            this.setSelected(this.currentIndex)
              this.scroll(e.keyCode)
          }
        }
      });

      this.container.on("onMouseDown", (e) => {
        const index = e.target.parent.id || gridView.currentIndex
        if (this.previousIndex !== index) {
            this.previousIndex = gridView.currentIndex
            gridView.currentIndex = parseInt(index)
            this.setSelected(gridView.currentIndex)
            launchApp();
        }
      })

      this.container.on("onMouseEnter", (e) => {
        const selectedBackgroundUrl = `${directory}/images/square_select.svg`
        const index = e.target.parent.id || gridView.currentIndex
        if (this.previousIndex !== index) {
            this.previousIndex = gridView.currentIndex
            gridView.currentIndex = parseInt(index)
            this.setSelected(gridView.currentIndex)
            this.container.getObjectById(index).getObjectById('imageBackground').url = selectedBackgroundUrl
        }
      })

      this.container.on("onMouseLeave", (e) => {
        const backgroundUrl = `${directory}/images/square.svg`
        this.container.getObjectById(this.previousIndex).getObjectById('imageBackground').url = backgroundUrl
      })

      this.setFocus = (focus) => {
        this.container.focus = focus
      }
      this.setSelected = (index) => {
        const backgroundUrl = `${directory}/images/square.svg`
        const selectedBackgroundUrl = `${directory}/images/square_select.svg`
        if (this.container.getObjectById(this.previousIndex)) {
          this.container.getObjectById(this.previousIndex).getObjectById('imageBackground').url = backgroundUrl
        }
        if (this.container.getObjectById(index)) {
          this.container.getObjectById(index).getObjectById('imageBackground').url = selectedBackgroundUrl
        }
      }
      this.setSelected(this.currentIndex)

      this.updateSize = (changeW, changeH) => {
        this.container.h = gridViewH * changeH
        this.container.w = gridViewW * changeW
        this.container.x = gridViewX * changeW
        this.container.y = gridViewY * changeH
        this.itemHeight = this.itemHeight * changeH

        this.itemHeight = itemHeight * changeH
        params.gapH = gapH * changeH

        for (let index = 0; index < this.children.length; index++) {
          const gridItem = this.children[index];
          gridItem.updateSize(changeW, changeH)
        }
      }
    }

    const params = {
      parent: contentContainer,
      itemHeight,
      itemWidth,
      gapH: 30,
      gapV: 30,
      noOfItemsInRow: 4,
      maxH: contentContainer.h,
      data: data.applications
    }
    const gridView = new GridView(params)
    gridView.setFocus(true)

    const launchApp = () => {
      if (fromUI) {
        var app = data.applications[gridView.currentIndex]
        console.log("Obtained app details");
        if (!app) return

        var { cmdName, displayName, applicationType, uri } = app
      }
      else {
        for (var index = 0;index < data.applications.length;index ++){
           console.log("Index value" + index);
           var app = data.applications[index]
           if (!app) return
           var { cmdName, displayName, applicationType, uri } = app
           if (app.displayName == applicationName){
             index = data.applications.length;
           }
           else
             displayName = applicationName;
           }
      }
      console.log("launch Application");
      hideMenu1();

      if (launchedApps[displayName] != undefined) {
        console.log("App already launched: " + displayName + " id:" + launchedApps[displayName]);
        var _apps = optimus.getApplications();
        console.log("apps size: " + _apps.length);
        for (var index = 0; index < _apps.length; index++) {
          console.log("_app found with name: " + _apps[index].id);
        }
        optimus.getApplicationById(launchedApps[displayName]).moveToFront();
        optimus.getApplicationById(launchedApps[displayName]).setFocus(true);
      } else {
        console.log("launching Application: " + displayName + " id:" + nextAppId);
        if (!Paramset) {

          if (applicationType == "native") {
            _launchParams = { "cmd": cmdName };
          }
          else if (applicationType == "pxscene") {
            _launchParams = { "cmd": "spark", "uri": uri };
          }
          else if (applicationType == "WebApp") {
            _launchParams = { "cmd": "WebApp", "uri": uri };
          }
        }
        const propsApp = {
          id: nextAppId,
          priority: 1,
          x: 0,
          y: 0,
          w: scene.getWidth(),
          h: scene.getHeight(),
          cx: 0,
          cy: 0,
          sx: 1.0,
          sy: 1.0,
          r: 0,
          a: 1,
          interactive: true,
          painting: true,
          clip: false,
          mask: false,
          draw: true,
          launchParams: _launchParams
        };
        const partnerApp = optimus.createApplication(propsApp);
        optimus.primaryApp = partnerApp;
        partnerApp.moveToFront();
        partnerApp.setFocus(true);
        partnerApp.on('onKeyDown', e => {
          if (e.keyCode === keys.M && keys.is_CTRL(e.flags)) {
            partnerApp.setFocus(false);
            partnerApp.a = 0
            gridView.setFocus(true)

          }
        })
        launchedApps[displayName] = nextAppId;
        nextAppId++;
        current_Appname = displayName;
      }
      console.log('launchApp', app)
    }

    const stopApp = () => {

      if (fromUI) {
        for (var index =0;index<data.applications.length;index++) {
//          var app = data.applications[gridView.currentIndex]
            var app = data.applications[index]
            if (!app) return
            if (app.displayName == current_Appname)
              var { displayName } = app
        }
      }
      else {
        console.log("Display name is not from UI");
        displayName = applicationName;
      }
      console.log("Stopping application: " + displayName + " id:" + launchedApps[displayName])
      optimus.getApplicationById(launchedApps[displayName]).destroy()
      delete launchedApps[displayName]
    }

    function destroyApps() {
      console.log("destroyApps");
      for (var i in launchedApps) {
        console.log("Destroying app:" + launchedApps[i]);
        optimus.getApplicationById(launchedApps[i]).destroy();
        delete launchedApps[i];
      }
    }

    function LoadOptions() {
      delete menuContainer
      menuHeight = 200
      menuContainer = scene.create({
        t: "rect", parent: fullScreen, w: fullScreen.w, h: menuHeight,
        y: fullScreen.h - menuHeight, x: 0, a: 0
      })
      delete menuConfig
      menuConfig = {
        parent: menuContainer,
        options: menuOptions,
        itemWidth: 125,
        itemHeight: 125,
        gapV: 25
      }
      delete menu
      menu = new Menu(menuConfig)
    }

    scene.root.on("onPreKeyDown", function (e) {
      if (e.keyCode == keys.M && keys.is_CTRL(e.flags)) {
        stopApp();
        showMenu1();
        gridView.setFocus(true);
      }
    });

    if (ethIP != "NA") {
      var displayIp = scene.create({
        t: "text",
        text: "eth0 IP: " + ethIP,
        parent: fullScreen,
        x: 1050,
        y: 680,
        textColor: 0xffffffff,
        pixelSize: 14,
        a: 0.6
      });
    }

    const screenW = scene.getWidth();
    const screenH = scene.getHeight();
    if (screenW !== actualW || screenH !== actualH) {
      const changeH = (screenH / actualH)
      const changeW = (screenW / actualW)
      gridView.updateSize(changeW, changeH)
      navBar.updateSize(changeW, changeH)
    }

    const initialW = scene.w;
    const initialH = scene.h;
    scene.on("onResize", function (e) {
      const changeW = e.w / initialW;
      const changeH = e.h / initialH;
      fullScreen.w = fullScreenW * changeW;
      fullScreen.h = fullScreenH * changeH
      gridView.updateSize(changeW, changeH);
      navBar.updateSize(changeW, changeH);
    });
  //}
    function SubscribeForEvents() {
       optimus.on("create", (app) => { console.log("optimus_event created @@@@@@***********")
           console.log("optimus_event: APPNAME  " + app.name);
           });
       optimus.on("suspend", (app) => { console.log("optimus_event suspend @@@@@@***********")
           console.log("optimus_event: APPNAME  " + app.name);
          });
       optimus.on("ready", (app) => { console.log("optimus_event ready @@@@@@***********")
           console.log("optimus_event: APPNAME  " + app.name);
          });
       optimus.on("resume", (app) => { console.log("optimus_event resume @@@@@@***********")
           console.log("optimus_event: APPNAME  " + app.name);
          });
       optimus.on("destroy", (app) => { console.log("optimus_event destroy @@@@@@***********")
           console.log("optimus_event: APPNAME  " + app.name);
          });
       console.log("optimus_event: SubscribeForEvents ***");
    }

    function LaunchApplication(e){
       console.log("LighteningCast LaunchApplications ");
       console.log("App Name :" + e.applicationName);
       console.log("App Params :" + e.parameters);
       fromUI=0;
       applicationName = e.applicationName;
       if (e.applicationName == "netflix"){
          _launchParams = { "cmd": applicationName +" "+ e.parameters };
          Paramset = 1;
       }
       else if((e.applicationName == "Youtube") || (e.applicationName == "youtube") || (e.applicationName == "YouTube TV")){
           _launchParams = {"cmd":"WebApp" , "uri": e.parameters};
          Paramset = 1;
       }
       launchApp();
       Paramset = 0;
       fromUI = 1;
    }

    function StopApplication(e){
       console.log("LighteningCast StopApplication");
       console.log("App Name :" + e.applicationName);

       applicationName = e.applicationName;

        fromUI=0;
        stopApp();
        showMenu1();
        gridView.setFocus(true);
        fromUI = 1;
    }

    function StateRequest(e) {
       console.log("LighteningCast StateRequest");
       console.log("App Name :" + e.applicationName);
       var appStatus = null;
       var appStateObj =null;
       applicationName = e.applicationName;

       if (launchedApps[applicationName] != undefined) {
          appStatus = optimus.getApplicationById(launchedApps[applicationName]).state();
          if((appStatus == "DESTROYED") || (appStatus == "SUSPENDED"))
              appStatus = "stopped";
          else
              appStatus = "running";
          }
       else {
         appStatus = "stopped";
       }
       appStateObj ={applicationName : applicationName, state : appStatus};
       LighteningCastObj.onApplicationStateChanged(appStateObj);

   }

   function registerForLighteningCastEvents() {
      console.log("LighteningCastObj registerForLighteningCastEvents");
       if(LighteningCastObj != null) {
           LighteningCastObj.on("onApplicationLaunchRequest",LaunchApplication);
           LighteningCastObj.on("onApplicationStopRequest",StopApplication);
           LighteningCastObj.on("onApplicationStateRequest",StateRequest);
        }
   }

   var SERVICE_NAME = "com.comcast.lighteningcast";

   while (LighteningCastObj === null)
      LighteningCastObj = scene.getService(SERVICE_NAME);


   registerForLighteningCastEvents();
   SubscribeForEvents();
   }

});
