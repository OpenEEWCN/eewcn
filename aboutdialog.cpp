#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "value.h"
#include "eewsettings.h"
#include <QIcon>
#include <QSysInfo>
#include <QDateTime>
#include <QTimeZone>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setFont(QFont(Value::getLanguageFontName(EEWSettings::GetSettings()->language)));
    QString sysinfo=QSysInfo::prettyProductName()+" "+QSysInfo::currentCpuArchitecture()+" "+QDateTime::currentDateTime().timeZone().id();
    ui->labelText->setText(tr("<html><p>EEW CN<br>%1</p><p>By: <a href=\"https://OpenEEWCN.github.io/eewcn-res/link.htm?key=OpenEEWCN\">OpenEEWCN</a></p>"
                              "<p>Build Time: %2<br>* All times in this application are UTC+8.</p>"
                              "<p>Data Source:<br><a href=\"https://OpenEEWCN.github.io/eewcn-res/link.htm?key=CENC\">CENC</a><br><a href=\"https://OpenEEWCN.github.io/eewcn-res/link.htm?key=SichuanEA\">Sichuan Earthquake Administration</a><br><a href=\"https://OpenEEWCN.github.io/eewcn-res/link.htm?key=FujianEA\">Fujian Earthquake Agency</a><br><a href=\"https://OpenEEWCN.github.io/eewcn-res/link.htm?key=Wolfx\">Wolfx</a><p>"
                              "<p>Map Data:<br><a href=\"https://OpenEEWCN.github.io/eewcn-res/link.htm?key=Mapbox\">Mapbox</a><br><a href=\"https://OpenEEWCN.github.io/eewcn-res/link.htm?key=AliyunDataVGeoAtlas\">Aliyun DataV.GeoAtlas</a><p>"
                              "<p>Reference:<br>Intensity Colors: <a href=\"https://OpenEEWCN.github.io/eewcn-res/link.htm?key=CEIV\">CEIV</a><br>Intensity Calculation: <a href=\"https://OpenEEWCN.github.io/eewcn-res/link.htm?key=SCEEW\">SCEEW</a></p>"
                              "<p>Fonts:<br><a href=\"https://OpenEEWCN.github.io/eewcn-res/link.htm?key=SarasaGothic\">Sarasa-Gothic</a></p>"
                              "<p><a href=\"https://OpenEEWCN.github.io/eewcn-res/link.htm?key=feedback\">Bug report or feedback</a></p>"
                              "<p><a href=\"https://OpenEEWCN.github.io/eewcn-res/link.htm?key=DownloadLatest\">Click here to download latest version</a></p></html>")
                           .arg(VERSION_NAME,
                           Value::timestampMSToChineseDateTimeFormat(Value::getCompileTimeMS())));
    ui->labelIcon->setPixmap(QIcon("Media/eewcn.ico").pixmap(128,128));
    adjustSize();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
