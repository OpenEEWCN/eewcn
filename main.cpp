#include "mainwindow.h"
#include "eewsettings.h"
#include "value.h"

#include <QApplication>
#include <QFontDatabase>
#include <QTranslator>
#include <QDir>

#ifdef Q_OS_ANDROID
int ExtractAssets(QString fromPath,QString toPath)
{
    QDir t(toPath);
    QFileInfo fi(fromPath);
    //该path是文件
    if(fi.isFile()){
        if(QFile::copy(fromPath,toPath))
            return true;
        else{
            qDebug()<<"copy failed:"<<fromPath;
            return false;
        }
    }
    //该path是目录，遍历
    QDir d(fromPath);
    if(!t.mkpath(".")){
        qDebug()<<"mkpath failed:"<<toPath;
    }
    foreach (QString p, d.entryList()) {
        //跳过“.”开头的路径
        if(p[0]!='.'){
            ExtractAssets(fromPath+"/"+p,toPath+"/"+p);
        }
    }
    return 0;
}
#endif

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
#if defined( Q_OS_MAC ) && !defined( QT_DEBUG )
    QDir::setCurrent(QCoreApplication::applicationDirPath()+"/../../..");
#elif defined( Q_OS_ANDROID )
    if(QDir().exists("Media"))
        qDebug()<<"Assets exists.";
    else
        ExtractAssets("assets:/",".");
#endif
    QStringList fontPaths={
        "Fonts/sarasa-bold.ttc",
        //"Fonts/sarasa-extralight.ttc",
        //"Fonts/sarasa-light.ttc",
        "Fonts/sarasa-regular.ttc",
        //"Fonts/sarasa-semibold.ttc"
    };
    //注意应当在QApplication之后执行
    foreach (QString p, fontPaths) {
#ifdef Q_OS_MAC
        if(QFontDatabase::addApplicationFont(QDir::currentPath()+"/"+p)==-1)
#else
        if(QFontDatabase::addApplicationFont(p)==-1)
#endif
            qDebug()<<"Cannot load font:"<<p;
    }
    EEWSettings::GetSettings()->LoadSettings();
    int langRegionIndex=(int)EEWSettings::GetSettings()->language;
    QTranslator tApp,tQt;
    if(tApp.load("eewcn_"+Value::langRegionTag[langRegionIndex]))
        a.installTranslator(&tApp);
    if(tQt.load("translations/qt_"+Value::langRegionTag[langRegionIndex]))
        a.installTranslator(&tQt);
    MainWindow w;
    w.show();
    w.ShowOnStartup();
    if(argc>1&&strcmp(argv[1],"-hide")==0)
        w.hide();
    return a.exec();
}
