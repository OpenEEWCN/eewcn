#include "logger.h"
#include "value.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

#define LOG_FILE "log.txt"

Logger Logger::sLogger;

Logger::Logger(QObject *parent) : QObject(parent),
    logStatusBar(nullptr), writeToFile(true)
{
    qDebug()<<"Logger initialized.";
}

Logger::~Logger()
{
    qDebug()<<"Logger released.";
}

Logger* Logger::getLogger()
{
    return &sLogger;
}

bool Logger::writelog(const QString &tag,const QString &s,bool showOnStatus)
{
    if(logStatusBar&&showOnStatus)
        logStatusBar->showMessage(s);
    QString dt=Value::timestampMSToChineseDateTimeFormat(QDateTime::currentMSecsSinceEpoch());
    qDebug("%s[%s]%s",dt.toUtf8().data(),tag.toUtf8().data(),s.toUtf8().data());
    if(writeToFile){
        QString path=Value::getAppDataPath(LOG_FILE);
        if(path.isEmpty())
            return false;
        QFile f(path);
        if(!f.open(QFile::Append|QFile::Text))
            return false;
        QTextStream fis(&f);
        fis.setCodec("UTF-8");
        fis<<dt<<"["<<tag<<"]"<<s<<Qt::endl;
    }
    return true;
}

bool Logger::info(const QString &s,bool showOnStatus)
{
    return writelog("INFO",s,showOnStatus);
}

bool Logger::warn(const QString &s,bool showOnStatus)
{
    return writelog("WARN",s,showOnStatus);
}

bool Logger::error(const QString &s,bool showOnStatus)
{
    return writelog("ERROR",s,showOnStatus);
}

void Logger::setStatusBar(QStatusBar *statusBar)
{
    logStatusBar=statusBar;
}

void Logger::setWriteToFile(bool enabled)
{
    writeToFile=enabled;
}

bool Logger::clearLogFile()
{
    QString path=Value::getAppDataPath(LOG_FILE,false);
    if(path.isEmpty())
        return true;
    return QFile::remove(path);
}
