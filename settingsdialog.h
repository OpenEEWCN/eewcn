#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();
    void LoadSettings();
    void SaveSettings();
    void SetEEWUsingCustom();
    void SetHistoryUsingCustom();
    void SetStationUsingCustom();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

    void on_buttonPathLocalAlert_clicked();

    void on_buttonPathLocalArrive_clicked();

    void on_buttonPathGlobalAlert_clicked();

    void on_buttonTestLocalAlert_clicked();

    void on_buttonTestLocalArrive_clicked();

    void on_buttonGlobalAlert_clicked();

    void on_buttonPathCriticalAlert_clicked();

    void on_buttonPathAlertUpdate_clicked();

    void on_buttonTestCriticalAlert_clicked();

    void on_buttonTestAlertUpdate_clicked();

    void ResizeWindow();
    void on_buttonPathWeakShakeSound_clicked();

    void on_buttonPathMidShakeSound_clicked();

    void on_buttonPathStrongShakeSound_clicked();

    void on_buttonTestWeakShakeSound_clicked();

    void on_buttonTestMidShakeSound_clicked();

    void on_buttonTestStrongShakeSound_clicked();

private:
    Ui::SettingsDialog *ui;
    bool isAutorun();
    void setAutorun(bool enable);
signals:
    void SignalResizeWindow();
};

#endif // SETTINGSDIALOG_H
