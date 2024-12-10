#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "eewsettings.h"
#include "eewsounds.h"
#include "earthquakelistentryform.h"
#include "testeewdialog.h"
#include "sethomedialog.h"
#include "value.h"
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QCloseEvent>
#include <QQuickItem>
#include <QProcess>
#include <QPainter>
#include <QScreen>
#include <QtPositioning/QGeoPolygon>
#include <algorithm>
#include <QTranslator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), timerIdActiveEEW(0),
    statusBarEEWIndex(0), isRunning(true), lastEEWDataTimestamp(0),
    lastHistoryDataTimestamp(0), mapCenterUsingBy(0)
{
    ui->setupUi(this);
    contextMenusWindow=new ContextMenusWindow(this);
    setWindowTitle(windowTitle()+" "+VERSION_NAME);
    InitEEW();
}

MainWindow::~MainWindow()
{
    ReleaseEEW();
    delete ui;
}

void MainWindow::SetHome(double lat, double lng)
{
    lastEEWDoc=QJsonDocument();
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"setHome",Q_ARG(QVariant,lat),
                              Q_ARG(QVariant,lng));
}

void MainWindow::InitEEW()
{
    eewSettings=EEWSettings::GetSettings();
    bool loadSettingsOk=eewSettings->LoadSettings();
    logger=Logger::getLogger();
    logger->setWriteToFile(eewSettings->logToFile);
    logger->setStatusBar(statusBar());
    if(eewSettings->clearLogsOnStartup&&!logger->clearLogFile())
        logger->error(tr("Cannot delete old logs."));
    if(!loadSettingsOk)
        logger->error(tr("Failed to load settings."));
    logger->info(tr("App started."));

    trayIcon=new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon("Media/eewcn.ico"));
    trayIcon->setToolTip(windowTitle());
    trayIcon->setContextMenu(contextMenusWindow->TrayMenu());//必须加上这句才能在Ubuntu上正确显示
    trayIcon->show();
    connect(trayIcon,&QSystemTrayIcon::activated,this,&MainWindow::TrayIconActivated);
    connect(trayIcon,&QSystemTrayIcon::messageClicked,this,&MainWindow::TrayMessageClicked);

    //检查OpenSSL版本号，若无法发送HTTPS请求需要下载OpenSSL库
    //MSVC:
    //https://wiki.openssl.org/index.php/Binaries（使用firedaemon网站的）
    //MinGW:
    //https://mirrors.tuna.tsinghua.edu.cn/msys2/mingw/mingw32/mingw-w64-i686-openssl-1.1.1.s-1-any.pkg.tar.zst
    //https://mirrors.tuna.tsinghua.edu.cn/msys2/mingw/mingw64/mingw-w64-x86_64-openssl-1.1.1.s-1-any.pkg.tar.zst
    logger->info(QSslSocket::sslLibraryBuildVersionString());

    Value::PWAVE_SPEED=eewSettings->pwaveSpeed;
    Value::SWAVE_SPEED=eewSettings->swaveSpeed;
    Value::EARTH_AVG_RADIUS=eewSettings->earthAvgRadius;

    nam=new QNetworkAccessManager(this);
    //nam->setTransferTimeout(eewSettings->timeoutNetworkReadMS);
    connect(nam,&QNetworkAccessManager::finished,this,&MainWindow::OnNetworkReply);
    nam->get(QNetworkRequest(QUrl(QString("https://OpenEEWCN.github.io/eewcn-res/qml/%1.qm").arg(Value::langRegionTag[(int)eewSettings->language]))));

    timerEEW=new QTimer(this);
    timerHistory=new QTimer(this);
    timerMapCenterUsingByHistory=new QTimer(this);
    timerUpdateAllStation=new QTimer(this);

#ifdef QT_DEBUG
    ui->quickMapView->setSource(QUrl::fromLocalFile("eewcn-res/qml/mapview.qml"));
#endif
    if (ui->quickMapView->status()==QQuickWidget::Ready){
        logger->info(tr("Loaded QML: %1").arg(ui->quickMapView->source().toString()));
        InitEEWAfterQMLLoaded();
    }else{
        connect(ui->quickMapView,&QQuickWidget::statusChanged,this,[this](QQuickWidget::Status status){
            if (status==QQuickWidget::Ready){
                logger->info(tr("Loaded QML: %1").arg(ui->quickMapView->source().toString()));
                InitEEWAfterQMLLoaded();
            }else if(status==QQuickWidget::Error){
                foreach (auto&e, ui->quickMapView->errors()) {
                    logger->error(e.toString());
                }
            }
        });
    }
}

void MainWindow::InitEEWAfterQMLLoaded(){
    QString mapboxAccessToken=eewSettings->mapboxAccessToken;
    QString mapboxStyle=eewSettings->mapboxStyle;
    if(mapboxAccessToken.isEmpty())
        mapboxAccessToken=EEWCN_MAPBOX_ACCESS_TOKEN;
    if(mapboxStyle.isEmpty())
        mapboxStyle=EEWCN_MAPBOX_STYLE;
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"setMapboxParam",
                              Q_ARG(QVariant,mapboxAccessToken),Q_ARG(QVariant,mapboxStyle));
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"setLabelLanguage",
                              Q_ARG(QVariant,(int)eewSettings->language));
    SetHome(eewSettings->userLatitude,eewSettings->userLongitude);
    connect(contextMenusWindow,&ContextMenusWindow::SignalSettingsChanged,this,[this](){
        Value::PWAVE_SPEED=eewSettings->pwaveSpeed;
        Value::SWAVE_SPEED=eewSettings->swaveSpeed;
        Value::EARTH_AVG_RADIUS=eewSettings->earthAvgRadius;
        SetHome(eewSettings->userLatitude,eewSettings->userLongitude);
        QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"setMapboxLogoVisible",Q_ARG(QVariant,eewSettings->showLogoOnMap));
        RestartStation();
    });
    contextMenusWindow->SetActionShowLegendsChecked(eewSettings->showLegends);
    SetShowLegends(eewSettings->showLegends);
    SetShowHistoryWindow(eewSettings->showHistoryList);
    contextMenusWindow->SetActionShowStatusBarChecked(eewSettings->showStatusBar);
    SetShowStatusBar(eewSettings->showStatusBar);
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"setMapboxLogoVisible",Q_ARG(QVariant,eewSettings->showLogoOnMap));
    if(!eewSettings->userExtra.contains("debuginfo")){
        ui->textDebug->setVisible(false);
        ui->quickMapView->rootObject()->findChild<QObject*>("debugText")->setProperty("visible",false);
    }
    GeoPolygon::Load("geodata",geoPolygons);
    logger->info(tr("Loaded %1 polygons.").arg(geoPolygons.size()));
    connect(this,&MainWindow::SignalRebuildNAM,this,&MainWindow::RebuildNAM);

    timerEEW->setSingleShot(true);
    timerHistory->setSingleShot(true);
    timerMapCenterUsingByHistory->setSingleShot(true);
    timerUpdateAllStation->setSingleShot(true);
    connect(timerEEW,&QTimer::timeout,this,&MainWindow::timerTask);
    connect(timerHistory,&QTimer::timeout,this,&MainWindow::timerTask);
    connect(timerMapCenterUsingByHistory,&QTimer::timeout,this,&MainWindow::timerTask);
    connect(timerUpdateAllStation,&QTimer::timeout,this,&MainWindow::timerTask);
    if(QGuiApplication::primaryScreen()->logicalDotsPerInch()==120)//125%缩放处理
        resizeDocks({ui->dockWidget},{260*5/4},Qt::Horizontal);
    else
        resizeDocks({ui->dockWidget},{260},Qt::Horizontal);
    OnCheckDataSourceValid();
    StartQuery(true);
    timestampAppStartSec=QDateTime::currentSecsSinceEpoch();
#if defined( Q_OS_ANDROID ) || defined( Q_OS_IOS )
    ui->quickMapView->installEventFilter(this);
    ui->listHistory->viewport()->installEventFilter(this);
    ui->listEEW->viewport()->installEventFilter(this);
#endif
}

void MainWindow::GetWolfxSeisList()
{
    logger->info(tr("Querying seis_list..."));
    QNetworkRequest req(QUrl("https://api.wolfx.jp/seis_list.json"));
    nam->get(req);
}

void MainWindow::StartStation(bool getdatafirst)
{
    QStringList builtinStation[]={
        {},//WS URL
        {},//GET URL
        {"http://localhost/station.json"}
    };
    /*if(!cds.IsStationAvailable()&&wolfxSeisList.empty()&&eewSettings->enableStationData){
        GetWolfxSeisList();
        return;
    }*/
    foreach (const WOLFX_SEIS&e, wolfxSeisList) {
        builtinStation[0].push_back(e.wsURL);
        builtinStation[1].push_back(e.getURL);
    }
    QStringList usingStations=eewSettings->debugUsingStationDataSource?builtinStation[2]:
        builtinStation[(int)eewSettings->usingStationDataSource];

    StationData::dataStoreTimeMS=eewSettings->stationDataStoreTimeSec*1000.0;
    stationDataList.clear();
    stationDataList.resize(usingStations.size());
    stationURLIndexDict.clear();
    for(int i=0;i<usingStations.size();i++){
        stationURLIndexDict.insert(usingStations[i],i);
        //WebSocket
        stationDataList[i].webSocket=new QWebSocket(QString(),QWebSocketProtocol::VersionLatest,this);
        stationDataList[i].webSocket->setProperty("index",i);
        stationDataList[i].index=i;
        stationDataList[i].url=usingStations[i];
        stationDataList[i].timer=new QTimer(this);
        stationDataList[i].timer->setProperty("index",i);
        stationDataList[i].timer->setSingleShot(true);
        connect(stationDataList[i].timer,&QTimer::timeout,this,&MainWindow::stationTimerTask);
        connect(stationDataList[i].webSocket,&QWebSocket::connected,this,[this,i](){
            logger->info(tr("Station[%1] WebSocket connected.").arg(i));
            stationDataList[i].timer->start(eewSettings->stationQueryInterval*1000);
            const QString&post="";//不需要初始查询
            if(post.length()>0)
                stationDataList[i].webSocket->sendTextMessage(post);
        });
        connect(stationDataList[i].webSocket,&QWebSocket::disconnected,this,[this,i,usingStations](){
            logger->info(tr("Station[%1] WebSocket disconnected.").arg(i));
            if(isRunning){
                QTimer::singleShot(eewSettings->stationQueryInterval*1000,this,[this,i,usingStations](){
                    logger->info(tr("Station[%1] WebSocket reconnecting...").arg(i));
                    stationDataList[i].webSocket->open(usingStations[i]);
                });
            }
        });
        connect(stationDataList[i].webSocket,QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),this,[this,i](QAbstractSocket::SocketError e){
            logger->error(tr("Station[%1] WebSocket error: %2").arg(i).arg((int)e));
        });
        connect(stationDataList[i].webSocket,&QWebSocket::pong,this,&MainWindow::OnWSStationPong);
        connect(stationDataList[i].webSocket,&QWebSocket::textMessageReceived,this,&MainWindow::OnWSStationReply);
        if(eewSettings->enableStationData){
            if(eewSettings->usingStationDataSource==STATION_DATA_SOURCE::STATION_DATA_SOURCE_WOLFX_LEAFPEACH_WEBSOCKET)
                stationDataList[i].webSocket->open(usingStations[i]);
            else if(getdatafirst)
                OnQueryStation(i);
            else
                stationDataList[i].timer->start(eewSettings->stationQueryInterval*1000);
        }
    }
    timerUpdateAllStation->start(1000);
}

