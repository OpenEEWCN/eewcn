#ifndef SETHOMEDIALOG_H
#define SETHOMEDIALOG_H

#include <QDialog>

namespace Ui {
class SetHomeDialog;
}

class SetHomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetHomeDialog(QWidget *parent = nullptr);
    ~SetHomeDialog();
    void SetPosition(double lat,double lng);
    double GetLatitude();
    double GetLongitude();

private:
    Ui::SetHomeDialog *ui;
};

#endif // SETHOMEDIALOG_H
