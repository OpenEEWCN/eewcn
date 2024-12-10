#include "eewsettings.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>
#include <sstream>

#define KEY(x) QT_STRINGIFY(x)

#define SETTINGS_PATH "settings.json"

EEWSettings EEWSettings::eewSettings;

EEWSettings::EEWSettings()
{
    BuildDefaults();
}

EEWSettings::~EEWSettings()
{
}

EEWSettings*EEWSettings::GetSettings()
{
    return &eewSettings;
}

bool EEWSettings::LoadSettings()
{
    QString path=Value::getAppDataPath(SETTINGS_PATH,false);
    if(path.isEmpty())
        return false;
    return LoadFromJSONFile(path);
}

bool EEWSettings::SaveSettings()
{
    QString path=Value::getAppDataPath(SETTINGS_PATH);
    if(path.isEmpty())
        return false;
    return SaveToJSONFile(path);
}

void EEWSettings::BuildDefaults()
{
    minimumLocalReportLevel10x=0;
    minimumEpicenterReportLevel10x=0;
    minimumGlobalReportLevel10x=0;
    userLatitude=29.8;
    userLongitude=106.4;
    postURL="http://localhost/";
    eewQueryInterval=1;
    historyQueryInterval=5;
    stationQueryInterval=1;
    language=EEWCN_LANGUAGE::EEWCN_CHINESE_MAINLAND;
    pathAlertSound="Media/eewalert.wav";
    pathSWaveArriveSound="Media/swavearrive.wav";
    pathNewRecordSound="Media/newrecord.wav";
    pathCriticalAlertSound="Media/eewcritical.wav";
    pathUpdateAlertSound="Media/eewupdate.wav";
    pathWeakShakeSound="Media/weakshake.wav";
    pathMidShakeSound="Media/midshake.wav";
    pathStrongShakeSound="Media/strongshake.wav";
    repeatsAlertSound=1;
    repeatsNewRecordSound=1;
    repeatsSWaveArriveSound=1;
    repeatsCriticalAlertSound=1;
    repeatsUpdateAlertSound=1;
    repeatsWeakShakeSound=1;
    repeatsMidShakeSound=1;
    repeatsStrongShakeSound=1;
    criticalLevel10x=70;
    historyQueryCount=20;
    eewQueryCount=10;
    showWindowOnStartup=true;
    windowRect=QRect(0,0,0,0);
    windowMaximized=false;
    userExtra="";
    mapboxStyle="";
    mapboxAccessToken="";
    showHistoryList=true;
    showStatusBar=true;
    showLegends=true;
    usingEEWDataSource=EEW_DATA_SOURCE::EEW_DATA_SOURCE_WOLFX;
    usingHistoryDataSource=HISTORY_DATA_SOURCE::HISTORY_DATA_SOURCE_WOLFX;
    usingStationDataSource=STATION_DATA_SOURCE::STATION_DATA_SOURCE_WOLFX_LEAFPEACH_WEBSOCKET;
    debugUsingEEWDataSource=false;
    debugUsingHistoryDataSource=false;
    debugUsingStationDataSource=false;
    showLogoOnMap=false;
    showEstimatedIntensity=false;
    showIntensityOnNewHistory=false;
    timeoutNetworkReadMS=30000;
    timeoutDataDelayRestartSec=0;
    logToFile=true;
    clearLogsOnStartup=true;
    stationDataStoreTimeSec=120;
    weakShakeIntensity=2;
    midShakeIntensity=4;
    strongShakeIntensity=6;
    dontAutoSwitchTab=false;
    enableStationData=false;
    pwaveSpeed=7;//km/s
    swaveSpeed=4;//km/s
    earthAvgRadius=6371;//km
}

