#ifndef EEWSETTINGS_H
#define EEWSETTINGS_H

#include <QString>
#include <QRect>
#include "value.h"

enum class HISTORY_DATA_SOURCE{
    HISTORY_DATA_SOURCE_WOLFX,
    HISTORY_DATA_SOURCE_CENCWEB,
    HISTORY_DATA_SOURCE_CENCAPP,
    HISTORY_DATA_SOURCE_FUJIANJU
};

enum class EEW_DATA_SOURCE{
    EEW_DATA_SOURCE_WOLFX,
    EEW_DATA_SOURCE_SICHUANJU
};

enum class STATION_DATA_SOURCE{
    STATION_DATA_SOURCE_WOLFX_LEAFPEACH_WEBSOCKET,
    STATION_DATA_SOURCE_WOLFX_LEAFPEACH_GET
};


class EEWSettings
{
private:
    EEWSettings();
    ~EEWSettings();
    static EEWSettings eewSettings;
public:
    bool LoadSettings();
    bool SaveSettings();
    static EEWSettings*GetSettings();
    /** @brief 构建默认配置*/
    void BuildDefaults();
    /** @brief 从字符串加载配置*/
    bool LoadFromJSONString(QString str);
    /** @brief 从文件加载配置*/
    bool LoadFromJSONFile(QString path);
    /** @brief 生成字符串*/
    QString ToJSONString();
    /** @brief 保存到文件*/
    bool SaveToJSONFile(QString path);
    /** @brief 本地起报级别*10（单位为震度，0为全部警报，-1为不警报，乘10是为了精确到小数点后一位）*/
    int minimumLocalReportLevel10x;
    /** @brief 震源地起报级别*10（单位为震度，0为全部警报，-1为不警报，乘10是为了精确到小数点后一位）*/
    int minimumEpicenterReportLevel10x;
    /** @brief 全球起报级别*10（单位为震度，0为全部警报，-1为不警报，乘10是为了精确到小数点后一位）*/
    int minimumGlobalReportLevel10x;
    /** @brief 严重级别*10（单位为震度，乘10是为了精确到小数点后一位）*/
    int criticalLevel10x;
    /** @brief 弱摇晃烈度*/
    int weakShakeIntensity;
    /** @brief 中等摇晃烈度*/
    int midShakeIntensity;
    /** @brief 强烈摇晃烈度*/
    int strongShakeIntensity;
    /** @brief 用户所处纬度*/
    double userLatitude;
    /** @brief 用户所处经度*/
    double userLongitude;
    /** @brief 转发URL*/
    QString postURL;
    /** @brief EEW获取间隔（秒）*/
    int eewQueryInterval;
    /** @brief EEW获取记录数量*/
    int eewQueryCount;
    /** @brief 记录获取间隔（秒）*/
    int historyQueryInterval;
    /** @brief 记录获取数量*/
    int historyQueryCount;
    /** @brief 测站获取间隔（秒）*/
    int stationQueryInterval;
    /** @brief 界面语言：0=日语 1=汉语 2=英语*/
    EEWCN_LANGUAGE language;
    /** @brief 地震波到达本地音效文件路径*/
    QString pathSWaveArriveSound;
    /** @brief 本地地震预警音效路径*/
    QString pathAlertSound;
    /** @brief 全球地震预警音效路径*/
    QString pathNewRecordSound;
    /** @brief 本地严重预警音效路径*/
    QString pathCriticalAlertSound;
    /** @brief 预警更新音效路径*/
    QString pathUpdateAlertSound;
    /** @brief 弱摇晃音效路径*/
    QString pathWeakShakeSound;
    /** @brief 中等摇晃音效路径*/
    QString pathMidShakeSound;
    /** @brief 强烈摇晃音效路径*/
    QString pathStrongShakeSound;
    /** @brief 地震波到达本地音效播放次数*/
    int repeatsSWaveArriveSound;
    /** @brief 本地地震预警音效播放次数*/
    int repeatsAlertSound;
    /** @brief 全球地震预警音效播放次数*/
    int repeatsNewRecordSound;
    /** @brief 本地严重预警音效播放次数*/
    int repeatsCriticalAlertSound;
    /** @brief 预警更新音效播放次数*/
    int repeatsUpdateAlertSound;
    /** @brief 预警更新音效播放次数*/
    int repeatsWeakShakeSound;
    /** @brief 弱摇晃音效播放次数*/
    int repeatsMidShakeSound;
    /** @brief 强烈摇晃音效播放次数*/
    int repeatsStrongShakeSound;
    /** @brief 程序启动后是否显示窗口*/
    bool showWindowOnStartup;
    /** @brief 程序窗口位置*/
    QRect windowRect;
    /** @brief 程序是否最大化*/
    bool windowMaximized;
    /** @brief 用户输入的其他配置信息*/
    QString userExtra;
    /** @brief 用户指定的Mapbox Access Token*/
    QString mapboxAccessToken;
    /** @brief 用户指定的Mapbox Style*/
    QString mapboxStyle;
    /** @brief 显示地震列表*/
    bool showHistoryList;
    /** @brief 显示状态栏*/
    bool showStatusBar;
    /** @brief 显示图例*/
    bool showLegends;
    /** @brief 使用的地震历史记录API*/
    HISTORY_DATA_SOURCE usingHistoryDataSource;
    /** @brief 使用的地震预警API*/
    EEW_DATA_SOURCE usingEEWDataSource;
    /** @brief 使用的测站API*/
    STATION_DATA_SOURCE usingStationDataSource;
    /** @brief 调试使用的地震历史记录API*/
    bool debugUsingHistoryDataSource;
    /** @brief 调试使用的地震预警API*/
    bool debugUsingEEWDataSource;
    /** @brief 调试使用的测站API*/
    bool debugUsingStationDataSource;
    /** @brief 在地图上显示LOGO*/
    bool showLogoOnMap;
    /** @brief 在地图上显示预估烈度*/
    bool showEstimatedIntensity;
    /** @brief 在收到新的地震记录后显示烈度分布*/
    bool showIntensityOnNewHistory;
    /** @brief 网络读取超时（毫秒）*/
    int timeoutNetworkReadMS;
    /** @brief 数据延迟后需要重启的时间（秒），0表示不执行此操作*/
    int timeoutDataDelayRestartSec;
    /** @brief 是否将日志输出到文件*/
    bool logToFile;
    /** @brief 启动后清空之前的Log*/
    bool clearLogsOnStartup;
    /** @brief 测站数据的存储时间*/
    int stationDataStoreTimeSec;
    /** @brief 不要自动切换列表*/
    bool dontAutoSwitchTab;
    /** @brief 启用测站数据*/
    bool enableStationData;
    double pwaveSpeed;
    double swaveSpeed;
    double earthAvgRadius;
private:
    QString RectToString(const QRect&r);
    QRect StringToRect(const QString&s);
};

#endif // EEWSETTINGS_H
