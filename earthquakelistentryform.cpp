#include "earthquakelistentryform.h"
#include "ui_earthquakelistentryform.h"
#include "value.h"
#include "eewsettings.h"

EarthquakeListEntryForm::EarthquakeListEntryForm(QWidget *parent) :
    QWidget(parent),ui(new Ui::EarthquakeListEntryForm),isHead(false),m_intensity(0.0)
{
    ui->setupUi(this);
    setFont(QFont(Value::getLanguageFontName(EEWSettings::GetSettings()->language)));
}

EarthquakeListEntryForm::~EarthquakeListEntryForm()
{
    delete ui;
}

void EarthquakeListEntryForm::SetData(const QString&location,const QString&timeStr,double magnitude,double depth,double latitude,double longitude,QObject*quickWidget)
{
    ui->labelLocation->setText(location);
    ui->labelMagnitude->setText(QString("M%1").arg(magnitude));
    ui->labelTime->setText(timeStr);
    ui->labelDepth->setText(QString("%1km").arg(depth));
    double intensity=Value::calcMaxInt(magnitude,depth,longitude,latitude);
    m_intensity=intensity;
    ui->labelIntensity->setText(QString("%1").arg(intensity));
    QVariant rTextColor,rBkColor;
    QMetaObject::invokeMethod(quickWidget,"getIntTextColors",Q_RETURN_ARG(QVariant,rTextColor),Q_ARG(QVariant,intensity));
    QMetaObject::invokeMethod(quickWidget,"getIntColors",Q_RETURN_ARG(QVariant,rBkColor),Q_ARG(QVariant,intensity));
    ui->labelIntensity->setStyleSheet(QString("color:%1;background-color:%2;font-family:Sarasa Mono SC%3")
                                          .arg(intensity<=0.0?"black":rTextColor.toString(),rBkColor.toString()).arg(intensity<=0.0?";border:1px solid":""));
    m_longitude=longitude;
    m_latitude=latitude;
    m_magnitude=magnitude;
    m_depth=depth;
    EEWSettings*eewSettings=EEWSettings::GetSettings();
    double localInt=Value::getIntensityAt(magnitude,depth,longitude,latitude,eewSettings->userLongitude,eewSettings->userLatitude);
    setToolTip(tr("Time: %1\nLocation: %2\nLatitude: %3, Longitude: %4\nMagnitude: %5\nDepth: %6km\nDistance: %7km\nEpicenter Intensity: %8\nLocal Intensity: %9")
               .arg(timeStr,location).arg(latitude).arg(longitude).arg(magnitude).arg(depth)
               .arg(Value::getDistanceSurface(longitude,latitude,eewSettings->userLongitude,eewSettings->userLatitude)).arg(intensity).arg(localInt));
}

void EarthquakeListEntryForm::SetStationData(const QString&location,const QString&timeStr,double pga,double pgv,double pgd,double intensity,
                    double height,double latitude,double longitude,QObject*quickWidget)
{
    ui->labelLocation->setText(location);
    ui->labelMagnitude->setText(QString::asprintf("PGA: %.6f gal\nPGV: %.6f \u339D/s\nPGD: %.6f \u339D",pga,pgv,pgd));
    ui->labelTime->setText(timeStr);
    ui->labelDepth->setText(QString("%1m").arg(height));
    ui->labelIntensity->setText(QString("%1").arg(intensity));
    QVariant rTextColor,rBkColor;
    QMetaObject::invokeMethod(quickWidget,"getIntTextColors",Q_RETURN_ARG(QVariant,rTextColor),Q_ARG(QVariant,intensity));
    QMetaObject::invokeMethod(quickWidget,"getIntColors",Q_RETURN_ARG(QVariant,rBkColor),Q_ARG(QVariant,intensity));
    ui->labelIntensity->setStyleSheet(QString("color:%1;background-color:%2;font-family:Sarasa Mono SC%3")
                                          .arg(intensity<=0.0?"black":rTextColor.toString(),rBkColor.toString()).arg(intensity<=0.0?";border:1px solid":""));
    m_longitude=longitude;
    m_latitude=latitude;
    m_depth=height;
    m_intensity=intensity;
    EEWSettings*eewSettings=EEWSettings::GetSettings();
    setToolTip(tr("Location: %1\nLatitude: %2, Longitude: %3\nHeight: %4m\nDistance: %5km\nIntensity: %6")
                   .arg(location).arg(latitude).arg(longitude).arg(height)
                   .arg(Value::getDistanceSurface(longitude,latitude,eewSettings->userLongitude,eewSettings->userLatitude)).arg(intensity));
}

void EarthquakeListEntryForm::ExpandHeight()
{
    int mw=width();
    int a,b,c,d;
    layout()->getContentsMargins(&a,&b,&c,&d);
    mw=mw-a-c-layout()->spacing()-ui->labelIntensity->width();
    ui->verticalLayoutDetail->getContentsMargins(&a,&b,&c,&d);
    mw=mw-a-c;
    ui->labelLocation->setMinimumWidth(mw);
    ui->labelTime->setMinimumWidth(mw);
    adjustSize();
    adjustSize();
}

void EarthquakeListEntryForm::SetHead(bool head)
{
    isHead=head;
    if(isHead){
        ui->labelIntensity->setMinimumWidth(ui->labelIntensity->minimumWidth()*4/3);
        ui->labelIntensity->setMaximumWidth(ui->labelIntensity->minimumWidth());
        ui->labelIntensity->setMinimumHeight(ui->labelIntensity->minimumWidth());
        ui->labelIntensity->setMaximumHeight(ui->labelIntensity->minimumWidth());
    }else{
        ui->verticalLayoutDetail->removeWidget(ui->labelTime);
        ui->horizontalLayoutIntDep->insertWidget(0,ui->labelTime);
        ui->labelDepth->hide();
    }
    QFont f=ui->labelIntensity->font();
    f.setPixelSize(ui->labelIntensity->minimumWidth()*4/5);
    ui->labelIntensity->setFont(f);
}

void EarthquakeListEntryForm::SetMapCenter()
{
    if(m_latitude!=0.0||m_longitude!=0.0)
        emit SignalSetMapCenter(m_latitude,m_longitude);
}

void EarthquakeListEntryForm::ShowIntensityOnMap()
{
    if(m_latitude!=0.0||m_longitude!=0.0)
        emit SignalShowIntensityOnMap(m_latitude,m_longitude,m_magnitude,m_depth);
}

double EarthquakeListEntryForm::GetIntensity()
{
    return m_intensity;
}