bool EEWSettings::LoadFromJSONString(QString str)
{
    QJsonParseError err;
    QJsonDocument doc=QJsonDocument::fromJson(str.toUtf8(),&err);
    if(err.error!=QJsonParseError::NoError)
        return false;
    QJsonObject root=doc.object();
    minimumLocalReportLevel10x=root.value(KEY(minimumLocalReportLevel10x)).toInt(minimumLocalReportLevel10x);
    minimumEpicenterReportLevel10x=root.value(KEY(minimumEpicenterReportLevel10x)).toInt(minimumEpicenterReportLevel10x);
    minimumGlobalReportLevel10x=root.value(KEY(minimumGlobalReportLevel10x)).toInt(minimumGlobalReportLevel10x);
    userLatitude=root.value(KEY(userLatitude)).toDouble(userLatitude);
    userLongitude=root.value(KEY(userLongitude)).toDouble(userLongitude);
    postURL=root.value(KEY(postURL)).toString(postURL);
    eewQueryInterval=root.value(KEY(eewQueryInterval)).toInt(eewQueryInterval);
    historyQueryInterval=root.value(KEY(historyQueryInterval)).toInt(historyQueryInterval);
    stationQueryInterval=root.value(KEY(pgaQueryInterval)).toInt(stationQueryInterval);
    language=(EEWCN_LANGUAGE)root.value(KEY(language)).toInt((int)language);
    pathAlertSound=root.value(KEY(pathAlertSound)).toString(pathAlertSound);
    pathSWaveArriveSound=root.value(KEY(pathSWaveArriveSound)).toString(pathSWaveArriveSound);
    pathNewRecordSound=root.value(KEY(pathNewRecordSound)).toString(pathNewRecordSound);
    pathCriticalAlertSound=root.value(KEY(pathCriticalAlertSound)).toString(pathCriticalAlertSound);
    pathUpdateAlertSound=root.value(KEY(pathUpdateAlertSound)).toString(pathUpdateAlertSound);
    pathWeakShakeSound=root.value(KEY(pathWeakShakeSound)).toString(pathWeakShakeSound);
    pathMidShakeSound=root.value(KEY(pathMidShakeSound)).toString(pathMidShakeSound);
    pathStrongShakeSound=root.value(KEY(pathStrongShakeSound)).toString(pathStrongShakeSound);
    repeatsAlertSound=root.value(KEY(repeatsAlertSound)).toInt(repeatsAlertSound);
    repeatsNewRecordSound=root.value(KEY(repeatsNewRecordSound)).toInt(repeatsNewRecordSound);
    repeatsSWaveArriveSound=root.value(KEY(repeatsSWaveArriveSound)).toInt(repeatsSWaveArriveSound);
    repeatsCriticalAlertSound=root.value(KEY(repeatsCriticalAlertSound)).toInt(repeatsCriticalAlertSound);
    repeatsUpdateAlertSound=root.value(KEY(repeatsUpdateAlertSound)).toInt(repeatsUpdateAlertSound);
    repeatsWeakShakeSound=root.value(KEY(repeatsWeakShakeSound)).toInt(repeatsWeakShakeSound);
    repeatsMidShakeSound=root.value(KEY(repeatsMidShakeSound)).toInt(repeatsMidShakeSound);
    repeatsStrongShakeSound=root.value(KEY(repeatsStrongShakeSound)).toInt(repeatsStrongShakeSound);
    criticalLevel10x=root.value(KEY(criticalLevel10x)).toInt(criticalLevel10x);
    historyQueryCount=root.value(KEY(historyQueryCount)).toInt(historyQueryCount);
    eewQueryCount=root.value(KEY(eewQueryCount)).toInt(eewQueryCount);
    showWindowOnStartup=root.value(KEY(showWindowOnStartup)).toBool(showWindowOnStartup);
    windowRect=StringToRect(root.value(KEY(windowRect)).toString(RectToString(windowRect)));
    windowMaximized=root.value(KEY(windowMaximized)).toBool(windowMaximized);
    userExtra=root.value(KEY(userExtra)).toString(userExtra);
    mapboxStyle=root.value(KEY(mapboxStyle)).toString(mapboxStyle);
    mapboxAccessToken=root.value(KEY(mapboxAccessToken)).toString(mapboxAccessToken);
    showHistoryList=root.value(KEY(showHistoryList)).toBool(showHistoryList);
    showStatusBar=root.value(KEY(showStatusBar)).toBool(showStatusBar);
    showLegends=root.value(KEY(showLegends)).toBool(showLegends);
    usingEEWDataSource=(EEW_DATA_SOURCE)root.value(KEY(usingEEWDataSource)).toInt((int)usingEEWDataSource);
    usingHistoryDataSource=(HISTORY_DATA_SOURCE)root.value(KEY(usingHistoryDataSource)).toInt((int)usingHistoryDataSource);
    usingStationDataSource=(STATION_DATA_SOURCE)root.value(KEY(usingStationDataSource)).toInt((int)usingStationDataSource);
    debugUsingEEWDataSource=root.value(KEY(debugUsingEEWDataSource)).toBool(debugUsingEEWDataSource);
    debugUsingHistoryDataSource=root.value(KEY(debugUsingHistoryDataSource)).toBool(debugUsingHistoryDataSource);
    debugUsingStationDataSource=root.value(KEY(debugUsingStationDataSource)).toBool(debugUsingStationDataSource);
    showLogoOnMap=root.value(KEY(showLogoOnMap)).toBool(showLogoOnMap);
    showEstimatedIntensity=root.value(KEY(showEstimatedIntensity)).toBool(showEstimatedIntensity);
    showIntensityOnNewHistory=root.value(KEY(showIntensityOnNewHistory)).toBool(showIntensityOnNewHistory);
    timeoutNetworkReadMS=root.value(KEY(timeoutNetworkReadMS)).toInt(timeoutNetworkReadMS);
    timeoutDataDelayRestartSec=root.value(KEY(timeoutDataDelayRestartSec)).toInt(timeoutDataDelayRestartSec);
    logToFile=root.value(KEY(logToFile)).toBool(logToFile);
    clearLogsOnStartup=root.value(KEY(clearLogsOnStartup)).toBool(clearLogsOnStartup);
    stationDataStoreTimeSec=root.value(KEY(stationDataStoreTimeSec)).toInt(stationDataStoreTimeSec);
    weakShakeIntensity=root.value(KEY(weakShakeIntensity)).toInt(weakShakeIntensity);
    midShakeIntensity=root.value(KEY(midShakeIntensity)).toInt(midShakeIntensity);
    strongShakeIntensity=root.value(KEY(strongShakeIntensity)).toInt(strongShakeIntensity);
    dontAutoSwitchTab=root.value(KEY(dontAutoSwitchTab)).toBool(dontAutoSwitchTab);
    enableStationData=root.value(KEY(enableStationData)).toBool(enableStationData);
    pwaveSpeed=root.value(KEY(pwaveSpeed)).toDouble(pwaveSpeed);
    swaveSpeed=root.value(KEY(swaveSpeed)).toDouble(swaveSpeed);
    earthAvgRadius=root.value(KEY(earthAvgRadius)).toDouble(earthAvgRadius);
    if(!userExtra.contains("showtesteew")){
        debugUsingEEWDataSource=false;
        debugUsingHistoryDataSource=false;
    }
    return true;
}