void MainWindow::StopStation()
{
    //停止WS和Timer
    for(int i=0;i<stationDataList.size();i++){
        stationDataList[i].timer->disconnect();
        stationDataList[i].webSocket->disconnect();
        stationDataList[i].timer->stop();
        stationDataList[i].webSocket->close();
        stationDataList[i].timer->deleteLater();
        stationDataList[i].webSocket->deleteLater();
    }
    stationDataList.clear();
    timerUpdateAllStation->stop();
}

void MainWindow::RestartStation()
{
    StopStation();
    StartStation(false);
}

void MainWindow::StoreStationData(int index,const QJsonDocument& doc)
{
    stationDataList[index].AddStationData(doc);
    stationDataList[index].RemoveOutdatedData();
}

void MainWindow::OnUpdateAllStation()
{
    ShowAllStationDataOnMap();
    ShowAllStationDataOnList();
    ReportAllStationData();
    timerUpdateAllStation->start(1000);
}

void MainWindow::timerTask()
{
    QObject*s=sender();
    if(s==timerEEW)
        OnQueryEEW();
    else if(s==timerHistory)
        OnQueryHistory();
    else if(s==timerMapCenterUsingByHistory)
        mapCenterUsingBy=mapCenterUsingBy&(~2);
    else if(s==timerUpdateAllStation)
        OnUpdateAllStation();
}

void MainWindow::stationTimerTask()
{
    OnQueryStation(sender()->property("index").toInt());
}

void MainWindow::ReleaseEEW()
{
    EndQuery();
    if(!eewSettings->SaveSettings())
        logger->error(tr("Failed to save settings."));
    logger->info(tr("App ended."));
}

void MainWindow::StartQuery(bool getdatafirst)
{
    if(getdatafirst)
        OnQueryEEW();
    else
        timerEEW->start(eewSettings->eewQueryInterval*1000);
    if(getdatafirst)
        OnQueryHistory();
    else
        timerHistory->start(eewSettings->historyQueryInterval*1000);
    StartStation(getdatafirst);
    logger->info(tr("Timer started."));
}

void MainWindow::EndQuery()
{
    timerEEW->stop();
    timerHistory->stop();
    StopStation();
    logger->info(tr("Timer stopped."));
}

void MainWindow::OnNetworkReply(QNetworkReply*reply)
{
    QNetworkRequest req=reply->request();
    QString reqPath=req.url().path();
    QJsonParseError err;
    if(reqPath.compare("/sc_eew.json")==0){
        //Wolfx地震预警（转换成ICL格式）
        if(reply->error()!=QNetworkReply::NoError){
            logger->error(tr("Query EEW Error: %1").arg(reply->error()));
        }else{
            QJsonDocument docWolfx=QJsonDocument::fromJson(reply->readAll(),&err);
            if(err.error!=QJsonParseError::NoError){
                logger->error(tr("EEW JSON Error: %1").arg(err.error));
            }else{
                //将Wolfx的数据格式转换成与ICL兼容的
                const QJsonObject&oWolfx=docWolfx.object();
                QJsonObject oData;
                const QString &eventId=oWolfx.value("EventID").toString();//"20240209040929.0001_1"
                oData.insert("eventId",Value::chineseDateTimeFormatToTimestampMS(eventId.leftRef(4)+"-"+
                                                                                  eventId.midRef(4,2)+"-"+
                                                                                  eventId.midRef(6,2)+" "+
                                                                                  eventId.midRef(8,2)+":"+
                                                                                  eventId.midRef(10,2)+":"+
                                                                                  eventId.midRef(12,2))/1000);
                oData.insert("updates",oWolfx.value("ReportNum").toInt());
                oData.insert("latitude",oWolfx.value("Latitude").toDouble());
                oData.insert("longitude",oWolfx.value("Longitude").toDouble());
                oData.insert("depth",oWolfx.value("Depth").toDouble());
                oData.insert("epicenter",oWolfx.value("HypoCenter").toString());
                oData.insert("startAt",Value::chineseDateTimeFormatToTimestampMS(oWolfx.value("OriginTime").toString()));
                oData.insert("magnitude",oWolfx.value("Magunitude").toDouble());
                QJsonArray aData;
                aData.push_back(oData);
                QJsonObject rData;
                rData.insert("data",aData);
                QJsonDocument doc;
                doc.setObject(rData);
                ShowEEWDataOnMap(doc);
                ShowEEWDataOnList(doc);
                ReportEEWData(doc);
                lastEEWDoc=doc;
            }
        }
        timerEEW->start(eewSettings->eewQueryInterval*1000);
    }else if(reqPath.compare("/cenc_eqlist.json")==0){
        //Wolfx地震记录（转换成台网网站格式）
        if(reply->error()!=QNetworkReply::NoError){
            logger->error(tr("Query History Error: %1").arg(reply->error()));
        }else{
            QJsonDocument docWolfx=QJsonDocument::fromJson(reply->readAll(),&err);
            if(err.error!=QJsonParseError::NoError){
                logger->error(tr("History JSON Error: %1").arg(err.error));
            }else{
                //将Wolfx的数据格式转换成与CENC兼容的
                QJsonArray aData;
                for(int i=1;i<=50;i++){
                    const QJsonObject&oWolfx=docWolfx.object().value(QString::asprintf("No%d",i)).toObject();
                    QJsonObject oData;
                    bool isAuto=oWolfx.value("type").toString().compare("automatic")==0;
                    int st=Value::chineseDateTimeFormatToTimestampMS(oWolfx.value("time").toString())/1000;
                    oData.insert("id",QString::asprintf("%d",st));
                    oData.insert("O_TIME",oWolfx.value("time").toString());
                    oData.insert("EPI_LAT",oWolfx.value("latitude").toString());
                    oData.insert("EPI_LON",oWolfx.value("longitude").toString());
                    oData.insert("EPI_DEPTH",oWolfx.value("depth").toString().toInt());
                    oData.insert("AUTO_FLAG",isAuto?tr("(AUTO)"):"M");
                    oData.insert("EQ_TYPE","M");
                    oData.insert("M",oWolfx.value("magnitude").toString());
                    oData.insert("LOCATION_C",oWolfx.value("location").toString());
                    aData.push_back(oData);
                }
                QJsonObject rData;
                rData.insert("shuju",aData);
                QJsonDocument doc;
                doc.setObject(rData);
                ShowHistoryDataOnMap(doc);
                ShowHistoryDataOnList(doc);
                ReportHistoryData(doc);
                lastHistoryDoc=doc;
            }
        }
        timerHistory->start(eewSettings->historyQueryInterval*1000);
    }else if(reqPath.compare("/v1/earlywarnings")==0){
        //ICL
        if(reply->error()!=QNetworkReply::NoError){
            logger->error(tr("Query EEW Error: %1").arg(reply->error()));
        }else{
            QJsonDocument doc=QJsonDocument::fromJson(reply->readAll(),&err);
            if(err.error!=QJsonParseError::NoError){
                logger->error(tr("EEW JSON Error: %1").arg(err.error));
            }else{
                ShowEEWDataOnMap(doc);
                ShowEEWDataOnList(doc);
                ReportEEWData(doc);
                lastEEWDoc=doc;
            }
        }
        timerEEW->start(eewSettings->eewQueryInterval*1000);
    }else if(reqPath.compare("/ajax/speedsearch")==0){
        //中国地震台网网站
        if(reply->error()!=QNetworkReply::NoError){
            logger->error(tr("Query History Error: %1").arg(reply->error()));
        }else{
            QByteArray responseData=reply->readAll();
            responseData.remove(0,1);
            responseData.remove(responseData.size()-1,1);
            QJsonDocument doc=QJsonDocument::fromJson(responseData,&err);
            if(err.error!=QJsonParseError::NoError){
                logger->error(tr("History JSON Error: %1").arg(err.error));
            }else{
                ShowHistoryDataOnMap(doc);
                ShowHistoryDataOnList(doc);
                ReportHistoryData(doc);
                lastHistoryDoc=doc;
            }
        }
        timerHistory->start(eewSettings->historyQueryInterval*1000);
    }else if(reqPath.compare("/api.htm")==0){
        //台网APP（转换成台网网站格式，注意这个是倒序的）
        if(reply->error()!=QNetworkReply::NoError){
            logger->error(tr("Query History Error: %1").arg(reply->error()));
        }else{
            QByteArray responseData=reply->readAll();
            QJsonDocument docApp=QJsonDocument::fromJson(responseData,&err);
            if(err.error!=QJsonParseError::NoError){
                logger->error(tr("History JSON Error: %1").arg(err.error));
            }else{
                //{"result":"OK","values":[
                //{"time":1680780147000,"longitude":122.8,"latitude":39.63,
                //"depth":10000.0,"updateTime":1680780431000,"url":"www.ceic.ac.cn",
                //"ring":false,"eqid":"AUAU.20230406192254.V.001","loc_name":"辽宁大连市庄河市",
                //"mag":3.2,"eq_type":"M","loc_province":"辽宁"},...]}
                const QJsonArray&appValues=docApp.object().value("values").toArray();
                QJsonArray aData;
                for(int i=0;i<appValues.size();i++){
                    const QJsonObject&appObj=appValues[i].toObject();
                    QJsonObject oData;
                    double mst=appObj.value("time").toDouble();
                    const QString &eqid=appObj.value("eqid").toString();
                    oData.insert("id",QString::asprintf("%lld",Value::chineseDateTimeFormatToTimestampMS(eqid.midRef(5,4)+"-"+
                                                                                                         eqid.midRef(9,2)+"-"+
                                                                                                         eqid.midRef(11,2)+" "+
                                                                                                         eqid.midRef(13,2)+":"+
                                                                                                         eqid.midRef(15,2)+":"+
                                                                                                         eqid.midRef(17,2))/1000));
                    oData.insert("O_TIME",Value::timestampMSToChineseDateTimeFormat(mst));
                    oData.insert("EPI_LAT",QString("%1").arg(appObj.value("latitude").toDouble()));
                    oData.insert("EPI_LON",QString("%1").arg(appObj.value("longitude").toDouble()));
                    oData.insert("EPI_DEPTH",appObj.value("depth").toDouble()/1000);
                    oData.insert("AUTO_FLAG","M");
                    oData.insert("EQ_TYPE","M");
                    oData.insert("M",QString("%1").arg(appObj.value("mag").toDouble()));
                    oData.insert("LOCATION_C",appObj.value("loc_name").toString());
                    aData.insert(0,oData);
                }
                QJsonObject rData;
                rData.insert("shuju",aData);
                QJsonDocument doc;
                doc.setObject(rData);
                ShowHistoryDataOnMap(doc);
                ShowHistoryDataOnList(doc);
                ReportHistoryData(doc);
                lastHistoryDoc=doc;
            }
        }
        timerHistory->start(eewSettings->historyQueryInterval*1000);
    }else if(reqPath.compare("/api/earlywarning/jsonPageList")==0){
        //四川地震局（转换成ICL格式）
        if(reply->error()!=QNetworkReply::NoError){
            logger->error(tr("Query EEW Error: %1").arg(reply->error()));
        }else{
            QByteArray responseData=reply->readAll();
            QJsonDocument docOriginal=QJsonDocument::fromJson(responseData,&err);
            if(err.error!=QJsonParseError::NoError){
                logger->error(tr("EEW JSON Error: %1").arg(err.error));
            }else{
                const QJsonArray&appValues=docOriginal.object().value("data").toArray();
                QJsonArray aData;
                for(int i=0;i<appValues.size();i++){
                    const QJsonObject&appObj=appValues[i].toObject();
                    QJsonObject oData;
                    const QString &eventId=appObj.value("eventId").toString();//"20240209040929.0001_1"
                    oData.insert("eventId",Value::chineseDateTimeFormatToTimestampMS(eventId.leftRef(4)+"-"+
                                                                                      eventId.midRef(4,2)+"-"+
                                                                                      eventId.midRef(6,2)+" "+
                                                                                      eventId.midRef(8,2)+":"+
                                                                                      eventId.midRef(10,2)+":"+
                                                                                      eventId.midRef(12,2))/1000);
                    oData.insert("updates",appObj.value("list").toArray().size());
                    oData.insert("startAt",appObj.value("shockTime").toDouble());
                    oData.insert("latitude",appObj.value("latitude").toDouble());
                    oData.insert("longitude",appObj.value("longitude").toDouble());
                    oData.insert("depth",0.0);
                    oData.insert("magnitude",appObj.value("magnitude").toDouble());
                    oData.insert("epicenter",appObj.value("placeName").toString());
                    aData.push_back(oData);
                }
                QJsonObject rData;
                rData.insert("data",aData);
                QJsonDocument doc;
                doc.setObject(rData);
                ShowEEWDataOnMap(doc);
                ShowEEWDataOnList(doc);
                ReportEEWData(doc);
                lastEEWDoc=doc;
            }
        }
        timerEEW->start(eewSettings->eewQueryInterval*1000);
    }else if(reqPath.compare("/earthquakeWarn/bulletin/list.json")==0){
        //福建地震局（转换成台网网站格式）
        if(reply->error()!=QNetworkReply::NoError){
            logger->error(tr("Query History Error: %1").arg(reply->error()));
        }else{
            QByteArray responseData=reply->readAll();
            QJsonDocument docOriginal=QJsonDocument::fromJson(responseData,&err);
            if(err.error!=QJsonParseError::NoError){
                logger->error(tr("History JSON Error: %1").arg(err.error));
            }else{
                const QJsonArray&appValues=docOriginal.object().value("list").toArray();
                QJsonArray aData;
                for(int i=0;i<appValues.size();i++){
                    const QJsonObject&appObj=appValues[i].toObject();
                    QJsonObject oData;
                    QString af=appObj.value("autoFlag").toString();
                    oData.insert("id",QString::asprintf("%d",appObj.value("id").toInt()));
                    oData.insert("O_TIME",appObj.value("shockTime").toString());
                    oData.insert("EPI_LAT",QString("%1").arg(appObj.value("latitude").toDouble()));
                    oData.insert("EPI_LON",QString("%1").arg(appObj.value("longitude").toDouble()));
                    oData.insert("EPI_DEPTH",appObj.value("depth").toDouble());
                    oData.insert("AUTO_FLAG",af.compare("I")==0?"M":af);
                    oData.insert("EQ_TYPE","M");
                    oData.insert("M",QString("%1").arg(appObj.value("magnitude").toDouble()));
                    QString loc=appObj.value("placeName").toString();
                    QString infoTypeName=appObj.value("infoTypeName").toString();
                    if(infoTypeName.compare("[正式测定]"))
                        loc.append(infoTypeName);
                    oData.insert("LOCATION_C",loc);
                    aData.push_back(oData);
                }
                QJsonObject rData;
                rData.insert("shuju",aData);
                QJsonDocument doc;
                doc.setObject(rData);
                ShowHistoryDataOnMap(doc);
                ShowHistoryDataOnList(doc);
                ReportHistoryData(doc);
                lastHistoryDoc=doc;
            }
        }
        timerHistory->start(eewSettings->historyQueryInterval*1000);
    }else if(stationURLIndexDict.value(req.url().toString(),-1)>=0){
        int urlIndex=stationURLIndexDict[req.url().toString()];
        if(reply->error()!=QNetworkReply::NoError){
            logger->error(tr("Query Station[%1] Error: %2").arg(urlIndex).arg(reply->error()));
        }else{
            QByteArray responseData=reply->readAll();
            QJsonDocument docOriginal=QJsonDocument::fromJson(responseData,&err);
            if(err.error!=QJsonParseError::NoError){
                logger->error(tr("Station[%1] JSON Error: %2").arg(urlIndex).arg(err.error));
            }else{
                QJsonDocument doc;
                doc.setObject(ConvertWolfxStationData(docOriginal.object(),urlIndex));
                StoreStationData(urlIndex,doc);
            }
        }
        stationDataList[urlIndex].timer->start(eewSettings->stationQueryInterval*1000);
    }else if(reqPath.compare("/seis_list.json")==0){
        if(reply->error()!=QNetworkReply::NoError){
            logger->error(tr("Get seis_list Error: %1").arg(reply->error()));
        }else{
            QJsonDocument doc=QJsonDocument::fromJson(reply->readAll(),&err);
            if(err.error!=QJsonParseError::NoError){
                logger->error(tr("seis_list JSON Error: %1").arg(err.error));
            }else{
                const QJsonObject &obj=doc.object();
                QString listStr=doc.toJson();
                logger->info(tr("Received seis_list: %1").arg(listStr));
                foreach (QString id, obj.keys()) {
                    const QJsonObject &seis=obj.value(id).toObject();
                    if(seis.value("enable").toBool()){
                        QString name=seis.value("location").toString();
                        QString note=seis.value("note").toString();
                        if(note.length()>0)
                            name.append(" (").append(note).append(")");
                        wolfxSeisList.push_back({
                            "wss://seis.wolfx.jp/"+id,
                            "https://api.wolfx.jp/"+id+".json",
                            name,
                            seis.value("longitude").toDouble(),
                            seis.value("latitude").toDouble(),
                            seis.value("elevation").toDouble()
                        });
                    }
                }
            }
        }
        QTimer::singleShot(eewSettings->stationQueryInterval,this,[this](){StartStation(true);});
    }else if(reqPath.compare("/ntp.json")==0){
        OnCheckSystemTimeValid(reply);
    }else if(reqPath.endsWith(".qm")){
        if(reply->error()!=QNetworkReply::NoError){
            logger->error(tr("Load translation error: %1").arg(reply->error()));
        }else{
            QByteArray ba=reply->readAll();
            qmlTranslationData.resize(ba.length());
            memcpy(qmlTranslationData.data(),ba.data(),ba.length());
            QTranslator *t=new QTranslator(this);//注意Translator必须长期存在
            if(t->load(qmlTranslationData.data(),qmlTranslationData.length())){
                QApplication::installTranslator(t);//即使QML已经加载，它也会让QML自动切换翻译
            }
        }
    }
    reply->deleteLater();//根据Qt文档，必须调用deleteLater
}

