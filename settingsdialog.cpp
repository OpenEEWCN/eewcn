#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "eewsettings.h"
#include "eewsounds.h"
#include "macloginitems.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFile>
#include <QScreen>
#include <QScrollBar>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setFont(QFont(Value::getLanguageFontName(EEWSettings::GetSettings()->language)));
    setAttribute(Qt::WA_QuitOnClose,false);
    connect(this,&SettingsDialog::SignalResizeWindow,this,&SettingsDialog::ResizeWindow,Qt::QueuedConnection);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::LoadSettings()
{
    emit SignalResizeWindow();
    EEWSettings*eewSettings=EEWSettings::GetSettings();
    ui->spinLatitude->setValue(eewSettings->userLatitude);
    ui->spinLongitude->setValue(eewSettings->userLongitude);
    ui->spinEEWInterval->setValue(eewSettings->eewQueryInterval);
    ui->spinHistoryInterval->setValue(eewSettings->historyQueryInterval);
    ui->spinStationInterval->setValue(eewSettings->stationQueryInterval);
    ui->spinLocalReportLevel->setValue(eewSettings->minimumLocalReportLevel10x/10);
    ui->spinEpicenterReportLevel->setValue(eewSettings->minimumEpicenterReportLevel10x/10);
    ui->spinGlobalReportLevel->setValue(eewSettings->minimumGlobalReportLevel10x/10);
    ui->editPostURL->setText(eewSettings->postURL);
    ui->comboLanguage->setCurrentIndex((int)eewSettings->language);
    ui->buttonPathLocalAlert->setText(eewSettings->pathAlertSound);
    ui->buttonPathLocalAlert->setToolTip(eewSettings->pathAlertSound);
    ui->buttonPathGlobalAlert->setText(eewSettings->pathNewRecordSound);
    ui->buttonPathGlobalAlert->setToolTip(eewSettings->pathNewRecordSound);
    ui->buttonPathLocalArrive->setText(eewSettings->pathSWaveArriveSound);
    ui->buttonPathLocalArrive->setToolTip(eewSettings->pathSWaveArriveSound);
    ui->buttonPathCriticalAlert->setText(eewSettings->pathCriticalAlertSound);
    ui->buttonPathCriticalAlert->setToolTip(eewSettings->pathCriticalAlertSound);
    ui->buttonPathAlertUpdate->setText(eewSettings->pathUpdateAlertSound);
    ui->buttonPathAlertUpdate->setToolTip(eewSettings->pathUpdateAlertSound);
    ui->buttonPathWeakShakeSound->setText(eewSettings->pathWeakShakeSound);
    ui->buttonPathWeakShakeSound->setToolTip(eewSettings->pathWeakShakeSound);
    ui->buttonPathMidShakeSound->setText(eewSettings->pathMidShakeSound);
    ui->buttonPathMidShakeSound->setToolTip(eewSettings->pathMidShakeSound);
    ui->buttonPathStrongShakeSound->setText(eewSettings->pathStrongShakeSound);
    ui->buttonPathStrongShakeSound->setToolTip(eewSettings->pathStrongShakeSound);
    ui->spinRepeatLocalAlert->setValue(eewSettings->repeatsAlertSound);
    ui->spinRepeatGlobalAlert->setValue(eewSettings->repeatsNewRecordSound);
    ui->spinRepeatLocalArrive->setValue(eewSettings->repeatsSWaveArriveSound);
    ui->spinRepeatCriticalAlert->setValue(eewSettings->repeatsCriticalAlertSound);
    ui->spinRepeatAlertUpdate->setValue(eewSettings->repeatsUpdateAlertSound);
    ui->spinRepeatsWeakShakeSound->setValue(eewSettings->repeatsWeakShakeSound);
    ui->spinRepeatsMidShakeSound->setValue(eewSettings->repeatsWeakShakeSound);
    ui->spinRepeatsStrongShakeSound->setValue(eewSettings->repeatsStrongShakeSound);
    ui->spinCriticalLevel->setValue(eewSettings->criticalLevel10x/10);
    ui->spinHistoryQueryCount->setValue(eewSettings->historyQueryCount);
    ui->spinEEWQueryCount->setValue(eewSettings->eewQueryCount);
    ui->spinQueryTimeout->setValue(eewSettings->timeoutNetworkReadMS/1000);
    ui->checkShowWindowOnStartup->setChecked(eewSettings->showWindowOnStartup);
    ui->comboUsingEEWData->setCurrentIndex((int)eewSettings->usingEEWDataSource);
    ui->comboUsingHistoryData->setCurrentIndex((int)eewSettings->usingHistoryDataSource);
    ui->comboUsingStation->setCurrentIndex((int)eewSettings->usingStationDataSource);
    ui->checkDebugEEWData->setChecked(eewSettings->debugUsingEEWDataSource);
    ui->checkDebugHistoryData->setChecked(eewSettings->debugUsingHistoryDataSource);
    ui->checkDebugStation->setChecked(eewSettings->debugUsingStationDataSource);
    ui->checkShowLogoOnMap->setChecked(eewSettings->showLogoOnMap);
    ui->checkShowIntensityOnMap->setChecked(eewSettings->showEstimatedIntensity);
    ui->checkLogToFile->setChecked(eewSettings->logToFile);
    ui->checkClearLogsOnStart->setChecked(eewSettings->clearLogsOnStartup);
    ui->checkAutorun->setChecked(isAutorun());
    ui->checkShowNewRecordIntensity->setChecked(eewSettings->showIntensityOnNewHistory);
    ui->spinStationRecordLength->setValue(eewSettings->stationDataStoreTimeSec);
    ui->spinWeakShakeIntensity->setValue(eewSettings->weakShakeIntensity);
    ui->spinMidShakeIntensity->setValue(eewSettings->midShakeIntensity);
    ui->spinStrongShakeIntensity->setValue(eewSettings->strongShakeIntensity);
    ui->checkDontAutoSwitchTab->setChecked(eewSettings->dontAutoSwitchTab);
    ui->checkEnableStationData->setChecked(eewSettings->enableStationData);
    ui->spinPWaveSpeed->setValue(eewSettings->pwaveSpeed);
    ui->spinSWaveSpeed->setValue(eewSettings->swaveSpeed);
    ui->spinEarthRadius->setValue(eewSettings->earthAvgRadius);
    if(!eewSettings->userExtra.contains("showtesteew")){
        ui->checkDebugEEWData->hide();
        ui->checkDebugHistoryData->hide();
        ui->checkDebugStation->hide();
    }
#if defined(Q_OS_ANDROID)||defined(Q_OS_IOS)
    ui->checkShowWindowOnStartup->setChecked(true);
    ui->checkShowWindowOnStartup->setEnabled(false);
#endif
}

