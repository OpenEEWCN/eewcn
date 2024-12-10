function Component()
{
    // constructor
    component.loaded.connect(this, Component.prototype.loaded);
    if (!installer.addWizardPage(component, "Page", QInstaller.TargetDirectory)){
        console.log("Could not add the dynamic page.");
    }
    installer.installationFinished.connect(this, Component.prototype.onInstallFinish);
    installer.finishButtonClicked.connect(this, Component.prototype.onClickFinish);
}

Component.prototype.isDefault = function()
{
    // select the component by default
    return true;
};

Component.prototype.createOperations = function()
{
    try {
        // call the base create operations function
        component.createOperations();
    } catch (e) {
        console.log(e);
    }
};

Component.prototype.loaded = function ()
{
    var page = gui.pageByObjectName("DynamicPage");
    if (page != null) {
        console.log("Connecting the dynamic page entered signal.");
        page.entered.connect(Component.prototype.dynamicPageEntered);
    }
};

Component.prototype.dynamicPageEntered = function ()
{
    var pageWidget = gui.pageWidgetByObjectName("DynamicPage");
    if (pageWidget != null) {
        pageWidget.windowTitle = "获取更新";
        pageWidget.m_pageLabel.text = "<html><head/><body><p>当前版本为：%1</p><p>如需获取最新版本，请<a href=\"https://OpenEEWCN.github.io/eewcn-res/link.htm?key=DownloadLatest\"><span style=\"text-decoration: underline; color:#0000ff;\">点击这里</span></a>。</p></body></html>".arg(installer.value("Version"));
    }
};

Component.prototype.onInstallFinish = function()
{
    try {
        if (installer.isInstaller() && installer.status == QInstaller.Success) {
            installer.addWizardPageItem( component, "CheckFinish", QInstaller.InstallationFinished );
            var form = component.userInterface( "CheckFinish" );
            if(form){
                form.checkBoxFinish.checked=true;
                form.checkBoxFinish.text="安装完成后运行"+installer.value("Name")+"(&R)";
            }
        }
    } catch(e) {
        console.log(e);
    }
};

Component.prototype.onClickFinish = function()
{
    try {
        if (installer.isInstaller() && installer.status == QInstaller.Success) {
            var form = component.userInterface( "CheckFinish" );
            if (form && form.checkBoxFinish.checked) {
                installer.executeDetached(installer.value("TargetDir") + "/eewcn.exe",[],installer.value("TargetDir"));
            }
        }
    } catch(e) {
        console.log(e);
    }
}
