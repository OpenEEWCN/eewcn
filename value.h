#ifndef VALUE_H
#define VALUE_H

#include <QString>

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#if VERSION_PATCH == 0
#if VERSION_MINOR == 0
#define VERSION_NAME QT_STRINGIFY(VERSION_MAJOR)
#else
#define VERSION_NAME QT_STRINGIFY(VERSION_MAJOR) "." QT_STRINGIFY(VERSION_MINOR)
#endif
#else
#define VERSION_NAME QT_STRINGIFY(VERSION_MAJOR) "." QT_STRINGIFY(VERSION_MINOR) "." QT_STRINGIFY(VERSION_PATCH)
#endif

#define EEWCN_MAPBOX_ACCESS_TOKEN ""
#define EEWCN_MAPBOX_STYLE ""

#define VALIDATION_UNKNOWN_MACHINE_ID "UNKNOWN"

#ifndef VALIDATION_MACHINE_ID
#define VALIDATION_MACHINE_ID VALIDATION_UNKNOWN_MACHINE_ID
#endif

enum class EEWCN_LANGUAGE{
    EEWCN_ENGLISH,
    EEWCN_CHINESE_MAINLAND,
    EEWCN_CHINESE_TAIWAN,
    EEWCN_JAPANESE
};

class Value
{
public:
    /**
     * @brief 计算震源地的烈度
     */
    static double calcMaxInt(double magnitude, double depth,double epicenterLng,double epicenterLat);
    static QString timestampMSToChineseDateTimeFormat(qint64 msts);
    static qint64 chineseDateTimeFormatToTimestampMS(QString fmt);
    static qint64 millisecondsAgoFromNow(qint64 msts);
    static qint64 millisecondsAgoFromNow(QString chineseDateTimeFormat);
    static qint64 toMilliseconds(int hours,int minutes,int seconds);
    static QString toTimeLengthHMS(int sec);
    /**
     * @brief 将角度转化为弧度
     */
    static double degToRad(double deg);
    /**
     * @brief 根据两点经纬度坐标计算大圆弧线距离(KM)
     */
    static double getDistanceSurface(double lng1, double lat1, double lng2, double lat2);
    /**
     * @brief 根据两点经纬度坐标计算三维直线距离(KM)
     */
    static double getDistanceStraight(double lng1, double lat1, double dep1, double lng2, double lat2, double dep2);
    /**
     * @brief 根据两点经纬度坐标计算纵波穿过该两点大圆弧线距离所需时间(ms)
     */
    static double getPWaveMSTimeSurface(double lng1, double lat1, double lng2, double lat2);
    /**
     * @brief 根据两点经纬度坐标计算横波穿过该两点大圆弧线距离所需时间(ms)
     */
    static double getSWaveMSTimeSurface(double lng1, double lat1, double lng2, double lat2);
    /**
     * @brief 根据两点经纬度坐标计算纵波穿过该两点三维直线距离所需时间(ms)
     */
    static double getPWaveMSTimeStraight(double lng1, double lat1, double dep1, double lng2, double lat2, double dep2);
    /**
     * @brief 根据两点经纬度坐标计算横波穿过该两点三维直线距离所需时间(ms)
     */
    static double getSWaveMSTimeStraight(double lng1, double lat1, double dep1, double lng2, double lat2, double dep2);
    /**
     * @brief 获取震源以外地点的震度（数值）
     */
    static double getIntensityAt(double magnitude, double depth, double epicenterLng,double epicenterLat,double atLng,double atLat);
    /**
     * @brief 获取震源以外地点的震度（文字描述）
     */
    static QString getIntensityDescriptionAt(double magnitude, double depth, double epicenterLng,double epicenterLat,double atLng,double atLat,int language = 2);
    /**
     * @brief 纵波在指定时间内传播了多远，负值表示还未传播到地表上来，在地下多少公里(地表弧线距离，km)
     */
    static double distancePWaveSpreadMS(double depth,double deltaMS);
    /**
     * @brief 横波在指定时间内传播了多远，负值表示还未传播到地表上来，在地下多少公里(地表弧线距离，km)
     */
    static double distanceSWaveSpreadMS(double depth,double deltaMS);
    /**
     * @brief 地震大于0的震度最多能在地表传多远(地表弧线距离，km)
     */
    static double maxDistanceSWaveSpread(double magnitude,double depth, double epicenterLng,double epicenterLat);
    static QString getLanguageFontName(EEWCN_LANGUAGE language);
    static qint64 getCompileTimeMS();
    static QString getAppDataPath(QString filename,bool makepath=true);
    static double PWAVE_SPEED,SWAVE_SPEED,EARTH_AVG_RADIUS;
    static QStringList langRegionTag;
};

#endif // VALUE_H