void SettingsDialog::SaveSettings()
{
    EEWSettings*eewSettings=EEWSettings::GetSettings();
    eewSettings->userLatitude=ui->spinLatitude->value();
    eewSettings->userLongitude=ui->spinLongitude->value();
    eewSettings->eewQueryInterval=ui->spinEEWInterval->value();
    eewSettings->historyQueryInterval=ui->spinHistoryInterval->value();
    eewSettings->stationQueryInterval=ui->spinStationInterval->value();
    eewSettings->minimumLocalReportLevel10x=ui->spinLocalReportLevel->value()*10;
    eewSettings->minimumEpicenterReportLevel10x=ui->spinEpicenterReportLevel->value()*10;
    eewSettings->minimumGlobalReportLevel10x=ui->spinGlobalReportLevel->value()*10;
    eewSettings->postURL=ui->editPostURL->text();
    eewSettings->language=(EEWCN_LANGUAGE)ui->comboLanguage->currentIndex();
    eewSettings->pathAlertSound=ui->buttonPathLocalAlert->text();
    eewSettings->pathSWaveArriveSound=ui->buttonPathLocalArrive->text();
    eewSettings->pathNewRecordSound=ui->buttonPathGlobalAlert->text();
    eewSettings->pathCriticalAlertSound=ui->buttonPathCriticalAlert->text();
    eewSettings->pathUpdateAlertSound=ui->buttonPathAlertUpdate->text();
    eewSettings->pathWeakShakeSound=ui->buttonPathWeakShakeSound->text();
    eewSettings->pathMidShakeSound=ui->buttonPathMidShakeSound->text();
    eewSettings->pathStrongShakeSound=ui->buttonPathStrongShakeSound->text();
    eewSettings->repeatsAlertSound=ui->spinRepeatLocalAlert->value();
    eewSettings->repeatsSWaveArriveSound=ui->spinRepeatLocalArrive->value();
    eewSettings->repeatsNewRecordSound=ui->spinRepeatGlobalAlert->value();
    eewSettings->repeatsCriticalAlertSound=ui->spinRepeatCriticalAlert->value();
    eewSettings->repeatsUpdateAlertSound=ui->spinRepeatAlertUpdate->value();
    eewSettings->repeatsWeakShakeSound=ui->spinRepeatsWeakShakeSound->value();
    eewSettings->repeatsMidShakeSound=ui->spinRepeatsMidShakeSound->value();
    eewSettings->repeatsStrongShakeSound=ui->spinRepeatsStrongShakeSound->value();
    eewSettings->criticalLevel10x=ui->spinCriticalLevel->value()*10;
    eewSettings->historyQueryCount=ui->spinHistoryQueryCount->value();
    eewSettings->eewQueryCount=ui->spinEEWQueryCount->value();
    eewSettings->timeoutNetworkReadMS=ui->spinQueryTimeout->value()*1000;
    eewSettings->showWindowOnStartup=ui->checkShowWindowOnStartup->isChecked();
    eewSettings->usingEEWDataSource=(EEW_DATA_SOURCE)ui->comboUsingEEWData->currentIndex();
    eewSettings->usingHistoryDataSource=(HISTORY_DATA_SOURCE)ui->comboUsingHistoryData->currentIndex();
    eewSettings->usingStationDataSource=(STATION_DATA_SOURCE)ui->comboUsingStation->currentIndex();
    eewSettings->debugUsingEEWDataSource=ui->checkDebugEEWData->isChecked();
    eewSettings->debugUsingHistoryDataSource=ui->checkDebugHistoryData->isChecked();
    eewSettings->debugUsingStationDataSource=ui->checkDebugStation->isChecked();
    eewSettings->showLogoOnMap=ui->checkShowLogoOnMap->isChecked();
    eewSettings->showEstimatedIntensity=ui->checkShowIntensityOnMap->isChecked();
    eewSettings->logToFile=ui->checkLogToFile->isChecked();
    eewSettings->clearLogsOnStartup=ui->checkClearLogsOnStart->isChecked();
    eewSettings->stationDataStoreTimeSec=ui->spinStationRecordLength->value();
    eewSettings->weakShakeIntensity=ui->spinWeakShakeIntensity->value();
    eewSettings->midShakeIntensity=ui->spinMidShakeIntensity->value();
    eewSettings->strongShakeIntensity=ui->spinStrongShakeIntensity->value();
    eewSettings->dontAutoSwitchTab=ui->checkDontAutoSwitchTab->isChecked();
    setAutorun(ui->checkAutorun->isChecked());
    eewSettings->showIntensityOnNewHistory=ui->checkShowNewRecordIntensity->isChecked();
    eewSettings->enableStationData=ui->checkEnableStationData->isChecked();
    eewSettings->pwaveSpeed=ui->spinPWaveSpeed->value();
    eewSettings->swaveSpeed=ui->spinSWaveSpeed->value();
    eewSettings->earthAvgRadius=ui->spinEarthRadius->value();
    eewSettings->SaveSettings();
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if(ui->buttonBox->standardButton(button)==QDialogButtonBox::RestoreDefaults&&
            QMessageBox::warning(this,button->text(),
                                 tr("Any settings will be lost, do you want to continue?"),
                                 QMessageBox::Yes|QMessageBox::No,QMessageBox::No)==QMessageBox::Yes){
        EEWSettings*eewSettings=EEWSettings::GetSettings();
        eewSettings->BuildDefaults();
        LoadSettings();
        accept();
    }
}

