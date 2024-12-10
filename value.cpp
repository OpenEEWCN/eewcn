#include "value.h"
#include <cmath>
#include <QDateTime>
#include <QTimeZone>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QNetworkInterface>
#include <QCryptographicHash>

#define PI acos(-1) //rad

double Value::PWAVE_SPEED=7;
double Value::SWAVE_SPEED=4;
double Value::EARTH_AVG_RADIUS=6371;


double Value::calcMaxInt(double magnitude, double depth,double epicenterLng,double epicenterLat)
{
    double a = 1.65 * magnitude;
    double b = depth < 10 ? 1.21 * std::log10(10) : 1.21 * std::log10(depth);
    return qMin(qMax(0.0,std::round(a / b)),12.0);
}

QString Value::timestampMSToChineseDateTimeFormat(qint64 msts)
{
    return QDateTime::fromMSecsSinceEpoch(msts,QTimeZone("Asia/Shanghai")).toString("yyyy-MM-dd hh:mm:ss");
}

qint64 Value::chineseDateTimeFormatToTimestampMS(QString fmt)
{
    QDateTime dt=QDateTime::fromString(fmt,"yyyy-MM-dd hh:mm:ss");
    dt.setTimeZone(QTimeZone("Asia/Shanghai"));
    return dt.toMSecsSinceEpoch();
}

qint64 Value::millisecondsAgoFromNow(qint64 msts)
{
    return QDateTime::currentMSecsSinceEpoch()-msts;
}

QString Value::toTimeLengthHMS(int sec)
{
    return QString::asprintf("%d:%02d:%02d",sec/3600,(sec/60)%60,sec%60);
}

qint64 Value::millisecondsAgoFromNow(QString chineseDateTimeFormat)
{
    return QDateTime::currentMSecsSinceEpoch()-chineseDateTimeFormatToTimestampMS(chineseDateTimeFormat);
}

qint64 Value::toMilliseconds(int hours,int minutes,int seconds)
{
    return (hours*3600+minutes*60+seconds)*1000ll;
}

double Value::degToRad(double deg)
{
    return deg*PI/180.0;
}

double Value::getDistanceSurface(double lng1, double lat1, double lng2, double lat2)
{
    double radLng1 = degToRad(lng1);
    double radLat1 = degToRad(lat1);
    double radLng2 = degToRad(lng2);
    double radLat2 = degToRad(lat2);
    double distance;

    double a = radLat2 - radLat1;//纬度差
    double b = radLng2 - radLng1;//经度差

    double c = sin(a / 2) * sin(a / 2) + cos(radLat1) * cos(radLat2) * sin(b / 2) * sin(b / 2);
    distance = (2 * atan2(sqrt(c), sqrt(1 - c))) * EARTH_AVG_RADIUS;//6371(km)是地球平均半径
    return distance;
}

double Value::getDistanceStraight(double lng1, double lat1, double dep1, double lng2, double lat2, double dep2)
{
    double radLng1 = degToRad(lng1);
    double radLat1 = degToRad(lat1);
    double radLng2 = degToRad(lng2);
    double radLat2 = degToRad(lat2);
    double distance;

    double a = radLat2 - radLat1;//纬度差
    double b = radLng2 - radLng1;//经度差

    double c = sin(a / 2) * sin(a / 2) + cos(radLat1) * cos(radLat2) * sin(b / 2) * sin(b / 2);
    double angle = 2 * atan2(sqrt(c), sqrt(1 - c));//弧度角
    double length1=EARTH_AVG_RADIUS-dep1;
    double length2=EARTH_AVG_RADIUS-dep2;
    distance = sqrt(length1*length1+length2*length2-2*length1*length2*cos(angle));
    return distance;
}

double Value::getPWaveMSTimeSurface(double lng1, double lat1, double lng2, double lat2)
{
    return 1000ll * getDistanceSurface(lng1, lat1, lng2, lat2) / PWAVE_SPEED;//纵波参考速度：5.5 - 7 km/s
}

double Value::getSWaveMSTimeSurface(double lng1, double lat1, double lng2, double lat2)
{
    return 1000ll * getDistanceSurface(lng1, lat1, lng2, lat2) / SWAVE_SPEED;//横波参考速度：3.2 - 4 km/s
}

