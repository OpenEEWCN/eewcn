#include "sethomedialog.h"
#include "ui_sethomedialog.h"
#include "value.h"
#include "eewsettings.h"

SetHomeDialog::SetHomeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetHomeDialog)
{
    ui->setupUi(this);
    setFont(QFont(Value::getLanguageFontName(EEWSettings::GetSettings()->language)));
    adjustSize();
}

SetHomeDialog::~SetHomeDialog()
{
    delete ui;
}

void SetHomeDialog::SetPosition(double lat, double lng)
{
    ui->spinLatitude->setValue(lat);
    ui->spinLongitude->setValue(lng);
}

double SetHomeDialog::GetLatitude()
{
    return ui->spinLatitude->value();
}

double SetHomeDialog::GetLongitude()
{
    return ui->spinLongitude->value();
}