void SettingsDialog::on_buttonPathLocalAlert_clicked()
{
    QString path=QFileDialog::getOpenFileName(this,NULL,ui->buttonPathLocalAlert->text(),
                                              tr("Wave sounds (*.wav);;All files (*)"));
    if(!path.isNull())
        ui->buttonPathLocalAlert->setText(path);
}

void SettingsDialog::on_buttonPathLocalArrive_clicked()
{
    QString path=QFileDialog::getOpenFileName(this,NULL,ui->buttonPathLocalArrive->text(),
                                              tr("Wave sounds (*.wav);;All files (*)"));
    if(!path.isNull())
        ui->buttonPathLocalArrive->setText(path);
}

void SettingsDialog::on_buttonPathGlobalAlert_clicked()
{
    QString path=QFileDialog::getOpenFileName(this,NULL,ui->buttonPathGlobalAlert->text(),
                                              tr("Wave sounds (*.wav);;All files (*)"));
    if(!path.isNull())
        ui->buttonPathGlobalAlert->setText(path);
}

void SettingsDialog::on_buttonTestLocalAlert_clicked()
{
    EEWSounds::GetSounds()->TestPlayAlertSound(ui->buttonPathLocalAlert->text(),ui->spinRepeatLocalAlert->value());
}

void SettingsDialog::on_buttonTestLocalArrive_clicked()
{
    EEWSounds::GetSounds()->TestPlaySWaveArriveSound(ui->buttonPathLocalArrive->text(),ui->spinRepeatLocalArrive->value());
}

