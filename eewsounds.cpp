#include "eewsounds.h"
#include "eewsettings.h"

#if defined( Q_OS_MAC )||defined( Q_OS_ANDROID )||defined( Q_OS_IOS )
EEWSounds* EEWSounds::eewSounds=nullptr;
#else
EEWSounds EEWSounds::eewSounds;
#endif

EEWSounds::EEWSounds()
{
}

EEWSounds::~EEWSounds()
{
}

EEWSounds*EEWSounds::GetSounds()
{
#if defined( Q_OS_MAC )||defined( Q_OS_ANDROID )||defined( Q_OS_IOS )
    if(eewSounds==nullptr)
        eewSounds=new EEWSounds();
    return eewSounds;
#else
    return &eewSounds;
#endif
}

void EEWSounds::PlayAlertSound()
{
    EEWSettings*eewSettings=EEWSettings::GetSettings();
    TestPlayAlertSound(eewSettings->pathAlertSound,eewSettings->repeatsAlertSound);
}

void EEWSounds::PlaySWaveArriveSound()
{
    EEWSettings*eewSettings=EEWSettings::GetSettings();
    TestPlaySWaveArriveSound(eewSettings->pathSWaveArriveSound,eewSettings->repeatsSWaveArriveSound);
}

void EEWSounds::PlayNewRecordSound()
{
    EEWSettings*eewSettings=EEWSettings::GetSettings();
    TestPlayNewRecordSound(eewSettings->pathNewRecordSound,eewSettings->repeatsNewRecordSound);
}

void EEWSounds::PlayCriticalAlertSound()
{
    EEWSettings*eewSettings=EEWSettings::GetSettings();
    TestPlayCriticalAlertSound(eewSettings->pathCriticalAlertSound,eewSettings->repeatsCriticalAlertSound);
}

void EEWSounds::PlayUpdateAlertSound()
{
    EEWSettings*eewSettings=EEWSettings::GetSettings();
    TestPlayUpdateAlertSound(eewSettings->pathUpdateAlertSound,eewSettings->repeatsUpdateAlertSound);
}

void EEWSounds::PlayWeakShakeSound()
{
    EEWSettings*eewSettings=EEWSettings::GetSettings();
    if(!seWeak.isPlaying())
        TestPlayWeakShakeSound(eewSettings->pathWeakShakeSound,eewSettings->repeatsWeakShakeSound);
}

void EEWSounds::PlayMidShakeSound()
{
    EEWSettings*eewSettings=EEWSettings::GetSettings();
    if(!seMid.isPlaying())
        TestPlayMidShakeSound(eewSettings->pathMidShakeSound,eewSettings->repeatsMidShakeSound);
}

void EEWSounds::PlayStrongShakeSound()
{
    EEWSettings*eewSettings=EEWSettings::GetSettings();
    if(!seStrong.isPlaying())
        TestPlayStrongShakeSound(eewSettings->pathStrongShakeSound,eewSettings->repeatsStrongShakeSound);
}

void EEWSounds::TestPlayAlertSound(const QString &path, int repeats)
{
    if(repeats>0){
        seAlert.setLoopCount(repeats);
        seAlert.setSource(QUrl::fromLocalFile(path));
        seAlert.play();
    }
}

void EEWSounds::TestPlayNewRecordSound(const QString &path, int repeats)
{
    if(repeats>0){
        seNewRecord.setLoopCount(repeats);
        seNewRecord.setSource(QUrl::fromLocalFile(path));
        seNewRecord.play();
    }
}

void EEWSounds::TestPlaySWaveArriveSound(const QString &path, int repeats)
{
    if(repeats>0){
        seArrive.setLoopCount(repeats);
        seArrive.setSource(QUrl::fromLocalFile(path));
        seArrive.play();
    }
}

void EEWSounds::TestPlayCriticalAlertSound(const QString &path, int repeats)
{
    if(repeats>0){
        seCritical.setLoopCount(repeats);
        seCritical.setSource(QUrl::fromLocalFile(path));
        seCritical.play();
    }
}

void EEWSounds::TestPlayUpdateAlertSound(const QString &path, int repeats)
{
    if(repeats>0){
        seUpdate.setLoopCount(repeats);
        seUpdate.setSource(QUrl::fromLocalFile(path));
        seUpdate.play();
    }
}

void EEWSounds::TestPlayWeakShakeSound(const QString &path, int repeats)
{
    if(repeats>0){
        seWeak.setLoopCount(repeats);
        seWeak.setSource(QUrl::fromLocalFile(path));
        seWeak.play();
    }
}

void EEWSounds::TestPlayMidShakeSound(const QString &path, int repeats)
{
    if(repeats>0){
        seMid.setLoopCount(repeats);
        seMid.setSource(QUrl::fromLocalFile(path));
        seMid.play();
    }
}

void EEWSounds::TestPlayStrongShakeSound(const QString &path, int repeats)
{
    if(repeats>0){
        seStrong.setLoopCount(repeats);
        seStrong.setSource(QUrl::fromLocalFile(path));
        seStrong.play();
    }
}
