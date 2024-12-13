#include "contextmenuswindow.h"
#include "ui_contextmenuswindow.h"
#include "mainwindow.h"
#include "settingsdialog.h"
#include "aboutdialog.h"
#include <QMessageBox>
#include <QContextMenuEvent>

ContextMenusWindow::ContextMenusWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ContextMenusWindow)
{
    ui->setupUi(this);
}

ContextMenusWindow::~ContextMenusWindow()
{
    delete ui;
}

void ContextMenusWindow::on_actionShow_Earthquakes_History_triggered(bool checked)
{
    ((MainWindow*)parent())->SetShowHistoryWindow(checked);
}

void ContextMenusWindow::on_action_Settings_triggered()
{
    SettingsDialog dlg(this);
    dlg.LoadSettings();
    if(dlg.exec()==QDialog::Accepted){
        dlg.SaveSettings();
        emit SignalSettingsChanged();
    }
}

void ContextMenusWindow::on_action_Quit_triggered()
{
    ((MainWindow*)parent())->QuitApp();
}

void ContextMenusWindow::on_actionShow_Main_Window_triggered()
{
    ((MainWindow*)parent())->show();
}

void ContextMenusWindow::SetActionShowHistoryChecked(bool checked)
{
    ui->actionShow_Earthquakes_History->setChecked(checked);
}

void ContextMenusWindow::TrackTrayMenu()
{
    TrackTrayMenu(cursor().pos());
}

void ContextMenusWindow::TrackTrayMenu(const QPoint&p)
{
    ui->menu_Tray_Menu->popup(p);
}

void ContextMenusWindow::TrackWindowMenu()
{
    TrackWindowMenu(cursor().pos());
}

void ContextMenusWindow::TrackWindowMenu(const QPoint&p)
{
    popupMenuPos=p;
    ui->menu_Window_Menu->popup(p);
}

void ContextMenusWindow::TrackListMenu()
{
    TrackListMenu(cursor().pos());
}

void ContextMenusWindow::TrackListMenu(const QPoint &p)
{
    popupMenuPos=p;
    ui->menuList_Menu->popup(p);
}

void ContextMenusWindow::on_actionTo_Last_Epi_center_triggered()
{
    ((MainWindow*)parent())->SetMapCenterToLastEEW();
}

void ContextMenusWindow::on_actionTo_Home_triggered()
{
    ((MainWindow*)parent())->SetMapCenterToHome();
}

void ContextMenusWindow::on_action_About_triggered()
{
    AboutDialog((MainWindow*)parent()).exec();
}

void ContextMenusWindow::on_action_Test_EEW_triggered()
{
    ((MainWindow*)parent())->OpenTestEEWDialog(popupMenuPos);
}

void ContextMenusWindow::on_actionSet_H_ome_triggered()
{
    ((MainWindow*)parent())->OpenSetHomeDialog(popupMenuPos);
}

void ContextMenusWindow::SetTestEEWVisible(bool v){
    ui->action_Test_EEW->setVisible(v);
}

void ContextMenusWindow::on_actionShow_L_egends_triggered(bool checked)
{
    ((MainWindow*)parent())->SetShowLegends(checked);
}

void ContextMenusWindow::SetActionShowLegendsChecked(bool checked)
{
    ui->actionShow_L_egends->setChecked(checked);
}

void ContextMenusWindow::on_actionShow_Status_Bar_triggered(bool checked)
{
    ((MainWindow*)parent())->SetShowStatusBar(checked);
}

void ContextMenusWindow::SetActionShowStatusBarChecked(bool checked)
{
    ui->actionShow_Status_Bar->setChecked(checked);
}

void ContextMenusWindow::on_actionView_Intensity_on_Map_triggered()
{
    ((MainWindow*)parent())->OnMenuCommandViewIntensity(popupMenuPos);
}

QMenu*ContextMenusWindow::TrayMenu()
{
    return ui->menu_Tray_Menu;
}
