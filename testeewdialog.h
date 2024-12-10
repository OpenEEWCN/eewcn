#ifndef TESTEEWDIALOG_H
#define TESTEEWDIALOG_H

#include <QDialog>

namespace Ui {
class TestEEWDialog;
}

class TestEEWDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TestEEWDialog(QWidget *parent = nullptr);
    ~TestEEWDialog();
    void SetPosition(double lat,double lng);
    double GetLongitude();
    double GetLatitude();
    double GetMagnitude();
    double GetDepth();
    QString GetLocation();

private:
    Ui::TestEEWDialog *ui;
};

#endif // TESTEEWDIALOG_H