void MainWindow::OnWSStationReply(const QString &msg)
{
    int i=sender()->property("index").toInt();
    int errorCode=0;
    if(msg.compare("pong")==0)
        return;
    QJsonObject obj;
    QJsonParseError err;
    QJsonDocument docOriginal=QJsonDocument::fromJson(msg.toUtf8(),&err);
    if(err.error!=QJsonParseError::NoError){
        logger->error(tr("Station[%1] JSON Error: %2").arg(i).arg(err.error));
    }else{
        obj=ConvertWolfxStationData(docOriginal.object(),i);
        QJsonDocument doc;
        doc.setObject(obj);
        //只更新到数据存储中，不显示在界面上
        StoreStationData(i,doc);
    }
}

QJsonObject MainWindow::ConvertWolfxStationData(const QJsonObject &obj,int index)
{
    QJsonObject converted;
    QJsonArray arr;
    QJsonObject stdata;
    stdata.insert("name",index>=wolfxSeisList.length()?"":wolfxSeisList[index].name);
    stdata.insert("longitude",index>=wolfxSeisList.length()?0.0:wolfxSeisList[index].longitude);
    stdata.insert("latitude",index>=wolfxSeisList.length()?0.0:wolfxSeisList[index].latitude);
    stdata.insert("height",index>=wolfxSeisList.length()?0.0:wolfxSeisList[index].height);
    stdata.insert("pga",obj.value("PGA").toDouble());
    stdata.insert("pgv",obj.value("PGV").toDouble());
    stdata.insert("pgd",obj.value("PGD").toDouble());
    stdata.insert("intensity",(int)obj.value("Intensity").toDouble());
    stdata.insert("update",Value::chineseDateTimeFormatToTimestampMS(obj.value("update_at").toString()));
    arr.append(stdata);
    converted.insert("data",arr);
    return converted;
}

void MainWindow::OnWSStationPong()
{
    int i=sender()->property("index").toInt();
    stationDataList[i].timer->start(eewSettings->stationQueryInterval*1000);
}

void MainWindow::OnQueryEEW()
{
    //先检查数据是否超过重启时间
    if(lastEEWDataTimestamp>0&&eewSettings->timeoutDataDelayRestartSec>0&&
            lastEEWDataTimestamp+eewSettings->timeoutDataDelayRestartSec<QDateTime::currentSecsSinceEpoch()){
        logger->info(tr("EEW timeout, resetting connection..."));
        lastEEWDataTimestamp=QDateTime::currentSecsSinceEpoch();
        ResetNetworkConnection();
        return;
    }
    QNetworkRequest req;
    QUrl url;
    switch(eewSettings->usingEEWDataSource){
    case EEW_DATA_SOURCE::EEW_DATA_SOURCE_WOLFX:default:url.setUrl("https://api.wolfx.jp/sc_eew.json");break;
    case EEW_DATA_SOURCE::EEW_DATA_SOURCE_SICHUANJU:url.setUrl(QString("http://118.113.105.29:8002/api/earlywarning/jsonPageList?orderType=1&pageNo=1&pageSize=%1&userLat=0.0&userLng=0.0").arg(eewSettings->eewQueryCount));break;
    }
    if(eewSettings->debugUsingEEWDataSource){
        url.setScheme("http");
        url.setHost("localhost");
    }
    req.setUrl(url);
    SetReplyTimeout(nam->get(req));
    debugInspect();
}

