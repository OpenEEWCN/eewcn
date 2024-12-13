#include "geopolygon.h"
#include "value.h"
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QTextStream>

int GeoPolygon::Load(const QString &path,QVector<GeoPolygon>&vec)
{
    QFileInfo fi(path);
    //该path是文件
    if(fi.isFile()){
        return LoadGeoJson(path,vec);
    }
    //该path是目录，遍历
    QDir d(path);
    if(d.entryList().size()==3){
        //该path下只有一个文件或目录
        foreach (QString p, d.entryList()) {
            if(p[0]!='.'){
                return Load(path+"/"+p,vec);
            }
        }
    }
    //该path下有多个文件或目录
    foreach(QString p,d.entryList()){
        if(p.compare(".")&&p.compare("..")){
            Load(path+"/"+p,vec);
        }
    }
    return vec.size();
}

int GeoPolygon::LoadGeoJson(const QString &path,QVector<GeoPolygon>&vec)
{
    QJsonParseError err;
    QFile f(path);
    if(!f.open(QFile::ReadOnly|QFile::Text))
        return false;
    QTextStream fin(&f);
    fin.setCodec("UTF-8");
    QJsonDocument doc=QJsonDocument::fromJson(fin.readAll().toUtf8(),&err);
    if(err.error!=QJsonParseError::NoError)
        return 0;
    //根节点必须是FeatureCollection
    if(doc.object().value("type").toString().compare("FeatureCollection"))
        return 0;
    const QJsonArray&features=doc.object().value("features").toArray();
    for(int i=0;i<features.size();i++){
        const QJsonObject&aFeature=features[i].toObject();
        if(aFeature.value("type").toString().compare("Feature")==0){
            const QJsonObject&geometry=aFeature.value("geometry").toObject();
            if(geometry.value("type").toString().compare("Polygon")==0){
                vec.push_back(GeoPolygon());
                vec.back().LoadPolygon(geometry.value("coordinates").toArray());
            }else if(geometry.value("type").toString().compare("MultiPolygon")==0){
                vec.push_back(GeoPolygon());
                vec.back().LoadMultiPolygon(geometry.value("coordinates").toArray());
            }
        }
    }
    return vec.size();
}

void GeoPolygon::CalcRepresentPoint()
{
    QVector<double>minLats,minLons,maxLats,maxLons;
    foreach(const QVector<double>&lats,latitudes){
        minLats.push_back(*std::min_element(lats.begin(),lats.end()));
        maxLats.push_back(*std::max_element(lats.begin(),lats.end()));
    }
    foreach(const QVector<double>&lons,longitudes){
        minLons.push_back(*std::min_element(lons.begin(),lons.end()));
        maxLons.push_back(*std::max_element(lons.begin(),lons.end()));
    }
    leftLongitude=*std::min_element(minLons.begin(),minLons.end());
    rightLongitude=*std::max_element(maxLons.begin(),maxLons.end());
    topLatitude=*std::max_element(maxLats.begin(),maxLats.end());
    bottomLatitude=*std::min_element(minLats.begin(),minLats.end());
    representLatitude=(topLatitude+bottomLatitude)/2;
    representLongitude=(leftLongitude+rightLongitude)/2;
}

#define IsCoord(jArray) jArray.size()>=2&&jArray[0].isDouble()

void GeoPolygon::LoadPolygon(const QJsonArray &crdArray)
{
    latitudes.clear();
    longitudes.clear();
    for(int j=0;j<crdArray.size();j++){
        const auto&crd=crdArray[j].toArray();
        longitudes.push_back(QVector<double>());
        latitudes.push_back(QVector<double>());
        auto&lats=latitudes.back();
        auto&lons=longitudes.back();
        for(int i=0;i<crd.size();i++){
            lons.push_back(crd[i].toArray()[0].toDouble());
            lats.push_back(crd[i].toArray()[1].toDouble());
        }
    }
    CalcRepresentPoint();
}

void GeoPolygon::LoadMultiPolygon(const QJsonArray &crdArray)
{
    latitudes.clear();
    longitudes.clear();
    for(int k=0;k<crdArray.size();k++){
        const auto&secondCrd=crdArray[k].toArray();
        for(int j=0;j<secondCrd.size();j++){
            const auto&crd=secondCrd[j].toArray();
            longitudes.push_back(QVector<double>());
            latitudes.push_back(QVector<double>());
            auto&lats=latitudes.back();
            auto&lons=longitudes.back();
            for(int i=0;i<crd.size();i++){
                lons.push_back(crd[i].toArray()[0].toDouble());
                lats.push_back(crd[i].toArray()[1].toDouble());
            }
        }
    }
    CalcRepresentPoint();
}

bool GeoPolygon::IsCoordIn(double lat, double lng)
{
    return lat>=bottomLatitude&&lat<=topLatitude&&lng>=leftLongitude&&lng<=rightLongitude;
}

double GeoPolygon::GetMaxIntensity(double epiLat, double epiLng, double mag, double dep)
{
    if(IsCoordIn(epiLat,epiLng))
        return Value::calcMaxInt(mag,dep,epiLng,epiLat);
    //为了避免性能问题只算一个中点的烈度
    /*double maxIntensity=0;
    for(int i=0;i<longitudes.size();i++){
        for(int j=0;j<longitudes[i].size();j++){
            double intensity=Value::getIntensityAt(mag,dep,epiLng,epiLat,longitudes[i][j],latitudes[i][j]);
            maxIntensity=qMax(maxIntensity,intensity);
        }
    }
    return maxIntensity;*/
    return Value::getIntensityAt(mag,dep,epiLng,epiLat,representLongitude,representLatitude);
}
