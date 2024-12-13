#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QStatusBar>


class Logger : public QObject
{
    Q_OBJECT
public:
    static Logger* getLogger();
    bool writelog(const QString &tag,const QString &s,bool showOnStatus=true);
    bool info(const QString &s,bool showOnStatus=true);
    bool warn(const QString &s,bool showOnStatus=true);
    bool error(const QString &s,bool showOnStatus=true);
    void setStatusBar(QStatusBar *statusBar);
    void setWriteToFile(bool enabled);
    bool clearLogFile();
private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();
    QStatusBar *logStatusBar;
    bool writeToFile;
    static Logger sLogger;
};

#endif // LOGGER_H
