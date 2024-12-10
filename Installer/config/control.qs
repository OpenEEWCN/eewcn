function Controller()
{
}

Controller.prototype.LicenseAgreementPageCallback = function()
{
    var widget = gui.currentPageWidget();
    if (widget != null) {
        widget.AcceptLicenseCheckBox.checked = true;
    }
};

Controller.prototype.IntroductionPageCallback = function()
{
    if (installer.isUninstaller()) {
        // Get the current wizard page
        var widget = gui.currentPageWidget(); 
        if (widget != null) {
            widget.findChild("PackageManagerRadioButton").enabled = false;
            widget.findChild("UpdaterRadioButton").enabled = false;
        }
    }
};

function getInstalledMaintenance(){
    //先遍历
    const regdir="HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    var rqo=installer.execute("reg",["query",regdir]);
    if(rqo.length==0){
        return "";
    }
    var programIds=rqo[0].split("\n");
    for(var e of programIds){
        var entrydir=e.trim();
        if(installer.value(entrydir+"\\DisplayName")=="EEW CN"){
            return installer.value(entrydir+"\\UninstallString");
        }
    }
    return "";
}

function quitInstall(){
    gui.clickButton(buttons.CancelButton);
}

if(installer.isUninstaller()){
    //这样写即使静默安装也会执行
    installer.uninstallationFinished.connect(function(){
        if(!installer.fileExists(installer.value("TargetDir")+"/eewcn.exe")){
            installer.performOperation("Delete","@UserStartMenuProgramsPath@/Startup/eewcn.lnk");
        }
    });
}

if(installer.isInstaller()){
    //先看是否已安装
    var maintenance=getInstalledMaintenance();
    if(maintenance!=""){
        //若是，提示一下
        if(QMessageBox.question("maintenance","安装 / Install / インストール","在安装本程序前，需要先卸载已经安装的程序，是否继续？\n"+
                                "You need to uninstall the existing program before installing. Continue?\n"+
                                "インストールする前に既存のプログラムをアンインストールする必要があります。続きますか？",
                                QMessageBox.Yes|QMessageBox.No)==QMessageBox.Yes){
            //若选择确定则卸载旧版
            if(installer.gainAdminRights()&&installer.execute(maintenance,["-c","pr"]).length==0){
                QMessageBox.critical("maintenance","维护 / Maintenance / 保守","未能正确卸载，请尝试在“控制面板”——“卸载程序”中卸载或手动删除程序文件。\n"+
                                     "Failed to uninstall, please try to uninstall the program from “Control Panel”-“Uninstall a Program” or delete the program files manually.\n"+
                                     "アンインストールに失敗しました。「コントロールパネル」-「プログラムのアンインストール」からアンインストールするか、プログラムファイルを手動で削除してください。");
                quitInstall();
            }
            installer.dropAdminRights();
        }else{
            //否则退出安装程序
            quitInstall();
        }
    }
}