void MainWindow::OnQueryHistory()
{
    //先检查数据是否超过重启时间
    if(lastHistoryDataTimestamp>0&&eewSettings->timeoutDataDelayRestartSec>0&&
            lastHistoryDataTimestamp+eewSettings->timeoutDataDelayRestartSec<QDateTime::currentSecsSinceEpoch()){
        logger->info(tr("History timeout, resetting connection..."));
        lastHistoryDataTimestamp=QDateTime::currentSecsSinceEpoch();
        ResetNetworkConnection();
        return;
    }
    QNetworkRequest req;
    QUrl url;
    switch(eewSettings->usingHistoryDataSource){
    case HISTORY_DATA_SOURCE::HISTORY_DATA_SOURCE_WOLFX:default:url.setUrl("https://api.wolfx.jp/cenc_eqlist.json");break;
    case HISTORY_DATA_SOURCE::HISTORY_DATA_SOURCE_CENCWEB:url.setUrl("https://www.ceic.ac.cn/ajax/speedsearch?num=6");break;
    case HISTORY_DATA_SOURCE::HISTORY_DATA_SOURCE_CENCAPP:url.setUrl("http://api.dizhensubao.igexin.com/api.htm");break;
    case HISTORY_DATA_SOURCE::HISTORY_DATA_SOURCE_FUJIANJU:url.setUrl(QString("http://218.5.2.111:9088/earthquakeWarn/bulletin/list.json?pageSize=%1").arg(eewSettings->historyQueryCount));break;
    }
    if(eewSettings->debugUsingHistoryDataSource){
        url.setScheme("http");
        url.setHost("localhost");
    }
    req.setUrl(url);
    if(eewSettings->usingHistoryDataSource==HISTORY_DATA_SOURCE::HISTORY_DATA_SOURCE_CENCAPP)
        SetReplyTimeout(nam->post(req,"{\"action\":\"requestMonitorDataAction\",\"startTime\":\"0\",\"dataSource\":\"CEIC\"}"));
    else
        SetReplyTimeout(nam->get(req));
}

void MainWindow::OnQueryStation(int i)
{
    //先检查数据是否超过重启时间
    if(stationDataList[i].dataStore.size()>0&&stationDataList[i].dataStore[0].timeElapsedData.size()>0&&
        stationDataList[i].dataStore[0].timeElapsedData.back().timestamp_ms>0&&eewSettings->timeoutDataDelayRestartSec>0&&
        stationDataList[i].dataStore[0].timeElapsedData.back().timestamp_ms+eewSettings->timeoutDataDelayRestartSec<QDateTime::currentSecsSinceEpoch()){
        logger->info(tr("Station timeout, resetting connection..."));
        stationDataList[i].dataStore[0].timeElapsedData.back().timestamp_ms=QDateTime::currentSecsSinceEpoch();
        ResetNetworkConnection();
        return;
    }
    QNetworkRequest req;
    QUrl url;
    //看是GET还是WS
    QString lourl=stationDataList[i].url.toLower();
    if(lourl.startsWith("ws:")||lourl.startsWith("wss:")){
        stationDataList[i].webSocket->ping();
    }else{
        url.setUrl(stationDataList[i].url);
        req.setUrl(url);
        SetReplyTimeout(nam->get(req));
    }
}

void MainWindow::ShowEEWDataOnMap(const QJsonDocument &doc)
{
    QQuickItem*mapView=ui->quickMapView->rootObject();
    const QJsonArray &eq=doc.object().value("data").toArray();
    if(lastEEWDoc.isEmpty()||lastEEWDoc!=doc){
        if(!eewUpdated(doc)){
            logger->info(tr("EEW data are changing but same."));
            return;
        }
        QString jsonReceive=doc.toJson(),jsonLast=lastEEWDoc.toJson();
        logger->info(tr("Put EEW marks...\nReceiving:%1\nPrevious:%2").arg(jsonReceive,jsonLast));
        //只会在数据变化时执行
        //将历史数据显示在地图上
        QMetaObject::invokeMethod(mapView,"clearEEWMarks");
        for(int i=eq.size()-1;i>=0;i--){
            const QJsonObject&e=eq[i].toObject();
            QMetaObject::invokeMethod(mapView,"putEEWMark",
                                      Q_ARG(QVariant,e.value("eventId").toInt()),
                                      Q_ARG(QVariant,e.value("latitude").toDouble()),
                                      Q_ARG(QVariant,e.value("longitude").toDouble()),
                                      Q_ARG(QVariant,Value::calcMaxInt(e.value("magnitude").toDouble(),
                                                                       e.value("depth").toDouble(),
                                                                       e.value("longitude").toDouble(),
                                                                       e.value("latitude").toDouble())));
        }
        if(eq.size()>0&&activeEEW.empty()){
            const QJsonObject&e=eq[0].toObject();
            SetMapCenterTo(e.value("latitude").toDouble(),e.value("longitude").toDouble());
        }
    }
}

void MainWindow::ShowEEWDataOnList(const QJsonDocument &doc)
{
    const QJsonArray &eq=doc.object().value("data").toArray();
    if(lastEEWDoc.isEmpty()||lastEEWDoc!=doc){
        if(!eewUpdated(doc))
            return;
        logger->info(tr("Refresh EEW list..."));
        for(int i=0;i<ui->listEEW->count();i++){
            QListWidgetItem*item=ui->listEEW->item(i);
            QWidget*entry=ui->listEEW->itemWidget(item);
            delete item;
            delete entry;
        }
        ui->listEEW->clear();
        for(int i=0;i<eq.size();i++){
            EarthquakeListEntryForm *entry=new EarthquakeListEntryForm(this);
            QListWidgetItem *item=new QListWidgetItem(ui->listEEW);
            ui->listEEW->addItem(item);
            ui->listEEW->setItemWidget(item,entry);
            entry->SetHead(i==0);
            //显示数据
            const QJsonObject&e=eq[i].toObject();
            QString location=e.value("epicenter").toString();
            location.append(QString(" #%1").arg(e.value("updates").toInt()));
            entry->SetData(location,Value::timestampMSToChineseDateTimeFormat(e.value("startAt").toDouble()),
                           e.value("magnitude").toDouble(),e.value("depth").toDouble(),
                           e.value("latitude").toDouble(),e.value("longitude").toDouble(),
                           ui->quickMapView->rootObject());
            connect(entry,&EarthquakeListEntryForm::SignalSetMapCenter,this,[this](double lat,double lng){
                SetMapCenterTo(lat,lng);
            });
            connect(entry,&EarthquakeListEntryForm::SignalShowIntensityOnMap,this,[this](double lat,double lng,double mag,double depth){
                ShowIntensityOnMap(lat,lng,mag,depth);
            });
        }
        ui->listEEW->ResizeItems();
        if(!lastEEWDoc.isEmpty()&&!eewSettings->dontAutoSwitchTab)
            ui->tabWidget->setCurrentIndex(1);
    }
}

void MainWindow::ReportEEWData(const QJsonDocument &doc){
    const QJsonArray &eq=doc.object().value("data").toArray();
    //对新增的EEW发送声音和通知
    qint64 msts=QDateTime::currentMSecsSinceEpoch();
    lastEEWDataTimestamp=msts/1000;
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"updateEEWTime",Q_ARG(QVariant,msts),
                              Q_ARG(QVariant,Value::timestampMSToChineseDateTimeFormat(msts)));
    if(activeEEW.empty()){
        const QJsonObject&e=eq[0].toObject();
        statusBar()->showMessage(tr("Latest EEW: %1, %2, M%3, %4km, Data updated at %5, Running Time: %6")
                                     .arg(Value::timestampMSToChineseDateTimeFormat(e.value("startAt").toDouble()))
                                     .arg(e.value("epicenter").toString())
                                     .arg(e.value("magnitude").toDouble())
                                     .arg(e.value("depth").toDouble())
                                     .arg(Value::timestampMSToChineseDateTimeFormat(msts).mid(11))
                                     .arg(Value::toTimeLengthHMS(QDateTime::currentSecsSinceEpoch()-timestampAppStartSec)));
    }
    for(int i=eq.size()-1;i>=0;i--){
        AddActiveEEW(eq[i].toObject());
    }
}

void MainWindow::AddActiveEEW(const QJsonObject &_e)
{
    QJsonObject e=_e;
    qint64 timestampNowMS=QDateTime::currentMSecsSinceEpoch();
    double eventId=e.value("eventId").toDouble();
    //对所有记录符合时间和距离条件的，列入警报
    //条件：
    //计算地震波是否已传播到最大距离
    //若是则略过，否则纳入警报
    double epiLat=e.value("latitude").toDouble();
    double epiLng=e.value("longitude").toDouble();
    double dep=e.value("depth").toDouble();
    double mag=e.value("magnitude").toDouble();
    e.insert("maxDistanceSwave",Value::maxDistanceSWaveSpread(mag,dep,epiLng,epiLat));
    double localInt=Value::getIntensityAt(mag,dep,epiLng,epiLat,eewSettings->userLongitude,eewSettings->userLatitude);
    double epiInt=Value::calcMaxInt(mag,dep,epiLng,epiLat);
    int activeEEWIndex=FindActiveEEWIndex(eventId);
    if(Value::distanceSWaveSpreadMS(dep,timestampNowMS-e.value("startAt").toDouble())<e.value("maxDistanceSwave").toDouble()
            //因为通常EEW也就1个所以直接这样遍历也没什么问题
            &&activeEEWIndex==-1&&(localInt*10>=eewSettings->minimumLocalReportLevel10x||epiInt*10>=eewSettings->minimumEpicenterReportLevel10x)){
        logger->info(tr("Active EEW:").append(QJsonDocument(e).toJson()));
        if(activeEEW.empty())
            timerIdActiveEEW=startTimer(50);
        mapCenterUsingBy=mapCenterUsingBy|1;
        show();
        activateWindow();
        raise();
        e.insert("maxIntensity",epiInt);
        activeEEW.push_back(e);
        CheckGeoIntensity();
        //播放声音
        if(epiInt*10>=eewSettings->criticalLevel10x)
            EEWSounds::GetSounds()->PlayCriticalAlertSound();
        else
            EEWSounds::GetSounds()->PlayAlertSound();
        //显示系统通知
        QString loc=e.value("epicenter").toString();
        QString timeStr=Value::timestampMSToChineseDateTimeFormat(e.value("startAt").toDouble());
        int distance=(int)Value::getDistanceSurface(epiLng,epiLat,eewSettings->userLongitude,eewSettings->userLatitude);
        trayIcon->showMessage(tr("EEW: %1 M%2 #%3").arg(loc).arg(mag).arg(e.value("updates").toInt()),
                              tr("%1\n%2, M%3\nDepth: %4km, Distance: %5km\nEpi.Int.: %6, Local Int.: %7")
                              .arg(timeStr,loc).arg(mag).arg(dep).arg(distance).arg(epiInt).arg(localInt),MakeIntensityIcon(epiInt));
        //发送POST消息
        if(!eewSettings->postURL.isEmpty()){
            QNetworkRequest req(QUrl(eewSettings->postURL));
            nam->post(req,QJsonDocument(e).toJson());
        }
    }
    //若出现对已有测定的修正该怎么处理？
    if(activeEEWIndex!=-1&&e.value("updates")!=activeEEW[activeEEWIndex].value("updates")){
        logger->info(tr("Active EEW Update:").append(QJsonDocument(e).toJson()));
        show();
        activateWindow();
        raise();
        double previousInt=activeEEW[activeEEWIndex].value("maxIntensity").toDouble(0.0);
        if(activeEEW[activeEEWIndex].value("playedSound").toBool(false)==true)
            e.insert("playedSound",true);
        e.insert("maxIntensity",qMax(previousInt,epiInt));
        activeEEW[activeEEWIndex]=e;
        CheckGeoIntensity();
        //播放声音
        if(previousInt*10<eewSettings->criticalLevel10x&&epiInt*10>=eewSettings->criticalLevel10x)
            EEWSounds::GetSounds()->PlayCriticalAlertSound();
        else
            EEWSounds::GetSounds()->PlayUpdateAlertSound();
        //显示系统通知
        QString loc=e.value("epicenter").toString();
        QString timeStr=Value::timestampMSToChineseDateTimeFormat(e.value("startAt").toDouble());
        int distance=(int)Value::getDistanceSurface(epiLng,epiLat,eewSettings->userLongitude,eewSettings->userLatitude);
        trayIcon->showMessage(tr("EEW: %1 M%2 #%3").arg(loc).arg(mag).arg(e.value("updates").toInt()),
                              tr("%1\n%2, M%3\nDepth: %4km, Distance: %5km\nEpi.Int.: %6, Local Int.: %7")
                              .arg(timeStr,loc).arg(mag).arg(dep).arg(distance).arg(epiInt).arg(localInt),MakeIntensityIcon(epiInt));
        //发送POST消息
        if(!eewSettings->postURL.isEmpty()){
            QNetworkRequest req(QUrl(eewSettings->postURL));
            nam->post(req,QJsonDocument(e).toJson());
        }
    }
}

