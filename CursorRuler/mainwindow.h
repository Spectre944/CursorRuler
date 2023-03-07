#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QMenu>
#include <QPainter>
#include <QPen>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QLockFile>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

#include <iostream>
#include <stdio.h>

#include "windows.h" // Подключаем библиотеку WinAPI
#pragma comment(lib, "user32.lib")

#define ANIM_TIMEOUT 100 //Refresh rate of projectile draw

struct Point {
    int x = 0;
    int y = 0;

    Point& operator =(QPoint that)
    {
        x = that.x();
        y = that.y();

        return *this;
    }

    Point& operator-=( const Point& that )
    {
       x -= that.x;
       y -= that.y;
       return *this;
    }

    Point operator-(const Point& that) const
    {
       Point result;
       result.x = this->x - that.x;
       result.y = this->y - that.y;
       return result;
    }

};



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void mainHandler(int);

    void calculateScale();
    void calculateDistance();
    void calculateProjectile();
    void drawTrajectory();

    Point   Point1, Point2,                                 // Point1, Point2 - click points for distance
            PointScale1, PointScale2,                       // and PointScale1, PointScale2 - points for scale
            PointVerticalGrid1, PointVerticalGrid2,         // PointVerticalGrid1, PointVerticalGrid2 point for transportier (vertical shooting)
            PointProjectileDraw1, PointProjectileDraw2;     // PointProjectileDraw1, PointProjectileDraw2 drawing point of projectile

    double  scale = 0;
    double  ProjectileSpeed,
            GunDepressionAngle,
            GunDepressionAngleVertDeg,
            DistanceBetwenPoints,
            ProjectileTimeOfFly,
            AzimutDeg,
            AzimutRad;

    bool onTop = true;
    bool onTransperentMouse = false;




private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);


    void on_checkBoxST_ComplexCalculations_clicked();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;

    QMenu *trayIconMenu;
    QAction *quitAction;
    QScreen *screen0;
    QTimer *animationTimer;



    HHOOK hhkLowLevelKybd;
    bool bWinKey;
    HHOOK hHook = NULL;
protected:


    virtual void closeEvent(QCloseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;


protected:

    //for hot key detection
    virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;


};



#endif // MAINWINDOW_H
