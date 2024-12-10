#include "testeewdialog.h"
#include "ui_testeewdialog.h"
#include "value.h"
#include "eewsettings.h"

TestEEWDialog::TestEEWDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestEEWDialog)
{
    ui->setupUi(this);
    setFont(QFont(Value::getLanguageFontName(EEWSettings::GetSettings()->language)));
    adjustSize();
}

TestEEWDialog::~TestEEWDialog()
{
    delete ui;
}

void TestEEWDialog::SetPosition(double lat, double lng)
{
    ui->spinLatitude->setValue(lat);
    ui->spinLongitude->setValue(lng);
}

double TestEEWDialog::GetLongitude()
{
    return ui->spinLongitude->value();
}

double TestEEWDialog::GetLatitude()
{
    return ui->spinLatitude->value();
}

double TestEEWDialog::GetMagnitude()
{
    return ui->spinMagnitude->value();
}

double TestEEWDialog::GetDepth()
{
    return ui->spinDepth->value();
}

QString TestEEWDialog::GetLocation()
{
    return ui->editLocation->text();
}