int MainWindow::FindActiveEEWIndex(double eventId)
{
    for(int i=0;i<activeEEW.size();i++){
        if(activeEEW[i].value("eventId").toDouble()==eventId)
            return i;
    }
    return -1;
}

void MainWindow::ShowHistoryDataOnMap(const QJsonDocument &doc)
{
    //地震台网记录，判断方法：CATA_ID属性
    //https://www.ceic.ac.cn/ajax/speedsearch?num=6&&page=1
    //{"shuju":[
    //{"id":"44762","CATA_ID":"CD20230419082211.00","SAVE_TIME":"2023-04-19 08:28:46","O_TIME":"2023-04-19 08:22:10",
    //"EPI_LAT":"40.77","EPI_LON":"105.44","EPI_DEPTH":10,"AUTO_FLAG":"M","EQ_TYPE":"M","O_TIME_FRA":"0",
    //"M":"3.3","M_MS":"0","M_MS7":"0","M_ML":"0","M_MB":"0","M_MB2":"0","SUM_STN":"0","LOC_STN":"0",
    //"LOCATION_C":"\u5185\u8499\u53e4\u963f\u62c9\u5584\u76df\u963f\u62c9\u5584\u5de6\u65d7","LOCATION_S":"","CATA_TYPE":"",
    //"SYNC_TIME":"2023-04-19 08:28:46","IS_DEL":"","EQ_CATA_TYPE":"","NEW_DID":"CD20230419082211"},
    //...],...}
    QQuickItem*mapView=ui->quickMapView->rootObject();
    const QJsonArray &eq=doc.object().value("shuju").toArray();
    if(lastHistoryDoc.isEmpty()||lastHistoryDoc!=doc){
        if(!historyUpdated(doc)){
            logger->info(tr("History data are changing but same."));
            return;
        }
        logger->info(tr("Put History marks..."));
        //只会在数据变化时执行
        //将历史数据显示在地图上
        //TempShowWindowIfHidden();
        QMetaObject::invokeMethod(mapView,"clearHistoryMarks");
        for(int i=eq.size()-1;i>=0;i--){
            const QJsonObject&e=eq[i].toObject();
            QMetaObject::invokeMethod(mapView,"putHistoryMark",Q_ARG(QVariant,e.value("EPI_LAT").toString().toDouble()),
                                      Q_ARG(QVariant,e.value("EPI_LON").toString().toDouble()),
                                      Q_ARG(QVariant,Value::calcMaxInt(e.value(e.value("EQ_TYPE").toString()).toString().toDouble(),e.value("EPI_DEPTH").toDouble(),
                                        e.value("EPI_LON").toString().toDouble(),e.value("EPI_LAT").toString().toDouble())));
        }
        QMetaObject::invokeMethod(mapView,"refreshMarks");
        if(mapCenterUsingBy&1){
            QVector<int>vNew;
            historyUpdated(doc,&vNew);
            foreach(int iNew,vNew){
                QMetaObject::invokeMethod(mapView,"setHistoryMarkVisible",Q_ARG(QVariant,eq.size()-1-iNew),
                                          Q_ARG(QVariant,true));
            }
            mapCenterUsingBy=mapCenterUsingBy|2;
            timerMapCenterUsingByHistory->start(5000);
        }
        //TempHideWindowIfHidden();
        if(!lastHistoryDoc.isEmpty()&&eq.size()>0){
            const QJsonObject&e=eq[0].toObject();
            bool ok=true;
            double lat=e.value("EPI_LAT").toString().toDouble(&ok);
            if(!ok)
                return;
            double lon=e.value("EPI_LON").toString().toDouble(&ok);
            if(!ok)
                return;
            if(lat==0.0&&lon==0.0)
                return;
            if(eewSettings->showIntensityOnNewHistory)
                ShowIntensityOnMap(lat,lon,
                                   e.value(e.value("EQ_TYPE").toString()).toString().toDouble(),e.value("EPI_DEPTH").toDouble(),true);
            else
                SetMapCenterTo(lat,lon);
        }
    }
}

void MainWindow::ShowHistoryDataOnList(const QJsonDocument &doc)
{
    const QJsonArray &eq=doc.object().value("shuju").toArray();
    if(lastHistoryDoc.isEmpty()||lastHistoryDoc!=doc){
        if(!historyUpdated(doc))
            return;
        logger->info(tr("Refresh History list..."));
        //TempShowWindowIfHidden();
        for(int i=0;i<ui->listHistory->count();i++){
            QListWidgetItem*item=ui->listHistory->item(i);
            QWidget*entry=ui->listHistory->itemWidget(item);
            delete item;
            delete entry;
        }
        ui->listHistory->clear();
        for(int i=0;i<eq.size();i++){
            EarthquakeListEntryForm *entry=new EarthquakeListEntryForm(this);
            QListWidgetItem *item=new QListWidgetItem(ui->listHistory);
            ui->listHistory->addItem(item);
            ui->listHistory->setItemWidget(item,entry);
            entry->SetHead(i==0);
            //显示数据
            const QJsonObject&e=eq[i].toObject();
            QString location=e.value("LOCATION_C").toString();
            if(e.value("AUTO_FLAG").toString().compare("M")!=0)
                location+=" "+e.value("AUTO_FLAG").toString();
            entry->SetData(location,e.value("O_TIME").toString(),
                           e.value(e.value("EQ_TYPE").toString()).toString().toDouble(),
                           e.value("EPI_DEPTH").toDouble(),e.value("EPI_LAT").toString().toDouble(),
                           e.value("EPI_LON").toString().toDouble(),ui->quickMapView->rootObject());
            connect(entry,&EarthquakeListEntryForm::SignalSetMapCenter,this,[this](double lat,double lng){
                SetMapCenterTo(lat,lng);
            });
            connect(entry,&EarthquakeListEntryForm::SignalShowIntensityOnMap,this,[this](double lat,double lng,double mag,double depth){
                ShowIntensityOnMap(lat,lng,mag,depth);
            });
        }
        ui->listHistory->ResizeItems();
        if(!eewSettings->dontAutoSwitchTab)
            ui->tabWidget->setCurrentIndex(0);
        //TempHideWindowIfHidden();
    }
}

void MainWindow::ReportHistoryData(const QJsonDocument &doc){
    lastHistoryDataTimestamp=QDateTime::currentSecsSinceEpoch();
    if(lastHistoryDoc==doc)
        return;
    const QJsonArray&eq=doc.object().value("shuju").toArray();
    const QJsonArray&lastEq=lastHistoryDoc.object().value("shuju").toArray();
    QMap<int,QJsonObject>lastEqMap;
    for(int i=0;i<lastEq.size();i++)
        lastEqMap.insert(lastEq[i].toObject().value("id").toString().toInt(),lastEq[i].toObject());
    for(int i=eq.size()-1;i>=0;i--){
        const QJsonObject&e=eq[i].toObject();
        int id=e.value("id").toString().toInt();
        double mag=e.value(e.value("EQ_TYPE").toString()).toString().toDouble();
        double dep=e.value("EPI_DEPTH").toDouble();
        double lng=e.value("EPI_LON").toString().toDouble();
        double lat=e.value("EPI_LAT").toString().toDouble();
        bool isAuto=e.value("AUTO_FLAG").toString().compare("M")!=0;
        //对所有未处理过的记录
        if(lastEqMap.find(id)==lastEqMap.end()||lastEqMap[id].value("AUTO_FLAG").toString().compare("M")!=0){
            if((!lastHistoryDoc.isEmpty()||(i==0&&eewSettings->showWindowOnStartup))&&
                    Value::calcMaxInt(mag,dep,lng,lat)*10>=eewSettings->minimumGlobalReportLevel10x){
                logger->info(tr("Report new record:").append(QJsonDocument(e).toJson()));
                //播放声音
                EEWSounds::GetSounds()->PlayNewRecordSound();
                //显示系统通知
                QString location=e.value("LOCATION_C").toString();
                if(isAuto)
                    location+=" "+e.value("AUTO_FLAG").toString();
                trayIcon->showMessage(tr("Earthquake Information"),
                                      tr("Time: %1\nLocation: %2\nMagnitude: %3, Depth: %4km")
                                      .arg(e.value("O_TIME").toString(),location)
                                          .arg(mag).arg(dep),MakeIntensityIcon(Value::calcMaxInt(mag,dep,lng,lat)));
                //发送POST消息
                if(!eewSettings->postURL.isEmpty()){
                    QNetworkRequest req(QUrl(eewSettings->postURL));
                    nam->post(req,QJsonDocument(e).toJson());
                }
            }
        }
    }
}

void MainWindow::ShowAllStationDataOnMap()
{
    int count=0;
    int i,j;
    for(j=0;j<stationDataList.size();j++){
        for(i=0;i<stationDataList[j].dataStore.size();i++){
            count++;
        }
    }
    if(!isHidden())
        QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"resizeStationItemsCount",Q_ARG(QVariant,count));
    count=0;
    int shakingCount=0;
    for(j=0;j<stationDataList.size();j++){
        for(i=0;i<stationDataList[j].dataStore.size();i++){
            double intensity=stationDataList[j].dataStore[i].GetMaxIntensityFromTimeElapse();
            if(intensity>=eewSettings->weakShakeIntensity)
                shakingCount++;
            if(!isHidden())
                QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"setStationItem",
                                          Q_ARG(QVariant,count),Q_ARG(QVariant,stationDataList[j].dataStore[i].longitude),
                                          Q_ARG(QVariant,stationDataList[j].dataStore[i].latitude),
                                          Q_ARG(QVariant,intensity),
                                          Q_ARG(QVariant,stationDataList[j].dataStore[i].name));
            count++;
        }
    }
    if(activeEEW.empty()&&shakingCount>0){
#if !defined(Q_OS_ANDROID)&&!defined(Q_OS_IOS)
        show();
        activateWindow();
        raise();
#endif
        QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"clearGeoItems");
        QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"focusShakingStation",Q_ARG(QVariant,eewSettings->weakShakeIntensity),
                                  Q_ARG(QVariant,7));
    }
}

