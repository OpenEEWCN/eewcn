#ifndef GEOPOLYGON_H
#define GEOPOLYGON_H

#include <QVector>
#include <QString>
#include <QJsonArray>

class GeoPolygon
{
public:
    /**
     * @brief 加载GeoJson，path既可能是文件也可能是目录
     * @return 加载成功的数量（仅直接子级）
     * */
    static int Load(const QString&path,QVector<GeoPolygon>&vec);
    /**
     * @brief 加载GeoJson，path必须是一个geojson文件
     * @return 加载成功的数量（仅直接子级）
     * */
    static int LoadGeoJson(const QString&path,QVector<GeoPolygon>&vec);
    double representLatitude,representLongitude;
    QVector<QVector<double>>longitudes,latitudes;//多个封闭曲线
    double leftLongitude,rightLongitude,topLatitude,bottomLatitude;
    /** @brief 采用外接矩形框判断点是否在该范围内，不完全准确*/
    bool IsCoordIn(double lat,double lng);
    double GetMaxIntensity(double epiLat,double epiLng,double mag,double dep);
private:
    void CalcRepresentPoint();
    void LoadPolygon(const QJsonArray&);
    void LoadMultiPolygon(const QJsonArray&);
};

#endif // GEOPOLYGON_H
