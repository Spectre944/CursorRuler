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
#include <QDebug>

#include <iostream>
#include <stdio.h>

#include "windows.h" // Подключаем библиотеку WinAPI
#pragma comment(lib, "user32.lib")

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

    // p1, p2 - click points for distance
    // and ps1, ps2 - points for scale
    // pv1, pv2 point for transportier (vertical shooting)
    Point p1, p2, ps1, ps2, pv1, pv2;
    double scale = 0;
    double Vspeed, Oangle, OangleVertDeg, Ddistance, Tfly;

    bool onTop = true;
    bool onTransperentMouse = false;




private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);


private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;

    QMenu *trayIconMenu;
    QAction *quitAction;
    QScreen *screen0;
    QPainter *projPainter;
    QPen *projPen;
    QPropertyAnimation *animation;
    //For tracking mouse coordinates
    //QPoint *m_startPoint = new QPoint;


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