double Value::getPWaveMSTimeStraight(double lng1, double lat1, double dep1, double lng2, double lat2, double dep2)
{
    return 1000ll * getDistanceStraight(lng1, lat1, dep1, lng2, lat2, dep2) / PWAVE_SPEED;//纵波参考速度：5.5 - 7 km/s
}

double Value::getSWaveMSTimeStraight(double lng1, double lat1, double dep1, double lng2, double lat2, double dep2)
{
    return 1000ll * getDistanceStraight(lng1, lat1, dep1, lng2, lat2, dep2) / SWAVE_SPEED;//横波参考速度：3.2 - 4 km/s
}

double Value::getIntensityAt(double magnitude, double depth, double epicenterLng,double epicenterLat,double atLng,double atLat)
{
    return qMax(0.0,std::round(1.92 + 1.63 * magnitude - 3.49 * log10(getDistanceStraight(epicenterLng,epicenterLat,depth,atLng,atLat,0))));
}

QString Value::getIntensityDescriptionAt(double magnitude, double depth, double epicenterLng,double epicenterLat,double atLng,double atLat,int language)
{
    int i=(int)getIntensityAt(magnitude,depth,epicenterLng,epicenterLat,atLng,atLat);
    QString desc[4][6]={
        {"No feeling","Slight feeling","Feeling on high","Medium feeling","Strong feeling","Extreme feeling"},
        {"无震感","可能有震感","高处有震感","较强震感","强烈震感","极强震感"},
        {"無震感","可能有震感","高處有震感","較強震感","強烈震感","極強震感"},
        {"感知なし","軽い感じ","高所に感じ","中度感じ","強い感じ","極端衝撃"}
    };
    if(language<0||language>3)
        language=2;
    //http://www.igeo.cgs.gov.cn/kpyd/dxzs/201608/t20160811_352186.html
    //我国把烈度划分为十二度，不同烈度的地震，其影响和破坏大体如下：
    //[0]小于三度人无感觉，只有仪器才能记录到；
    //[1]三度在夜深人静时人有感觉；
    //[2]四～五度睡觉的人会惊醒，吊灯摇晃；
    //[3]六度器皿倾倒，房屋轻微损坏；
    //[4]七～八度房屋受到破坏，地面出现裂缝；
    //[5]九～十度房屋倒塌，地面破坏严重；
    //[5]十一～十二度毁灭性的破坏；
    //例如，1976年唐山地震，震级为7.6级，震中烈度为十一度；
    //受唐山地震的影响，天津市地震烈度为八度，北京市烈度为六度，再远到石家庄、太原等就只有四至五度了。
    int index[13]={0,0,0,1,2,2,3,4,4,5, 5, 5, 5};
    //             0 1 2 3 4 5 6 7 8 9 10 11 12
    return desc[language][index[i]];
}

double Value::distancePWaveSpreadMS(double depth,double deltaMS)
{
    double rWave=deltaMS*PWAVE_SPEED/1000.0;
    if(rWave<depth)
        return rWave-depth;
    if(rWave>2*EARTH_AVG_RADIUS-depth)
        return 2*PI*EARTH_AVG_RADIUS;
    double dc=EARTH_AVG_RADIUS-depth;//两圆心距
    double angle=acos((EARTH_AVG_RADIUS*EARTH_AVG_RADIUS+dc*dc-rWave*rWave)/(2*EARTH_AVG_RADIUS*dc));
    return angle*EARTH_AVG_RADIUS;
}

double Value::distanceSWaveSpreadMS(double depth,double deltaMS)
{
    double rWave=deltaMS*SWAVE_SPEED/1000.0;
    if(rWave<depth)
        return rWave-depth;
    if(rWave>2*EARTH_AVG_RADIUS-depth)
        return 2*PI*EARTH_AVG_RADIUS;
    double dc=EARTH_AVG_RADIUS-depth;//两圆心距
    double angle=acos((EARTH_AVG_RADIUS*EARTH_AVG_RADIUS+dc*dc-rWave*rWave)/(2*EARTH_AVG_RADIUS*dc));
    return angle*EARTH_AVG_RADIUS;
}

QString Value::getLanguageFontName(EEWCN_LANGUAGE language)
{
    const char*fn[]={"Sarasa UI SC","Sarasa UI SC","Sarasa UI TC","Sarasa UI J"};
    return fn[(int)language];
}