void SettingsDialog::on_buttonGlobalAlert_clicked()
{
    EEWSounds::GetSounds()->TestPlayNewRecordSound(ui->buttonPathGlobalAlert->text(),ui->spinRepeatGlobalAlert->value());
}

void SettingsDialog::on_buttonPathCriticalAlert_clicked()
{
    QString path=QFileDialog::getOpenFileName(this,NULL,ui->buttonPathCriticalAlert->text(),
                                              tr("Wave sounds (*.wav);;All files (*)"));
    if(!path.isNull())
        ui->buttonPathCriticalAlert->setText(path);
}

void SettingsDialog::on_buttonPathAlertUpdate_clicked()
{
    QString path=QFileDialog::getOpenFileName(this,NULL,ui->buttonPathAlertUpdate->text(),
                                              tr("Wave sounds (*.wav);;All files (*)"));
    if(!path.isNull())
        ui->buttonPathAlertUpdate->setText(path);
}

void SettingsDialog::on_buttonTestCriticalAlert_clicked()
{
    EEWSounds::GetSounds()->TestPlayCriticalAlertSound(ui->buttonPathCriticalAlert->text(),ui->spinRepeatCriticalAlert->value());
}

void SettingsDialog::on_buttonTestAlertUpdate_clicked()
{
    EEWSounds::GetSounds()->TestPlayUpdateAlertSound(ui->buttonPathAlertUpdate->text(),ui->spinRepeatAlertUpdate->value());
}

bool SettingsDialog::isAutorun()
{
#ifdef Q_OS_WIN
    QStringList pgList=QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    if(pgList.size()==0)
        return false;
    QString pg=pgList[0];
    pg.append("/Startup/eewcn.lnk");
    return QFile::exists(pg);
#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    //Check Ubuntu specific startup entry
    QStringList cfgList=QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    if(cfgList.size()==0)
        return false;
    QString cfg=cfgList[0];
    cfg.append("/autostart/eewcn.desktop");
    return QFile::exists(cfg);
#elif defined(Q_OS_MAC) && !defined(Q_OS_IOS)
    QDir appPath=QCoreApplication::applicationDirPath();
    appPath.cd("../..");
    MacLoginItems li;
    return li.isAutorun(appPath.absolutePath().toUtf8());
#else
    ui->checkAutorun->setEnabled(false);
    return false;
#endif
}

void SettingsDialog::setAutorun(bool enable)
{
#ifdef Q_OS_WIN
    QStringList pgList=QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    if(pgList.size()==0)
        return;
    QString pg=pgList[0];
    pg.append("/Startup/eewcn.lnk");
    if(enable){
        QFile cpath(__argv[0]);
        cpath.link(pg);
    }else{
        QFile::remove(pg);
    }
#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    //Ubuntu create startup entry
    QStringList cfgList=QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    if(cfgList.size()==0)
        return;
    QString cfg=cfgList[0];
    QDir d;
    d.mkpath(cfg+"/autostart");
    cfg.append("/autostart/eewcn.desktop");
    if(enable){
        QFile f(cfg);
        if(f.open(QIODevice::Text|QIODevice::WriteOnly)){
            QDir apppath(QCoreApplication::applicationDirPath());
            apppath.cdUp();
            QString txt="[Desktop Entry]\n"
                        "Name=EEW CN\n"
                        "Exec="+apppath.absolutePath()+"/eewcn.sh\n"
                        "Terminal=true\n"
                        "Type=Application\n"
                        "Categories=Application;\n"
                        "StartupNotify=true\n"
                        "Path="+apppath.absolutePath()+"\n";
            f.write(txt.toUtf8());
            f.close();
            txt="chmod +x "+cfg;
            system(txt.toUtf8());
        }
    }else {
        QFile::remove(cfg);
    }
#elif defined(Q_OS_MAC) && !defined(Q_OS_IOS)
    QDir appPath=QCoreApplication::applicationDirPath();
    appPath.cd("../..");
    MacLoginItems li;
    li.setAutorun(enable,appPath.absolutePath().toUtf8());
#endif
}

