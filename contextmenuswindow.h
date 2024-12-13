#ifndef CONTEXTMENUSWINDOW_H
#define CONTEXTMENUSWINDOW_H

#include <QMainWindow>

namespace Ui {
class ContextMenusWindow;
}

class ContextMenusWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ContextMenusWindow(QWidget *parent);
    ~ContextMenusWindow();
    void SetTestEEWVisible(bool);
    void SetActionShowHistoryChecked(bool checked);
    void SetActionShowStatusBarChecked(bool checked);
    void SetActionShowLegendsChecked(bool checked);
    void TrackWindowMenu();
    void TrackWindowMenu(const QPoint&p);
    void TrackTrayMenu();
    void TrackTrayMenu(const QPoint&p);
    void TrackListMenu();
    void TrackListMenu(const QPoint&p);
    QMenu*TrayMenu();

signals:
    void SignalSettingsChanged();

private slots:
    void on_actionShow_Earthquakes_History_triggered(bool checked);

    void on_action_Settings_triggered();

    void on_action_Quit_triggered();

    void on_actionShow_Main_Window_triggered();

    void on_actionTo_Last_Epi_center_triggered();

    void on_actionTo_Home_triggered();

    void on_action_About_triggered();

    void on_action_Test_EEW_triggered();

    void on_actionSet_H_ome_triggered();

    void on_actionShow_L_egends_triggered(bool checked);

    void on_actionShow_Status_Bar_triggered(bool checked);

    void on_actionView_Intensity_on_Map_triggered();

private:
    Ui::ContextMenusWindow *ui;
    QPoint popupMenuPos;
};

#endif // CONTEXTMENUSWINDOW_H
