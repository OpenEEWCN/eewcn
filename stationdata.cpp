#include "stationdata.h"
#include <algorithm>
#include <QJsonObject>
#include <QJsonArray>

qint64 StationData::dataStoreTimeMS=2*60*1000;

StationData::StationData(QObject*parent):QObject(parent),
    index(-1),status(0),webSocket(nullptr),timer(nullptr)
{
    //Nothing
}

StationData::StationData(const StationData&other):QObject(other.parent()),
    index(other.index),status(other.status),webSocket(other.webSocket),
    timer(other.timer),url(other.url)
{
}

StationData::~StationData()
{
    //Nothing
}

double StationData::DataStore::GetMaxPGAFromTimeElapse()
{
    return std::max_element(timeElapsedData.begin(),timeElapsedData.end(),[](TimeElapsedData&a,TimeElapsedData&b){return a.pga<b.pga;})->pga;
}

double StationData::DataStore::GetMaxPGVFromTimeElapse()
{
    return std::max_element(timeElapsedData.begin(),timeElapsedData.end(),[](TimeElapsedData&a,TimeElapsedData&b){return a.pgv<b.pgv;})->pgv;
}

double StationData::DataStore::GetMaxPGDFromTimeElapse()
{
    return std::max_element(timeElapsedData.begin(),timeElapsedData.end(),[](TimeElapsedData&a,TimeElapsedData&b){return a.pgd<b.pgd;})->pgd;
}

double StationData::DataStore::GetMaxIntensityFromTimeElapse()
{
    return std::max_element(timeElapsedData.begin(),timeElapsedData.end(),[](TimeElapsedData&a,TimeElapsedData&b){return a.intensity<b.intensity;})->intensity;
}

void StationData::AddStationData(const QJsonDocument& doc)
{
    //接受的格式：
    //{data:[{name:AAA,longitude:000,latitude:000,height:000,pga:000,pgv:000,pgd:000,intensity:000,update:000},...]}
    //wolfx测站字段格式：https://api.wolfx.jp/seis.html
    QJsonArray arr=doc.object().value("data").toArray();
    for(int i=0;i<arr.size();i++){
        QJsonObject obj=arr[i].toObject();
        typename QVector<DataStore>::Iterator iter=std::find_if(dataStore.begin(),dataStore.end(),[&obj](DataStore&p){return p.name.compare(obj.value("name").toString())==0;});
        if(dataStore.empty()||iter==dataStore.end()){
            DataStore ds;
            ds.name=obj.value("name").toString();
            dataStore.push_back(ds);
            iter=dataStore.end()-1;
        }
        DataStore &ds=*iter;
        ds.longitude=obj.value("longitude").toDouble();
        ds.latitude=obj.value("latitude").toDouble();
        ds.height=obj.value("height").toDouble();
        DataStore::TimeElapsedData ted;
        ted.timestamp_ms=(qint64)obj.value("update").toDouble();
        ted.intensity=obj.value("intensity").toDouble();
        ted.pga=obj.value("pga").toDouble();
        ted.pgv=obj.value("pgv").toDouble();
        ted.pgd=obj.value("pgd").toDouble();
        ds.timeElapsedData.push_back(ted);
    }
}

void StationData::RemoveOutdatedData()
{
    qint64 oldestTimestamp=QDateTime::currentMSecsSinceEpoch()-dataStoreTimeMS;
    for (DataStore&ds : dataStore) {
        while(ds.timeElapsedData.size()>1&&ds.timeElapsedData.front().timestamp_ms<oldestTimestamp){
            ds.timeElapsedData.pop_front();
        }
    }
}