void MainWindow::ShowAllStationDataOnList()
{
    QVector<int>sortList;
    for(int j=0;j<stationDataList.size();j++){
        for(int i=0;i<stationDataList[j].dataStore.size();i++){
            QListWidgetItem*item=ui->listStation->item(sortList.size());
            if(item==nullptr){
                EarthquakeListEntryForm*entry=new EarthquakeListEntryForm(this);
                item=new QListWidgetItem(ui->listStation);
                ui->listStation->addItem(item);
                ui->listStation->setItemWidget(item,entry);
                entry->SetHead(true);
                connect(entry,&EarthquakeListEntryForm::SignalSetMapCenter,this,[this](double lat,double lng){
                    SetMapCenterTo(lat,lng);
                });
            }
            sortList.push_back((j<<16)|(i&0xFFFF));
        }
    }
    std::sort(sortList.begin(),sortList.end(),[this](int a,int b){
        double ia,ib;
        ia=stationDataList[(a>>16)&0xFFFF].dataStore[a&0xFFFF].GetMaxIntensityFromTimeElapse();
        ib=stationDataList[(b>>16)&0xFFFF].dataStore[b&0xFFFF].GetMaxIntensityFromTimeElapse();
        return ia>ib;
    });
    //显示数据
    for(int i=0;i<sortList.size();i++){
        EarthquakeListEntryForm*entry=(EarthquakeListEntryForm*)ui->listStation->itemWidget(ui->listStation->item(i));
        StationData::DataStore&ds=stationDataList[(sortList[i]>>16)&0xFFFF].dataStore[sortList[i]&0xFFFF];
        StationData::DataStore::TimeElapsedData&lastted=ds.timeElapsedData.back();
        double intensity=ds.GetMaxIntensityFromTimeElapse();
        entry->SetStationData(ds.name,
                              Value::timestampMSToChineseDateTimeFormat(ds.timeElapsedData.back().timestamp_ms),
                              lastted.pga,lastted.pgv,lastted.pgd,intensity,
                              ds.height,ds.latitude,ds.longitude,ui->quickMapView->rootObject());
        if(!eewSettings->dontAutoSwitchTab&&intensity>=eewSettings->weakShakeIntensity&&activeEEW.empty()&&i==0){
            ui->tabWidget->setCurrentIndex(2);
            ui->listStation->scrollToTop();
        }
    }
    ui->listStation->ResizeItems();
    //删除多余的列表项
    while(ui->listStation->count()>sortList.size()){
        QListWidgetItem*item=ui->listStation->takeItem(sortList.size());
        QWidget*entry=ui->listStation->itemWidget(item);
        delete item;
        delete entry;
    }
}

void MainWindow::ReportAllStationData()
{
    if(activeEEW.size()>0)
        return;
    //播放声音，POST
    double maxIntensity=0.0;
    QVector<int>strongJ,strongI,midJ,midI,weakJ,weakI;
    for(int j=0;j<stationDataList.size();j++){
        for(int i=0;i<stationDataList[j].dataStore.size();i++){
            double intensity=stationDataList[j].dataStore[i].timeElapsedData.back().intensity;
            maxIntensity=qMax(intensity,maxIntensity);
            if(intensity>=(double)eewSettings->strongShakeIntensity){
                strongJ.push_back(j);
                strongI.push_back(i);
            }else if(intensity>=(double)eewSettings->midShakeIntensity){
                midJ.push_back(j);
                midI.push_back(i);
            }else if(intensity>=(double)eewSettings->weakShakeIntensity){
                weakJ.push_back(j);
                weakI.push_back(i);
            }
        }
    }
    if(maxIntensity>=(double)eewSettings->strongShakeIntensity)
        EEWSounds::GetSounds()->PlayStrongShakeSound();
    else if(maxIntensity>=(double)eewSettings->midShakeIntensity)
        EEWSounds::GetSounds()->PlayMidShakeSound();
    else if(maxIntensity>=(double)eewSettings->weakShakeIntensity)
        EEWSounds::GetSounds()->PlayWeakShakeSound();
    else
        return;

    if(!eewSettings->postURL.isEmpty()){
        QJsonObject dataObj;
        QJsonArray arr;
        int i;
        for(i=0;i<strongJ.size();i++){
            QJsonObject ds;
            ds.insert("name",stationDataList[strongJ[i]].dataStore[strongI[i]].name);
            ds.insert("longitude",stationDataList[strongJ[i]].dataStore[strongI[i]].longitude);
            ds.insert("latitude",stationDataList[strongJ[i]].dataStore[strongI[i]].latitude);
            ds.insert("height",stationDataList[strongJ[i]].dataStore[strongI[i]].height);
            ds.insert("pga",stationDataList[strongJ[i]].dataStore[strongI[i]].timeElapsedData.back().pga);
            ds.insert("pgv",stationDataList[strongJ[i]].dataStore[strongI[i]].timeElapsedData.back().pgv);
            ds.insert("pgd",stationDataList[strongJ[i]].dataStore[strongI[i]].timeElapsedData.back().pgd);
            ds.insert("intensity",stationDataList[strongJ[i]].dataStore[strongI[i]].timeElapsedData.back().intensity);
            ds.insert("update",stationDataList[strongJ[i]].dataStore[strongI[i]].timeElapsedData.back().timestamp_ms);
            arr.push_back(ds);
        }
        for(i=0;i<midJ.size();i++){
            QJsonObject ds;
            ds.insert("name",stationDataList[midJ[i]].dataStore[midI[i]].name);
            ds.insert("longitude",stationDataList[midJ[i]].dataStore[midI[i]].longitude);
            ds.insert("latitude",stationDataList[midJ[i]].dataStore[midI[i]].latitude);
            ds.insert("height",stationDataList[midJ[i]].dataStore[midI[i]].height);
            ds.insert("pga",stationDataList[midJ[i]].dataStore[midI[i]].timeElapsedData.back().pga);
            ds.insert("pgv",stationDataList[midJ[i]].dataStore[midI[i]].timeElapsedData.back().pgv);
            ds.insert("pgd",stationDataList[midJ[i]].dataStore[midI[i]].timeElapsedData.back().pgd);
            ds.insert("intensity",stationDataList[midJ[i]].dataStore[midI[i]].timeElapsedData.back().intensity);
            ds.insert("update",stationDataList[midJ[i]].dataStore[midI[i]].timeElapsedData.back().timestamp_ms);
            arr.push_back(ds);
        }
        for(i=0;i<weakJ.size();i++){
            QJsonObject ds;
            ds.insert("name",stationDataList[weakJ[i]].dataStore[weakI[i]].name);
            ds.insert("longitude",stationDataList[weakJ[i]].dataStore[weakI[i]].longitude);
            ds.insert("latitude",stationDataList[weakJ[i]].dataStore[weakI[i]].latitude);
            ds.insert("height",stationDataList[weakJ[i]].dataStore[weakI[i]].height);
            ds.insert("pga",stationDataList[weakJ[i]].dataStore[weakI[i]].timeElapsedData.back().pga);
            ds.insert("pgv",stationDataList[weakJ[i]].dataStore[weakI[i]].timeElapsedData.back().pgv);
            ds.insert("pgd",stationDataList[weakJ[i]].dataStore[weakI[i]].timeElapsedData.back().pgd);
            ds.insert("intensity",stationDataList[weakJ[i]].dataStore[weakI[i]].timeElapsedData.back().intensity);
            ds.insert("update",stationDataList[weakJ[i]].dataStore[weakI[i]].timeElapsedData.back().timestamp_ms);
            arr.push_back(ds);
        }
        dataObj.insert("data",arr);

        QNetworkRequest req(QUrl(eewSettings->postURL));
        nam->post(req,QJsonDocument(dataObj).toJson());
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
#if defined( Q_OS_ANDROID )||defined( Q_OS_IOS )
    isRunning=false;
#endif
    if(isRunning){
        hide();
        event->ignore();
    }else{
        event->accept();
        eewSettings->windowMaximized=windowState()&Qt::WindowMaximized;
        if(!eewSettings->windowMaximized)
            eewSettings->windowRect=geometry();
    }
}

void MainWindow::TrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    //通过SetContextMenu设置菜单后就不用监听该事件了
    //case QSystemTrayIcon::Context:contextMenusWindow->TrackTrayMenu();break;
    case QSystemTrayIcon::Trigger:show();break;
    default:break;
    }
}

void MainWindow::TrayMessageClicked()
{
    show();
}

void MainWindow::on_dockWidget_visibilityChanged(bool visible)
{
    contextMenusWindow->SetActionShowHistoryChecked(visible);
    if(isRunning&&ui->dockWidget->parentWidget()->isVisible()){
        eewSettings->showHistoryList=visible;
        eewSettings->SaveSettings();
    }
}

void MainWindow::SetShowHistoryWindow(bool show)
{
    ui->dockWidget->setVisible(show);
}

void MainWindow::SetShowStatusBar(bool show)
{
    ui->statusbar->setVisible(show);
    eewSettings->showStatusBar=show;
}

void MainWindow::on_frameMapView_customContextMenuRequested(const QPoint &pos)
{
    ExitViewIntensity();
    contextMenusWindow->TrackWindowMenu(ui->frameMapView->mapToGlobal(pos));
}

