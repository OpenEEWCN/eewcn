#ifndef EEWSOUNDS_H
#define EEWSOUNDS_H

#include <QSoundEffect>
#include <QMap>


class EEWSounds
{
private:
    EEWSounds();
    ~EEWSounds();
#if defined( Q_OS_MAC )||defined( Q_OS_ANDROID )||defined( Q_OS_IOS )
    static EEWSounds *eewSounds;
#else
    static EEWSounds eewSounds;
#endif
    QSoundEffect seAlert,seArrive,seNewRecord,seCritical,seUpdate,seWeak,seMid,seStrong;//栈空间就不用指定parent了；经测试，WAV音频最好使用16位整形格式，否则可能有BUG）
public:
    static EEWSounds*GetSounds();
    void PlayAlertSound();
    void PlaySWaveArriveSound();
    void PlayNewRecordSound();
    void PlayCriticalAlertSound();
    void PlayUpdateAlertSound();
    void PlayWeakShakeSound();
    void PlayMidShakeSound();
    void PlayStrongShakeSound();
    void TestPlayAlertSound(const QString&path,int repeats);
    void TestPlaySWaveArriveSound(const QString&path,int repeats);
    void TestPlayNewRecordSound(const QString&path,int repeats);
    void TestPlayCriticalAlertSound(const QString&path,int repeats);
    void TestPlayUpdateAlertSound(const QString&path,int repeats);
    void TestPlayWeakShakeSound(const QString&path,int repeats);
    void TestPlayMidShakeSound(const QString&path,int repeats);
    void TestPlayStrongShakeSound(const QString&path,int repeats);
};

#endif // EEWSOUNDS_H