double Value::maxDistanceSWaveSpread(double magnitude,double depth, double epicenterLng,double epicenterLat)
{
    double d=200.0;//弧线距离
    while(true){
        double rad=d/EARTH_AVG_RADIUS;
        double dc=EARTH_AVG_RADIUS-depth;
        double straightd=sqrt(dc*dc+EARTH_AVG_RADIUS*EARTH_AVG_RADIUS-2*dc*EARTH_AVG_RADIUS*cos(rad));
        double intensity=std::round(1.92 + 1.63 * magnitude - 3.49 * log10(straightd));
        if(intensity<=d/1000)//不太确定该如何判断取值
            break;
        d+=200.0;
    }
    return d;
}

qint64 Value::getCompileTimeMS()
{
    QString dateTime=__DATE__ __TIME__;
    dateTime.replace("  "," 0");
    return QLocale(QLocale::English).toDateTime(dateTime,"MMM dd yyyyhh:mm:ss").toTime_t()*1000ll;
}

QString Value::getAppDataPath(QString filename,bool makepath)
{
#ifdef Q_OS_ANDROID
    QStringList locations=QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    QString appdata;
    foreach (appdata, locations) {
        if(!appdata.startsWith("/data"))
            break;
    }
#elif defined(Q_OS_IOS)
    QString appdata=QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#else
    QString appdata=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
    if(appdata.isEmpty())
        return "";
    if(makepath&&!QFile::exists(appdata)){
        QDir dir(appdata);
        dir.mkpath(".");
    }
    return appdata.append('/').append(filename);
}

const unsigned char shift[4]={'Y','U','X','N'};//偏移
const unsigned char revshift[4]={'L','X','F','Y'};//反偏移（部分）

QString Value::getCompiledMachineID()
{
    QString uuid=VALIDATION_MACHINE_ID;
    if(uuid.isEmpty())
        return uuid;
    unsigned char remshift[4];//反偏移（剩余部分）
    for(int i=0;i<4;i++){
        remshift[i]=shift[i]-revshift[i];
    }
    QString fmt=uuid.remove("-");
    unsigned char x[16];
    for(int i=0;i<16;i++){
        unsigned int n;
        sscanf(fmt.mid(i*2,2).toStdString().c_str(),"%02X",&n);
        x[i]=(n+shift[i%4]-remshift[i%4])%256;
    }
    return QByteArray::fromRawData((char*)x,16).toBase64();
}

QStringList Value::getAllLocalMachineIDs()
{
    QString uuid=VALIDATION_UNKNOWN_MACHINE_ID;
    QStringList ids,fmts;
#ifdef Q_OS_WIN
    //Windows可直接用uid
    uuid=QSysInfo::machineUniqueId();
#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    //Linux可直接用uid
    uuid=QSysInfo::machineUniqueId();
#elif defined(Q_OS_MAC) && !defined(Q_OS_IOS)
    //Mac可直接用uid
    uuid=QSysInfo::machineUniqueId();
#endif
    if(uuid.compare(VALIDATION_UNKNOWN_MACHINE_ID)==0){//其他系统建议用网卡mac（6字节）的MD5（16字节）作为id
        foreach (QNetworkInterface ni, QNetworkInterface::allInterfaces()) {
            QString mac=ni.hardwareAddress().remove(":");
            if(mac.length()==12&&!mac.startsWith("000000")){
                unsigned char x[6];
                for(int i=0;i<6;i++){
                    unsigned int n;
                    sscanf(mac.mid(i*2,2).toStdString().c_str(),"%02X",&n);
                    x[i]=n%256;
                }
                QByteArray aMD5=QCryptographicHash::hash(QByteArray::fromRawData((char*)x,6),QCryptographicHash::Md5);
                fmts.push_back(aMD5.toHex());
            }
        }
    }else{
        fmts.push_back(uuid.remove("-"));
    }
    foreach (QString fmt, fmts) {
        unsigned char x[16];
        for(int i=0;i<16;i++){
            unsigned int n;
            sscanf(fmt.mid(i*2,2).toStdString().c_str(),"%02X",&n);
            x[i]=(n+shift[i%4])%256;
        }
        ids.push_back(QByteArray::fromRawData((char*)x,16).toBase64());
    }
    return ids;
}

QStringList Value::langRegionTag={"en","zh_CN","zh_TW","ja"};