void SettingsDialog::ResizeWindow()
{
    QRect rDesktop=QApplication::primaryScreen()->availableGeometry();
    QSize sFrame=((QWidget*)parent())->frameSize(),sClient=((QWidget*)parent())->size();
    QRect rContent=ui->scrollAreaWidgetContents->rect();
    QRect rScroll=ui->scrollArea->contentsRect();
    QMargins mScroll=ui->scrollArea->contentsMargins();
    int dx=rContent.width()-rScroll.width()+ui->scrollArea->verticalScrollBar()->sizeHint().width()+mScroll.left()+mScroll.right();
    int dy=rContent.height()-rScroll.height()+mScroll.top()+mScroll.bottom();
    int newWidth=qMin(rDesktop.width()-sFrame.width()+sClient.width(),size().width()-sFrame.width()+sClient.width()+dx);
    int newHeight=qMin(rDesktop.height()-sFrame.height()+sClient.height(),size().height()+dy);
    QRect r;
    int newLeft=geometry().left()-(newWidth-width())/2;
    int newTop=geometry().top()-(newHeight-height())/2;
    r.setLeft(qMin(qMax(rDesktop.left(),newLeft),rDesktop.right()-newWidth));
    r.setTop(qMin(qMax(rDesktop.top(),newTop),rDesktop.bottom()-newHeight));
    r.setWidth(newWidth);
    r.setHeight(newHeight);
    setGeometry(r);
}

void SettingsDialog::SetEEWUsingCustom()
{
    ui->comboUsingEEWData->setItemText(ui->comboUsingEEWData->currentIndex(),tr("You are using custom scripts."));
    ui->comboUsingEEWData->setEnabled(false);
}

void SettingsDialog::SetHistoryUsingCustom()
{
    ui->comboUsingHistoryData->setItemText(ui->comboUsingHistoryData->currentIndex(),tr("You are using custom scripts."));
    ui->comboUsingHistoryData->setEnabled(false);
}

void SettingsDialog::SetStationUsingCustom()
{
    ui->comboUsingStation->setItemText(ui->comboUsingStation->currentIndex(),tr("You are using custom scripts."));
    ui->comboUsingStation->setEnabled(false);
}

void SettingsDialog::on_buttonPathWeakShakeSound_clicked()
{
    QString path=QFileDialog::getOpenFileName(this,NULL,ui->buttonPathWeakShakeSound->text(),
                                                tr("Wave sounds (*.wav);;All files (*)"));
    if(!path.isNull())
        ui->buttonPathWeakShakeSound->setText(path);
}


void SettingsDialog::on_buttonPathMidShakeSound_clicked()
{
    QString path=QFileDialog::getOpenFileName(this,NULL,ui->buttonPathMidShakeSound->text(),
                                                tr("Wave sounds (*.wav);;All files (*)"));
    if(!path.isNull())
        ui->buttonPathMidShakeSound->setText(path);
}


void SettingsDialog::on_buttonPathStrongShakeSound_clicked()
{
    QString path=QFileDialog::getOpenFileName(this,NULL,ui->buttonPathStrongShakeSound->text(),
                                                tr("Wave sounds (*.wav);;All files (*)"));
    if(!path.isNull())
        ui->buttonPathStrongShakeSound->setText(path);
}


void SettingsDialog::on_buttonTestWeakShakeSound_clicked()
{
    EEWSounds::GetSounds()->TestPlayWeakShakeSound(ui->buttonPathWeakShakeSound->text(),ui->spinRepeatsWeakShakeSound->value());
}


void SettingsDialog::on_buttonTestMidShakeSound_clicked()
{
    EEWSounds::GetSounds()->TestPlayMidShakeSound(ui->buttonPathMidShakeSound->text(),ui->spinRepeatsMidShakeSound->value());
}


void SettingsDialog::on_buttonTestStrongShakeSound_clicked()
{
    EEWSounds::GetSounds()->TestPlayStrongShakeSound(ui->buttonPathStrongShakeSound->text(),ui->spinRepeatsStrongShakeSound->value());
}

