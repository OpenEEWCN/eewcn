#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "eewsettings.h"
#include "contextmenuswindow.h"
#include "geopolygon.h"
#include "logger.h"
#include "stationdata.h"
#include <QMainWindow>
#include <QTimer>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QSystemTrayIcon>
#include <QWebSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void InitEEW();
    void InitEEWAfterQMLLoaded();
    void ReleaseEEW();
    void OnQueryEEW();
    void OnQueryHistory();
    void OnQueryStation(int i);
    void OnCheckDataSourceValid();
    void OnCheckSystemTimeValid(QNetworkReply *&reply);
    void ShowEEWDataOnMap(const QJsonDocument &doc);
    void ShowEEWDataOnList(const QJsonDocument &doc);
    void ReportEEWData(const QJsonDocument &doc);
    void ShowHistoryDataOnMap(const QJsonDocument &doc);
    void ShowHistoryDataOnList(const QJsonDocument &doc);
    void ReportHistoryData(const QJsonDocument &doc);
    void ShowAllStationDataOnMap();
    void ShowAllStationDataOnList();
    void ReportAllStationData();
    void SetShowHistoryWindow(bool show);
    void SetShowStatusBar(bool show);
    void SetShowLegends(bool show);
    void ShowOnStartup();
    void AddActiveEEW(const QJsonObject&e);
    void SetMapCenterTo(double lat,double lng);
    void SetMapCenterToHome();
    void SetMapCenterToLastEEW();
    void OpenTestEEWDialog(const QPoint&pos);
    void OpenSetHomeDialog(const QPoint&pos);
    void QuitApp();
    void ResetNetworkConnection();
    void SetHome(double lat,double lng);
    void OnMenuCommandViewIntensity(const QPoint&pos);
    void StartStation(bool getdatafirst);
    void StopStation();
    void RestartStation();
    void StoreStationData(int index,const QJsonDocument& doc);
    void OnUpdateAllStation();
    QIcon MakeIntensityIcon(double intensity);

protected:
    void closeEvent(QCloseEvent*event)override;
    bool eventFilter(QObject*obj,QEvent*e)override;

private slots:
    void timerTask();
    void stationTimerTask();
    void OnNetworkReply(QNetworkReply*reply);
    void OnWSStationReply(const QString &msg);//在函数内部获取下标i
    void OnWSStationPong();
    void TrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void TrayMessageClicked();
    void on_dockWidget_visibilityChanged(bool visible);
    void on_frameMapView_customContextMenuRequested(const QPoint &pos);
    void timerEvent(QTimerEvent*event)override;
    void resizeEvent(QResizeEvent*)override;

    void on_listHistory_customContextMenuRequested(const QPoint &pos);

    void on_listEEW_customContextMenuRequested(const QPoint &pos);

signals:
    void SignalRebuildNAM();

private:
    int FindActiveEEWIndex(double eventId);
    void debugInspect();
    void CheckGeoIntensity();
    void SetReplyTimeout(QNetworkReply*reply);
    void RebuildNAM();
    void ExitViewIntensity();
    void ShowIntensityOnMap(double lat,double lng,double mag,double depth,bool isNewHistory=false);
    bool eewUpdated(const QJsonDocument&jsonNew);
    bool historyUpdated(const QJsonDocument&jsonNew,QVector<int>*vNew=nullptr);
    void TempShowWindowIfHidden();
    void TempHideWindowIfHidden();
    void StartQuery(bool getdatafirst = false);
    void EndQuery();
    void GetWolfxSeisList();
    QJsonObject ConvertWolfxStationData(const QJsonObject& obj,int index);

    Ui::MainWindow *ui;
    ContextMenusWindow *contextMenusWindow;
    EEWSettings*eewSettings;
    QNetworkAccessManager *nam;
    QTimer *timerEEW,*timerHistory,*timerMapCenterUsingByHistory,*timerUpdateAllStation;
    QJsonDocument lastEEWDoc,lastHistoryDoc;
    QSystemTrayIcon *trayIcon;
    QVector<QJsonObject>activeEEW;
    int timerIdActiveEEW;
    int statusBarEEWIndex;
    bool isRunning;
    QVector<GeoPolygon>geoPolygons;
    /** @brief 单位为秒*/
    qint64 lastEEWDataTimestamp,lastHistoryDataTimestamp;
    Logger* logger;
    bool _tempShowIsHidden;//该变量仅用于窗口隐藏时收到新记录后临时显示用
    int mapCenterUsingBy;//0=无对象占用 1=被速报占用 2=被记录占用 3=被同时占用
    QVector<StationData>stationDataList;
    QMap<QString,int>stationURLIndexDict;
    struct WOLFX_SEIS
    {
        QString wsURL,getURL,name;
        double longitude,latitude,height;
    };
    QVector<struct WOLFX_SEIS>wolfxSeisList;
    int timestampAppStartSec;
    QVector<uchar>qmlTranslationData;
};
#endif // MAINWINDOW_H
