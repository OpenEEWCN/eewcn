#ifndef EARTHQUAKELISTENTRYFORM_H
#define EARTHQUAKELISTENTRYFORM_H

#include <QWidget>

namespace Ui {
class EarthquakeListEntryForm;
}

class EarthquakeListEntryForm : public QWidget
{
    Q_OBJECT

public:
    explicit EarthquakeListEntryForm(QWidget *parent = nullptr);
    ~EarthquakeListEntryForm();
    void SetData(const QString&location,const QString&timeStr,double magnitude,
                 double depth,double latitude,double longitude,QObject*quickWidget);
    void SetStationData(const QString&location,const QString&timeStr,double pga,double pgv,double pgd,double intensity,
                 double height,double latitude,double longitude,QObject*quickWidget);
    void ExpandHeight();
    void SetHead(bool head);
    void SetMapCenter();
    void ShowIntensityOnMap();
    double GetIntensity();

signals:
    void SignalSetMapCenter(double lat,double lng);
    void SignalShowIntensityOnMap(double lat,double lng,double mag,double depth);

private:
    Ui::EarthquakeListEntryForm *ui;
    bool isHead;
    double m_longitude,m_latitude,m_magnitude,m_depth,m_intensity;
};

#endif // EARTHQUAKELISTENTRYFORM_H