void MainWindow::ShowOnStartup()
{
    if(eewSettings->windowRect.width()>0||eewSettings->windowRect.height()>0)
        setGeometry(eewSettings->windowRect);
    if(eewSettings->windowMaximized)
        setWindowState(Qt::WindowMaximized|windowState());
    if(!eewSettings->showWindowOnStartup)
        hide();
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    QMainWindow::timerEvent(event);
    if(event->timerId()==timerIdActiveEEW){
        qint64 timestampNowMS=QDateTime::currentMSecsSinceEpoch();
        QQuickItem*mapView=ui->quickMapView->rootObject();
        for(int i=0;i<activeEEW.size();){
            //对已有的EEW计算，显示信息，检查是否抵达用户所在地，并发送声音，对已失效的EEW移除出列表
            //注意跟上方历史数据的区别，这个是生效中的，上方是所有历史的数据
            //对所有记录符合时间和距离条件的，列入警报
            //条件：
            //计算地震波是否已传播到最大距离
            //若是则略过，否则纳入警报
            QJsonObject&e=activeEEW[i];
            double eventId=e.value("eventId").toDouble();
            double epiLat=e.value("latitude").toDouble();
            double epiLng=e.value("longitude").toDouble();
            double dep=e.value("depth").toDouble();
            double mag=e.value("magnitude").toDouble();
            double epiTimestampMS=e.value("startAt").toDouble();//millisecond
            qint64 timeLeftMS=Value::getSWaveMSTimeStraight(epiLng,epiLat,dep,eewSettings->userLongitude,eewSettings->userLatitude,0)-timestampNowMS+epiTimestampMS;
            double maxSwaveR=e.value("maxDistanceSwave").toDouble();
            double epiInt=Value::calcMaxInt(mag,dep,epiLng,epiLat);
            double localInt=Value::getIntensityAt(mag,dep,epiLng,epiLat,eewSettings->userLongitude,eewSettings->userLatitude);
            QString timeStr=Value::timestampMSToChineseDateTimeFormat(epiTimestampMS);
            //已传播地表弧线距离
            double ds=Value::distanceSWaveSpreadMS(dep,timestampNowMS-epiTimestampMS);
            //震中与用户所在地表弧线距离
            double dsu=Value::getDistanceSurface(epiLng,epiLat,eewSettings->userLongitude,eewSettings->userLatitude);
            if(ds<e.value("maxDistanceSwave").toDouble()){
                //还未到最大传播距离
                if(ds>=dsu&&e.value("playedSound").toBool(false)==false){
                    e.insert("playedSound",true);
                    //播放抵达声音
                    EEWSounds::GetSounds()->PlaySWaveArriveSound();
                }
                //更新地图上的圆圈标记
                QMetaObject::invokeMethod(mapView,"setEEWCircle",
                                          Q_ARG(QVariant,eventId),//eventId
                                          Q_ARG(QVariant,epiLat),//latitude
                                          Q_ARG(QVariant,epiLng),//longitude
                                          Q_ARG(QVariant,dep),//depth
                                          Q_ARG(QVariant,timestampNowMS-epiTimestampMS),//elapsedMillieconds
                                          Q_ARG(QVariant,Value::distancePWaveSpreadMS(dep,timestampNowMS-epiTimestampMS)),//radiusPwave
                                          Q_ARG(QVariant,ds),//radiusSwave
                                          Q_ARG(QVariant,epiInt),
                                          Q_ARG(QVariant,i+1));
                //show,intensity,localint,location,timeStr,magnitude,depth,counterSec
                QMetaObject::invokeMethod(mapView,"setHeadBar",Q_ARG(QVariant,eventId),
                                          Q_ARG(QVariant,epiInt),Q_ARG(QVariant,localInt),
                                          Q_ARG(QVariant,e.value("epicenter").toString()),
                                          Q_ARG(QVariant,timeStr),Q_ARG(QVariant,mag),Q_ARG(QVariant,dep),
                                          Q_ARG(QVariant,(int)(timeLeftMS/1000)),
                                          Q_ARG(QVariant,e.value("updates").toInt()),
                                          Q_ARG(QVariant,i+1));
                if(i==statusBarEEWIndex){
                    statusBar()->showMessage(tr("EEW %1/%2: %3, %4, M%5, Dep:%6km, Dis:%7km, EpiInt:%8, LocalInt:%9, Swave arriving in [%10]s. (R:%11<MAX:%12km)")
                                             .arg(statusBarEEWIndex+1).arg(activeEEW.size()).arg(timeStr,
                                             e.value("epicenter").toString()).arg(mag)
                                             .arg(dep).arg((int)dsu).arg(epiInt).arg(localInt)
                                             .arg(qMax(0,(int)(timeLeftMS/1000))).arg((int)ds).arg((int)maxSwaveR));
                }
                i++;
            }else{
                //移除地图上的圆圈标记
                activeEEW.erase(activeEEW.begin()+i);
                CheckGeoIntensity();
                QMetaObject::invokeMethod(mapView,"removeEEWCircle",Q_ARG(QVariant,eventId));
                QMetaObject::invokeMethod(mapView,"removeHeadBar",Q_ARG(QVariant,eventId));
                if(activeEEW.empty()){
                    mapCenterUsingBy=mapCenterUsingBy&(~1);
                    killTimer(timerIdActiveEEW);
                    timerIdActiveEEW=0;
                    SetMapCenterToLastEEW();
                }
                break;//不能同时清除多个HeadBar，否则在QML中会有残留
            }
        }
        if(activeEEW.size()>0){
            //计算倒计时并显示
            static int timerCounter=0;
            if(++timerCounter>=60){//3秒*每秒执行次数
                timerCounter=0;
                statusBarEEWIndex=(statusBarEEWIndex+1)%activeEEW.size();
            }
            if(statusBarEEWIndex>=activeEEW.size())
                statusBarEEWIndex=0;
        }
        if((mapCenterUsingBy&2)==0)
            QMetaObject::invokeMethod(mapView,"fitViewportToEEWCircle");
        QMetaObject::invokeMethod(mapView,"updateLegendsSWave");
    }
}

void MainWindow::SetMapCenterTo(double lat, double lng)
{
    ExitViewIntensity();
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"focusLocation",Q_ARG(QVariant,lat),
                              Q_ARG(QVariant,lng),Q_ARG(QVariant,7));
}

void MainWindow::SetMapCenterToHome()
{
    SetMapCenterTo(eewSettings->userLatitude,eewSettings->userLongitude);
}

void MainWindow::SetMapCenterToLastEEW()
{
    if(lastEEWDoc.isEmpty())
        return;
    const QJsonObject&e=lastEEWDoc.object().value("data").toArray()[0].toObject();
    SetMapCenterTo(e.value("latitude").toDouble(),e.value("longitude").toDouble());
}

void MainWindow::OpenTestEEWDialog(const QPoint &pos)
{
    QPoint mpos=ui->quickMapView->mapFromGlobal(pos);
    QVariant v;
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"posToGeo",Q_RETURN_ARG(QVariant,v),
                              Q_ARG(QVariant,mpos.x()),Q_ARG(QVariant,mpos.y()));
    QMap<QString,QVariant>c=v.toMap();
    TestEEWDialog dlg(this);
    dlg.SetPosition(c.value("latitude").toDouble(),c.value("longitude").toDouble());
    if(dlg.exec()==QDialog::Accepted){
        QJsonObject obj;
        int index=FindActiveEEWIndex(1);
        obj.insert("eventId",1);
        obj.insert("updates",index==-1?1:activeEEW[index].value("updates").toInt()+1);
        obj.insert("latitude",dlg.GetLatitude());
        obj.insert("longitude",dlg.GetLongitude());
        obj.insert("depth",dlg.GetDepth());
        obj.insert("epicenter",dlg.GetLocation());
        obj.insert("startAt",time(NULL)*1000.0);
        obj.insert("magnitude",dlg.GetMagnitude());
        AddActiveEEW(obj);
        //多震测试
        /*obj.insert("eventId",2);
        obj.insert("epicenter",dlg.GetLocation()+"2");
        obj.insert("startAt",time(NULL)*1000.0+5000);
        obj.insert("longitude",dlg.GetLongitude()+5);
        AddActiveEEW(obj);
        obj.insert("eventId",3);
        obj.insert("epicenter",dlg.GetLocation()+"3");
        obj.insert("startAt",time(NULL)*1000.0+10000);
        obj.insert("longitude",dlg.GetLongitude()+10);
        AddActiveEEW(obj);*/
    }
}

void MainWindow::OpenSetHomeDialog(const QPoint &pos)
{
    QPoint mpos=ui->quickMapView->mapFromGlobal(pos);
    QVariant v;
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"posToGeo",Q_RETURN_ARG(QVariant,v),
                              Q_ARG(QVariant,mpos.x()),Q_ARG(QVariant,mpos.y()));
    QMap<QString,QVariant>c=v.toMap();
    SetHomeDialog dlg(this);
    dlg.SetPosition(c.value("latitude").toDouble(),c.value("longitude").toDouble());
    if(dlg.exec()==QDialog::Accepted){
        eewSettings->userLatitude=dlg.GetLatitude();
        eewSettings->userLongitude=dlg.GetLongitude();
        eewSettings->SaveSettings();
        SetHome(eewSettings->userLatitude,eewSettings->userLongitude);
    }
}

void MainWindow::QuitApp()
{
    isRunning=false;
    close();
}

void MainWindow::SetShowLegends(bool show)
{
    //https://zhuanlan.zhihu.com/p/66170604
    //不能越级查找
    ui->quickMapView->rootObject()->findChild<QObject*>("columnLegends")->setProperty("visible",show);
    eewSettings->showLegends=show;
    eewSettings->SaveSettings();
}

void MainWindow::debugInspect()
{
    if(!ui->textDebug->isVisible())
        return;
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"debugInspect");
    ui->textDebug->setText(QString::asprintf("activeEEW:%d",activeEEW.size()));
}

void MainWindow::CheckGeoIntensity()
{
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"clearGeoItems");
    if(eewSettings->showEstimatedIntensity){
        QVector<int>indexToAdd[13];//各烈度都有哪些地方[0,12]
        for(int i=0;i<geoPolygons.size();i++){
            double maxIntensity=0;//综合所有地震算出的烈度
            foreach(const QJsonObject&e,activeEEW){//对所有的地震计算烈度
                double epiLat=e.value("latitude").toDouble();
                double epiLng=e.value("longitude").toDouble();
                double dep=e.value("depth").toDouble();
                double mag=e.value("magnitude").toDouble();
                double localInt=geoPolygons[i].GetMaxIntensity(epiLat,epiLng,mag,dep);
                maxIntensity=qMax(maxIntensity,localInt);
            }
            if(maxIntensity>0){
                //这个地方需要加入显示
                indexToAdd[(int)maxIntensity].push_back(i);
            }
        }
        //注意obj数不宜过多，会卡，因此所用geojson数据也不能过于详细，到地级市的范围足够了
        int displayedPolygons=0;
        for(int intensity=12;intensity>=1;intensity--){
            QVector<QGeoPolygon>gpv;
            for(int i=0;i<indexToAdd[intensity].size();i++){
                int iGeo=indexToAdd[intensity][i];
                for(int iPart=0;iPart<geoPolygons[iGeo].longitudes.size();iPart++){
                    gpv.append(QGeoPolygon());
                    QGeoPolygon&gp=gpv.back();
                    for(int j=0;j<geoPolygons[iGeo].longitudes[iPart].size();j++){
                        gp.addCoordinate(QGeoCoordinate(geoPolygons[iGeo].latitudes[iPart][j],geoPolygons[iGeo].longitudes[iPart][j]));
                    }
                }
            }
            QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"addGeoItem",
                                      Q_ARG(QVariant,intensity),
                                      Q_ARG(QVariant,QVariant::fromValue(gpv)));
            displayedPolygons+=indexToAdd[intensity].size();
        }
        logger->info(tr("Displayed %1 polygon(s).").arg(displayedPolygons));
    }
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"refreshMarks");
}

void MainWindow::resizeEvent(QResizeEvent*event)
{
    QMainWindow::resizeEvent(event);
    const QSize &s=event->size();
    if(s.width()>s.height()){
        addDockWidget(Qt::RightDockWidgetArea,ui->dockWidget);
    }else{
        addDockWidget(Qt::BottomDockWidgetArea,ui->dockWidget);
    }
}

void MainWindow::OnCheckDataSourceValid()
{
    //是否有测试选项
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"setShowWarnInfo",Q_ARG(QVariant,eewSettings->userExtra.contains("showtesteew")),
                              Q_ARG(QVariant,tr("TEST")),Q_ARG(QVariant,tr("NOT REAL INFO")));
    //时间设置是否正常（相差5秒内）
    nam->get(QNetworkRequest(QUrl("https://api.wolfx.jp/ntp.json")));
}

