#ifndef STATIONDATA_H
#define STATIONDATA_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QJsonDocument>

class StationData:public QObject
{
    Q_OBJECT
public:
    explicit StationData(QObject*parent = nullptr);
    StationData(const StationData&other);
    ~StationData();
    int index;//数据源下标，由程序初始化
    int status;//0=未使用 -1=故障且无法恢复 1=已使用正常 2=已使用但数据过期需要更新或恢复，由程序初始化
    QWebSocket*webSocket;//由程序初始化
    QTimer*timer;//用于查询间隔的定时器，由程序初始化
    static qint64 dataStoreTimeMS;//数据存储时间，由程序初始化
    QString url;//数据源的链接，由用户初始化

    struct DataStore{
        double longitude,latitude;//由用户初始化，也可以在实时获取时设置
        double height;//由用户初始化，也可以在实时获取时设置（单位：米）
        QString name;//由用户初始化，也可以在实时获取时设置
        struct TimeElapsedData{
            qint64 timestamp_ms;
            double pga,pgv,pgd;//实时获取
            double intensity;//实时获取并计算
        };
        QVector<TimeElapsedData>timeElapsedData;//按时间序列存储的结果
        double GetMaxPGAFromTimeElapse();
        double GetMaxPGVFromTimeElapse();
        double GetMaxPGDFromTimeElapse();
        double GetMaxIntensityFromTimeElapse();
    };
    QVector<DataStore>dataStore;//存储该数据源产生的所有结果（对于wolfx数据，一个源只有一组结果）
    void AddStationData(const QJsonDocument& doc);
    void RemoveOutdatedData();
};

#endif // STATIONDATA_H