bool EEWSettings::LoadFromJSONFile(QString path)
{
    QFile f(path);
    if(!f.open(QFile::ReadOnly|QFile::Text))
        return false;
    QTextStream fin(&f);
    fin.setCodec("UTF-8");
    return LoadFromJSONString(fin.readAll());
}

QString EEWSettings::ToJSONString()
{
    QJsonObject root;
    root.insert(KEY(minimumLocalReportLevel10x),minimumLocalReportLevel10x);
    root.insert(KEY(minimumEpicenterReportLevel10x),minimumEpicenterReportLevel10x);
    root.insert(KEY(minimumGlobalReportLevel10x),minimumGlobalReportLevel10x);
    root.insert(KEY(userLatitude),userLatitude);
    root.insert(KEY(userLongitude),userLongitude);
    root.insert(KEY(postURL),postURL);
    root.insert(KEY(eewQueryInterval),eewQueryInterval);
    root.insert(KEY(historyQueryInterval),historyQueryInterval);
    root.insert(KEY(pgaQueryInterval),stationQueryInterval);
    root.insert(KEY(language),(int)language);
    root.insert(KEY(pathAlertSound),pathAlertSound);
    root.insert(KEY(pathSWaveArriveSound),pathSWaveArriveSound);
    root.insert(KEY(pathNewRecordSound),pathNewRecordSound);
    root.insert(KEY(pathCriticalAlertSound),pathCriticalAlertSound);
    root.insert(KEY(pathUpdateAlertSound),pathUpdateAlertSound);
    root.insert(KEY(pathWeakShakeSound),pathWeakShakeSound);
    root.insert(KEY(pathMidShakeSound),pathMidShakeSound);
    root.insert(KEY(pathStrongShakeSound),pathStrongShakeSound);
    root.insert(KEY(repeatsSWaveArriveSound),repeatsSWaveArriveSound);
    root.insert(KEY(repeatsAlertSound),repeatsAlertSound);
    root.insert(KEY(repeatsNewRecordSound),repeatsNewRecordSound);
    root.insert(KEY(repeatsCriticalAlertSound),repeatsCriticalAlertSound);
    root.insert(KEY(repeatsUpdateAlertSound),repeatsUpdateAlertSound);
    root.insert(KEY(repeatsWeakShakeSound),repeatsWeakShakeSound);
    root.insert(KEY(repeatsMidShakeSound),repeatsMidShakeSound);
    root.insert(KEY(repeatsStrongShakeSound),repeatsStrongShakeSound);
    root.insert(KEY(criticalLevel10x),criticalLevel10x);
    root.insert(KEY(historyQueryCount),historyQueryCount);
    root.insert(KEY(eewQueryCount),eewQueryCount);
    root.insert(KEY(showWindowOnStartup),showWindowOnStartup);
    root.insert(KEY(windowRect),RectToString(windowRect));
    root.insert(KEY(windowMaximized),windowMaximized);
    root.insert(KEY(userExtra),userExtra);
    root.insert(KEY(mapboxStyle),mapboxStyle);
    root.insert(KEY(mapboxAccessToken),mapboxAccessToken);
    root.insert(KEY(showHistoryList),showHistoryList);
    root.insert(KEY(showStatusBar),showStatusBar);
    root.insert(KEY(showLegends),showLegends);
    root.insert(KEY(usingEEWDataSource),(int)usingEEWDataSource);
    root.insert(KEY(usingHistoryDataSource),(int)usingHistoryDataSource);
    root.insert(KEY(usingStationDataSource),(int)usingStationDataSource);
    root.insert(KEY(debugUsingEEWDataSource),debugUsingEEWDataSource);
    root.insert(KEY(debugUsingHistoryDataSource),debugUsingHistoryDataSource);
    root.insert(KEY(debugUsingStationDataSource),debugUsingStationDataSource);
    root.insert(KEY(showLogoOnMap),showLogoOnMap);
    root.insert(KEY(showEstimatedIntensity),showEstimatedIntensity);
    root.insert(KEY(showIntensityOnNewHistory),showIntensityOnNewHistory);
    root.insert(KEY(timeoutNetworkReadMS),timeoutNetworkReadMS);
    root.insert(KEY(timeoutDataDelayRestartSec),timeoutDataDelayRestartSec);
    root.insert(KEY(logToFile),logToFile);
    root.insert(KEY(clearLogsOnStartup),clearLogsOnStartup);
    root.insert(KEY(stationDataStoreTimeSec),stationDataStoreTimeSec);
    root.insert(KEY(weakShakeIntensity),weakShakeIntensity);
    root.insert(KEY(midShakeIntensity),midShakeIntensity);
    root.insert(KEY(strongShakeIntensity),strongShakeIntensity);
    root.insert(KEY(dontAutoSwitchTab),dontAutoSwitchTab);
    root.insert(KEY(enableStationData),enableStationData);
    root.insert(KEY(pwaveSpeed),pwaveSpeed);
    root.insert(KEY(swaveSpeed),swaveSpeed);
    root.insert(KEY(earthAvgRadius),earthAvgRadius);
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

bool EEWSettings::SaveToJSONFile(QString path)
{
    QFile f(path);
    if(!f.open(QFile::WriteOnly|QFile::Text|QFile::Truncate))
        return false;
    QTextStream fout(&f);
    fout.setCodec("UTF-8");
    fout<<ToJSONString();
    return true;
}

QString EEWSettings::RectToString(const QRect &r)
{
    return QString::asprintf("%d %d %d %d",r.left(),r.top(),r.width(),r.height());
}

QRect EEWSettings::StringToRect(const QString &s)
{
    int x,y,w,h;
    std::istringstream(s.toStdString())>>x>>y>>w>>h;
    return QRect(x,y,w,h);
}