void MainWindow::OnCheckSystemTimeValid(QNetworkReply*& reply)
{
    QJsonParseError err;
    if(reply->error()!=QNetworkReply::NoError){
        logger->error(tr("Query NTP Error: %1").arg(reply->error()));
        QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"setShowWarnInfo",Q_ARG(QVariant,true),
                                  Q_ARG(QVariant,tr("INVALID DATA")),Q_ARG(QVariant,tr("CHECK NETWORK STATUS")));
    }else{
        QByteArray responseData=reply->readAll();
        QJsonDocument docOriginal=QJsonDocument::fromJson(responseData,&err);
        if(err.error!=QJsonParseError::NoError){
            logger->error(tr("NTP JSON Error: %1").arg(err.error));
            QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"setShowWarnInfo",Q_ARG(QVariant,true),
                                      Q_ARG(QVariant,tr("INVALID DATA")),Q_ARG(QVariant,tr("NTP DATA ERROR")));
        }else{
            QString fmt=docOriginal.object().value("CST").toObject().value("str").toString("1970-01-01 00:00:00");
            qint64 serverUtcMSec=Value::chineseDateTimeFormatToTimestampMS(fmt);
            qint64 localUtcMSec=QDateTime::currentMSecsSinceEpoch();
            if(abs(serverUtcMSec-localUtcMSec)<=5000){
                logger->info(tr("Server Time: %1").arg(fmt));
            }else{
                QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"setShowWarnInfo",Q_ARG(QVariant,true),
                                          Q_ARG(QVariant,tr("INVALID DATA")),Q_ARG(QVariant,tr("CHECK IF SYSTEM TIME IS CORRECT")));
            }
        }
    }
}

void MainWindow::SetReplyTimeout(QNetworkReply*reply)
{
    QTimer*readTimeoutTimer=new QTimer(reply);
    readTimeoutTimer->setSingleShot(true);
    connect(readTimeoutTimer,&QTimer::timeout,this,[reply,this](){
        if(reply->isRunning()){
            reply->abort();//即使调用了abort也会进入 OnNetworkReply 的回调
            reply->deleteLater();
            logger->warn(tr("Reply Abort:%1").arg(reply->url().toString()));
            emit SignalRebuildNAM();
        }
    });
    readTimeoutTimer->start(eewSettings->timeoutNetworkReadMS);
}

void MainWindow::RebuildNAM()
{
    logger->info(tr("Rebuild NAM..."));
    EndQuery();
    if(nam)
        delete nam;
    nam=new QNetworkAccessManager(this);
    //nam->setTransferTimeout(eewSettings->timeoutNetworkReadMS);
    connect(nam,&QNetworkAccessManager::finished,this,&MainWindow::OnNetworkReply);
    StartQuery();
}

void MainWindow::ResetNetworkConnection()
{
    RebuildNAM();
}

void MainWindow::on_listHistory_customContextMenuRequested(const QPoint &pos)
{
    if(ui->listHistory->itemAt(pos))
        contextMenusWindow->TrackListMenu(ui->listHistory->mapToGlobal(pos));
}

void MainWindow::OnMenuCommandViewIntensity(const QPoint &pos)
{
    if(ui->tabWidget->currentIndex()==0)
        ((EarthquakeListEntryForm*)ui->listHistory->itemWidget(ui->listHistory->itemAt(ui->listHistory->mapFromGlobal(pos))))->ShowIntensityOnMap();
    else
        ((EarthquakeListEntryForm*)ui->listEEW->itemWidget(ui->listEEW->itemAt(ui->listEEW->mapFromGlobal(pos))))->ShowIntensityOnMap();
}

void MainWindow::ExitViewIntensity()
{
    if(activeEEW.size()>0)
        return;
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"clearGeoItems");
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"refreshIntensityPolygons");
}

void MainWindow::ShowIntensityOnMap(double lat, double lng, double mag, double depth,bool isNewHistory)
{
    if(activeEEW.size()>0&&!isNewHistory)
        return;
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"clearGeoItems");
    QVector<int>indexToAdd[13];//各烈度都有哪些地方[0,12]
    for(int i=0;i<geoPolygons.size();i++){
        double maxIntensity=0;//综合所有地震算出的烈度
        double localInt=geoPolygons[i].GetMaxIntensity(lat,lng,mag,depth);
        maxIntensity=qMax(maxIntensity,localInt);
        if(maxIntensity>0){
            //这个地方需要加入显示
            indexToAdd[(int)maxIntensity].push_back(i);
        }
    }
    //注意obj数不宜过多，会卡，因此所用geojson数据也不能过于详细，到地级市的范围足够了
    int displayedPolygons=0;
    for(int intensity=12;intensity>=1;intensity--){
        QVector<QGeoPolygon>gpv;
        for(int i=0;i<indexToAdd[intensity].size();i++){
            int iGeo=indexToAdd[intensity][i];
            for(int iPart=0;iPart<geoPolygons[iGeo].longitudes.size();iPart++){
                gpv.append(QGeoPolygon());
                QGeoPolygon&gp=gpv.back();
                for(int j=0;j<geoPolygons[iGeo].longitudes[iPart].size();j++){
                    gp.addCoordinate(QGeoCoordinate(geoPolygons[iGeo].latitudes[iPart][j],geoPolygons[iGeo].longitudes[iPart][j]));
                }
            }
        }
        QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"addGeoItem",
                                  Q_ARG(QVariant,intensity),
                                  Q_ARG(QVariant,QVariant::fromValue(gpv)));
        displayedPolygons+=indexToAdd[intensity].size();
    }
    if(displayedPolygons==0){
        SetMapCenterTo(lat,lng);
        return;
    }
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"refreshIntensityPolygons");
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"fitIntensityPolygons",
                              Q_ARG(QVariant,lat),
                              Q_ARG(QVariant,lng),
                              Q_ARG(QVariant,Value::calcMaxInt(mag,depth,lng,lat)));
}

void MainWindow::on_listEEW_customContextMenuRequested(const QPoint &pos)
{
    if(ui->listEEW->itemAt(pos))
        contextMenusWindow->TrackListMenu(ui->listEEW->mapToGlobal(pos));
}

bool MainWindow::eewUpdated(const QJsonDocument&jsonNew)
{
    if(jsonNew.object().value("code").toInt()!=0)
        return false;
    QMap<double,int>lastMap;
    const QJsonArray&arrLast=lastEEWDoc.object().value("data").toArray(),
        arrUpdate=jsonNew.object().value("data").toArray();
    for(int i=0;i<arrLast.size();i++)
        lastMap.insert(arrLast[i].toObject().value("eventId").toDouble(),i);
    for(int i=0;i<arrUpdate.size();i++){
        double eventId=arrUpdate[i].toObject().value("eventId").toDouble();
        if(lastMap.find(eventId)==lastMap.end())
            return true;
        if(arrUpdate[i].toObject()!=arrLast[lastMap[eventId]].toObject())
            return true;
    }
    return false;
}

bool MainWindow::historyUpdated(const QJsonDocument&jsonNew,QVector<int>*vNew)
{
    //地震台网记录，判断方法：CATA_ID属性
    //https://www.ceic.ac.cn/ajax/speedsearch?num=6&&page=1
    //{"shuju":[
    //{"id":"44762","CATA_ID":"CD20230419082211.00","SAVE_TIME":"2023-04-19 08:28:46","O_TIME":"2023-04-19 08:22:10",
    //"EPI_LAT":"40.77","EPI_LON":"105.44","EPI_DEPTH":10,"AUTO_FLAG":"M","EQ_TYPE":"M","O_TIME_FRA":"0",
    //"M":"3.3","M_MS":"0","M_MS7":"0","M_ML":"0","M_MB":"0","M_MB2":"0","SUM_STN":"0","LOC_STN":"0",
    //"LOCATION_C":"\u5185\u8499\u53e4\u963f\u62c9\u5584\u76df\u963f\u62c9\u5584\u5de6\u65d7","LOCATION_S":"","CATA_TYPE":"",
    //"SYNC_TIME":"2023-04-19 08:28:46","IS_DEL":"","EQ_CATA_TYPE":"","NEW_DID":"CD20230419082211"},
    //...],...}

    QMap<QString,int>lastMap;
    const QJsonArray&arrLast=lastHistoryDoc.object().value("shuju").toArray(),
        arrUpdate=jsonNew.object().value("shuju").toArray();
    for(int i=0;i<arrLast.size();i++)
        lastMap.insert(arrLast[i].toObject().value("id").toString(),i);
    if(vNew)
        vNew->clear();
    for(int i=0;i<arrUpdate.size();i++){
        QString eventId=arrUpdate[i].toObject().value("id").toString();
        if(lastMap.find(eventId)==lastMap.end()){
            if(vNew){
                vNew->push_back(i);
            }else{
                return true;
            }
        }
        if(arrUpdate[i].toObject()!=arrLast[lastMap[eventId]].toObject()){
            if(vNew){
                vNew->push_back(i);
            }else{
                return true;
            }
        }
    }
    if(vNew&&vNew->size()>0)
        return true;
    return false;
}

void MainWindow::TempShowWindowIfHidden()
{
    //这个_hidden相关的代码是因为有一个疑似Qt组件的Bug：如果窗口不显示的话，如果有两次或两次以上的列表更新会发现地图左上角会有些不正常的记录显示
    //而且列表的布局也会有些问题
    _tempShowIsHidden=isHidden();
    if(_tempShowIsHidden)
        show();
}

void MainWindow::TempHideWindowIfHidden()
{
    if(_tempShowIsHidden)
        hide();
}

QIcon MainWindow::MakeIntensityIcon(double intensity)
{
    QPixmap pm(96,96);
    QPainter pt(&pm);
    QFont f("Sarasa Mono SC");
    f.setPixelSize(pm.height());
    f.setBold(true);
    pt.setFont(f);
    QVariant rTextColor,rBkColor;
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"getIntTextColors",Q_RETURN_ARG(QVariant,rTextColor),Q_ARG(QVariant,intensity));
    QMetaObject::invokeMethod(ui->quickMapView->rootObject(),"getIntColors",Q_RETURN_ARG(QVariant,rBkColor),Q_ARG(QVariant,intensity));
    QBrush bgBrush(QColor(rBkColor.toString()));
    pt.fillRect(0,0,pm.width(),pm.height(),bgBrush);
    QPen textPen(QColor(rTextColor.toString()));
    pt.setPen(textPen);
    pt.drawText(0,0,pm.width(),pm.height(),Qt::AlignCenter,QString("%1").arg(intensity));
    return pm;
}

bool MainWindow::eventFilter(QObject*obj,QEvent*e)
{
    static QWidget*longPressWidget=nullptr;
    static qint64 pts=0;
    if(e->type()==QEvent::MouseButtonPress){
        QMouseEvent*me=(QMouseEvent*)e;
        if(me->button()==Qt::LeftButton){
            longPressWidget=(QWidget*)obj;
            pts=QDateTime::currentMSecsSinceEpoch();
        }
    }else if(e->type()==QEvent::MouseButtonRelease){
        QMouseEvent*me=(QMouseEvent*)e;
        if(me->button()==Qt::LeftButton&&longPressWidget&&QDateTime::currentMSecsSinceEpoch()>pts+1000){
            QPoint pt=longPressWidget->mapFromGlobal(QCursor::pos());
            QContextMenuEvent cme(QContextMenuEvent::Mouse,pt);
            QApplication::sendEvent(longPressWidget,&cme);
        }
    }
    return QObject::eventFilter(obj,e);
}
