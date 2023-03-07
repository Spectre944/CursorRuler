#include "mainwindow.h"
#include "ui_mainwindow.h"

// Crosslink between Qt class and win callback
MainWindow * mwReference;

//Keyboard hook
LRESULT CALLBACK LowLevelKeyboardMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    DWORD SHIFT_key=0;
    DWORD CTRL_key=0;
    DWORD ALT_key=0;


    if  ((nCode == HC_ACTION) &&   ((wParam == WM_SYSKEYDOWN) ||  (wParam == WM_KEYDOWN) || (wParam == WM_RBUTTONDOWN) || (wParam == WM_LBUTTONDOWN)))
    {
        KBDLLHOOKSTRUCT hooked_key =    *((KBDLLHOOKSTRUCT*)lParam);
        DWORD dwMsg = 1;
        dwMsg += hooked_key.scanCode << 16;
        dwMsg += hooked_key.flags << 24;
        char lpszKeyName[1024] = {0};

        int key = hooked_key.vkCode;

        SHIFT_key = GetAsyncKeyState(VK_SHIFT);
        CTRL_key = GetAsyncKeyState(VK_CONTROL);
        ALT_key = GetAsyncKeyState(VK_MENU);

        /*
        qDebug() << wParam;
        qDebug() << key;
        */

        if  (GetAsyncKeyState(VK_SHIFT)>= 0) key +=32;

        /*********************************************
         ***   Hotkey scope                         ***
         ***   do stuff here                        ***
         **********************************************/

        if (CTRL_key !=0 && key == 'x' )
        {
            //clear Point1 Point2 line
            mwReference->mainHandler(0);
            CTRL_key=0;

        }
        if (CTRL_key !=0 && key == 'z' )
        {
            //clear PointScale1 PointScale2 line
            mwReference->mainHandler(1);
            CTRL_key=0;

        }

        if (CTRL_key !=0 && key == 'z' )
        {
            //clear PointScale1 PointScale2 line
            mwReference->mainHandler(1);
            CTRL_key=0;

        }

        if (ALT_key != 0 && wParam == WM_RBUTTONDOWN )
        {
            //set coordintats Point1 Point2  // First click Point1, Second Point2
            mwReference->mainHandler(2);
            ALT_key = 0;
        }

        if (CTRL_key != 0 && wParam == WM_RBUTTONDOWN )
        {
            //set coordintats PointScale1 PointScale2  // First click Point1, Second Point2
            mwReference->mainHandler(3);
            CTRL_key = 0;
        }


        if (ALT_key != 0 && key == 'q' )
        {
            //set coordintats PointVerticalGrid1 PointVerticalGrid2  // First click Point1, Second Point2
            mwReference->mainHandler(4);
            ALT_key = 0;
        }


        if (wParam == WM_LBUTTONDOWN )
        {
            //draw trajectory on the screen
            mwReference->drawTrajectory();
        }



        SHIFT_key = 0;
        CTRL_key = 0;
        ALT_key = 0;

        //}

    }
    return CallNextHookEx(NULL,    nCode,wParam,lParam);

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Cursor ruler");

    // Setup variables
    mwReference = this;
    bWinKey     = false;

    // Install the low-level keyboard & mouse hooks
    hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardMouseProc, 0, 0);
    hhkLowLevelKybd = SetWindowsHookEx(WH_MOUSE_LL, LowLevelKeyboardMouseProc, 0, 0);

    //get screen resolution and mouse position
    screen0 = QApplication::screens().at(0);

    //tray menu
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    //tray to minimize program
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/icons/ruler.ico"));
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();

    //custom stylesheet, not ready
    QFile theme;

    theme.setFileName("styleSheetCustom.qss");
    if(theme.exists()){

        theme.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(theme.readAll());
        ui->centralwidget->setStyleSheet(styleSheet);
    }
    else{
        theme.setFileName(":/styles/styleSheetCustom.qss");
        theme.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(theme.readAll());
        ui->centralwidget->setStyleSheet(styleSheet);
    }

    //Open application with full screen and transparent background
    Qt::WindowFlags flags;
    flags |= Qt::FramelessWindowHint;
    flags |= Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowState(Qt::WindowFullScreen);

    animationTimer = new QTimer(this);
    animationTimer->setInterval(ANIM_TIMEOUT);

    connect(animationTimer, &QTimer::timeout, this, &MainWindow::drawTrajectory);

    //hide elements
    ui->label_15->hide();
    ui->label_14->hide();

    ui->spinBoxST_ProjectileMass->hide();
    ui->spinBoxST_ProjectileCaliber->hide();

    //tray action, quit from programm
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));


    // Регистрируем HotKey "ALT + E" OnTopMode
    RegisterHotKey((HWND)MainWindow::winId(),               // Устанавливаем системный идентификатор окна виджета, который будет обрабатывать HotKey
                   101,                                     // Устанавливаем идентификатор HotKey
                   MOD_ALT,                                 // Устанавливаем модификаторы
                   'E');

    // Регистрируем HotKey "ALT + R" Hide settings
    RegisterHotKey((HWND)MainWindow::winId(),               // Устанавливаем системный идентификатор окна виджета, который будет обрабатывать HotKey
                   102,                                     // Устанавливаем идентификатор HotKey
                   MOD_ALT,                                 // Устанавливаем модификаторы
                   'R');


}

MainWindow::~MainWindow()
{
    // Be friendly! Remove hooks!
    UnhookWindowsHookEx(hhkLowLevelKybd);

    delete ui;
}

void MainWindow::mainHandler(int id)
{
    switch (id) {

    //delete poins and clear screen
    case 0:

        Point1 = QPoint(0,0);
        Point2 = QPoint(0,0);
        PointProjectileDraw2 = QPoint(0,0);

        ui->labelCL_Point1->setText("(" + QString::number(Point1.x) + ";" + QString::number(Point1.y) + ")");
        ui->labelCL_Point2->setText("(" + QString::number(Point2.x) + ";" + QString::number(Point2.y) + ")");

        calculateDistance();
        calculateProjectile();

        repaint();
        break;

        //delete poins and clear screen
    case 1:

        PointScale1 = QPoint(0,0);
        PointScale2 = QPoint(0,0);
        repaint();
        break;

        //Set Point1 Point2 and display
    case 2:

        //clear point and reset all if was double click on last point
        if(Point1.x ==  QCursor::pos(screen0).x() && Point1.y ==  QCursor::pos(screen0).y()){
            Point1 = QPoint(0,0);
            Point2 = QPoint(0,0);
            repaint();

            ui->labelCL_Point1->setText("(" + QString::number(Point1.x) + ";" + QString::number(Point1.y) + ")");
            ui->labelCL_Point2->setText("(" + QString::number(Point2.x) + ";" + QString::number(Point2.y) + ")");

            ProjectileSpeed = ui->spinBoxST_StartSpeed->value();
            calculateDistance();
            calculateProjectile();
            break;
        }


        Point1 = QCursor::pos(screen0);
        ui->labelCL_Point1->setText("(" + QString::number(Point1.x) + ";" + QString::number(Point1.y) + ")");

        if(Point2.x == 0 && Point2.y == 0)
            Point2 = QCursor::pos(screen0);
        Point1 = QCursor::pos(screen0);
        ui->labelCL_Point1->setText("(" + QString::number(Point1.x) + ";" + QString::number(Point1.y) + ")");

        if(Point2.x == 0 && Point2.y == 0)
            Point2 = QCursor::pos(screen0);

        ui->labelCL_Point2->setText("(" + QString::number(Point2.x) + ";" + QString::number(Point2.y) + ")");


        ProjectileSpeed = ui->spinBoxST_StartSpeed->value();
        calculateDistance();
        calculateProjectile();

        repaint();

        break;

        //Set PointScale1 PointScale2 and display
    case 3:

        //clear point and reset all if was double click on last point
        if(PointScale1.x ==  QCursor::pos(screen0).x() && PointScale1.y ==  QCursor::pos(screen0).y()){
            PointScale1 = QPoint(0,0);
            PointScale2 = QPoint(0,0);

            repaint();
            calculateScale();

            break;

        }

        PointScale1 = QCursor::pos(screen0);

        if(PointScale2.x == 0 && PointScale2.y == 0)
            PointScale2 = QCursor::pos(screen0);

        calculateScale();

        repaint();

        break;

        //Set PointScale1 PointScale2 and display
    case 4:

        PointVerticalGrid1 = QCursor::pos(screen0);

        if(ui->checkBoxST_VerticalShoot->isChecked())
            PointVerticalGrid2 = QPoint(PointVerticalGrid1.x - 200 * cos(GunDepressionAngleVertDeg), PointVerticalGrid1.y - 200 * sin(GunDepressionAngleVertDeg));
        else
            PointVerticalGrid2 = QPoint(PointVerticalGrid1.x - 200 * cos(GunDepressionAngle), PointVerticalGrid1.y - 200 * sin(GunDepressionAngle));

        repaint();

        break;


    default:
        break;
    }
}

void MainWindow::calculateScale()
{
    //get ammout of pixels from right click
    Point RightClick;

    RightClick.x = abs(PointScale2.x - PointScale1.x);
    RightClick.y = abs(PointScale2.y - PointScale1.y);

    //because grid has squere shape, get rid of some angle if user dont draw staight line
    if(RightClick.x <= RightClick.y){
        scale = ui->doubleSpinBoxST_MapScale->value() / RightClick.y;
    }
    else{
        scale = ui->doubleSpinBoxST_MapScale->value() / RightClick.x;
    }

    ui->labelCL_Scale->setText(QString::number(scale));

}

void MainWindow::calculateDistance()
{

    static const double TWOPI = 6.2831853071795865;
    static const double RAD2DEG = 57.2957795130823209;


    Point LeftClick = Point2 - Point1;

    double PixelDist = sqrt(LeftClick.x * LeftClick.x + LeftClick.y * LeftClick.y);
    double MeterDistance = PixelDist * scale;
    DistanceBetwenPoints = MeterDistance;


    AzimutRad = atan2(Point1.x - Point2.x, Point2.y - Point1.y);
    if (AzimutRad < 0.0)
        AzimutRad += TWOPI;
    AzimutDeg = RAD2DEG * AzimutRad;


    ui->labelCL_Distance->setText(QString::number(MeterDistance,  'f', 2) + " m");
    ui->labelCL_Azimut->setText(QString::number(AzimutDeg,  'f', 2) + " °");

}

void MainWindow::calculateProjectile()
{

    static const double g = 9.81;
    static const double RAD2DEG = 57.2957795130823209;
    const double pi = 3.14159;

    const double Diameter = ui->spinBoxST_ProjectileCaliber->value()/1000.0;
    const double dt = 0.1;
    const double Projectile_mass = ui->spinBoxST_ProjectileMass->value();

    const double k = 0.01; // air resistance coefficient
    const double rho = 1.2; // air density
    const double Cd = 0.47; // drag coefficient
    const double A = pi * (Diameter/2) * (Diameter/2); // cross-sectional area of projectile

    if(ui->checkBoxST_ComplexCalculations->isChecked()){

        GunDepressionAngle = asin((DistanceBetwenPoints * g) / (ProjectileSpeed * ProjectileSpeed))/2;

        double x = 0; // horizontal distance
        double y = 0; // vertical distance
        double t = 0; // time

        double v_x = ProjectileSpeed * cos(GunDepressionAngle);
        double v_y = ProjectileSpeed * sin(GunDepressionAngle);

        double v = sqrt(v_x * v_x + v_y * v_y);
        double Fx = -0.5 * rho * v * v * Cd * A * v_x / v;
        double Fy = -0.5 * rho * v * v * Cd * A * v_y / v;

        double ax = Fx / Projectile_mass;
        double ay = -g + Fy / Projectile_mass;

        while (x <= DistanceBetwenPoints && y >= 0 ) {

            v = sqrt(v_x * v_x + v_y * v_y);
            Fx = -0.5 * rho * v * v * Cd * A * v_x / v;
            Fy = -0.5 * rho * v * v * Cd * A * v_y / v;
            ax = Fx / Projectile_mass;
            ay = -g + Fy / Projectile_mass;
            x += v_x * dt;
            y += v_y * dt;
            v_x += ax * dt;
            v_y += ay * dt;
            t += dt;

            if(t > 300)
                break;

        }

        ui->labelCL_Time->setText(QString::number(t, 'f', 2) + " s");
        ui->labelCL_Angle->setText(QString::number(sin(GunDepressionAngle) * RAD2DEG, 'f', 2) + " °");

    }
    else{

        if(ui->checkBoxST_VerticalShoot->isChecked()){

            GunDepressionAngle = asin((DistanceBetwenPoints * g) / (ProjectileSpeed * ProjectileSpeed))/2;

            ProjectileTimeOfFly = (2 * ProjectileSpeed * cos(GunDepressionAngle))/g;

            GunDepressionAngleVertDeg = pi/2 - GunDepressionAngle;

            ui->labelCL_Time->setText(QString::number(ProjectileTimeOfFly, 'f', 2) + " s");
            ui->labelCL_Angle->setText(QString::number((90 - (sin(GunDepressionAngle) * RAD2DEG)), 'f', 2) + " °");

        }
        else{

            GunDepressionAngle = asin((DistanceBetwenPoints * g) / (ProjectileSpeed * ProjectileSpeed))/2;

            ProjectileTimeOfFly = (2 * ProjectileSpeed * sin(GunDepressionAngle))/g;

            ui->labelCL_Time->setText(QString::number(ProjectileTimeOfFly, 'f', 2) + " s");
            ui->labelCL_Angle->setText(QString::number(sin(GunDepressionAngle) * RAD2DEG, 'f', 2) + " °");
        }

    }

}

void MainWindow::drawTrajectory()
{

    static double t;
    double dt = ANIM_TIMEOUT/1000.0;            //Refresh rate, good acuracy of mooving point when ANIM_TIMEOUT = 1000, bad when ANIM_TIMEOUT = 1
    const double N = ProjectileTimeOfFly / dt; // Total number of steps

    Point CalcPoint = Point2 - Point1;

    double PixelDist = sqrt(CalcPoint.x * CalcPoint.x + CalcPoint.y * CalcPoint.y);
    double step = PixelDist / N;

    static double distance = 0;

    if(t <= 0){

        animationTimer->start();
        distance = 0;
        PointProjectileDraw2 = Point2; // position of shell x

    }

    //Calculate the new x and y coordinates based on the current position and the angle
    double newX = PointProjectileDraw2.x + step * cos(AzimutRad - 1.57079633);
    double newY = PointProjectileDraw2.y + step * sin(AzimutRad - 1.57079633);

    //Update the position of the circle
    PointProjectileDraw2 = QPoint(newX, newY);

    distance += step;
    t += dt;
    ui->labelCL_Time->setText(QString::number(ProjectileTimeOfFly - t, 'f', 2) + " s");

    if(t > ProjectileTimeOfFly){
        animationTimer->stop();
        t = 0;
        PointProjectileDraw2 = QPoint(0,0);
        distance = 0;
        ui->labelCL_Time->setText(QString::number(ProjectileTimeOfFly, 'f', 2) + " s");
    }

    repaint();



}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //hide windows to tray
    if(this->isVisible()){
        this->hide();
        event->ignore();
    }
}


void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    QPainterPath pathP, pathPS, pathPV;

    //pathP.addText(Point1.x,Point1.y, QFont("Times", 14), QString::number(DistanceBetwenPoints) + "\n" + QString::number(ProjectileTimeOfFly));

    pathP.moveTo(Point2.x, Point2.y);
    pathP.lineTo(Point1.x, Point1.y);

    pathPS.moveTo(PointScale2.x, PointScale2.y);
    pathPS.lineTo(PointScale1.x, PointScale1.y);

    pathPV.moveTo(PointVerticalGrid1.x, PointVerticalGrid1.y);
    pathPV.lineTo(PointVerticalGrid1.x - 200, PointVerticalGrid1.y);
    pathPV.moveTo(PointVerticalGrid1.x, PointVerticalGrid1.y);
    pathPV.lineTo(PointVerticalGrid1.x, PointVerticalGrid1.y - 200);
    pathPV.moveTo(PointVerticalGrid1.x, PointVerticalGrid1.y);
    pathPV.lineTo(PointVerticalGrid2.x, PointVerticalGrid2.y);

    //QPen pen(Qt::red, 2, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
    QPen pen(Qt::red, 2, Qt::DashDotLine, Qt::RoundCap);
    painter.setPen(pen);
    painter.drawPath(pathP);

    pen.setColor(Qt::green);
    painter.setPen(pen);
    painter.drawPath(pathPS);

    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    painter.drawEllipse(PointProjectileDraw2.x, PointProjectileDraw2.y, 5, 5);

    painter.drawPath(pathPV);
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType)
    Q_UNUSED(result)
    // Преобразуем указатель message в MSG WinAPI
    MSG* msg = reinterpret_cast<MSG*>(message);

    // Если сообщение является HotKey, то ...
    if(msg->message == WM_HOTKEY){

        if(msg->wParam == 101){
            // OnTop regime
            if(onTop == false){
                setWindowFlag(Qt::WindowStaysOnTopHint, true);
                setFocus();
                show();
                onTop = true;
            }
            else{
                setWindowFlag(Qt::WindowStaysOnTopHint, false);
                onTop = false;
            }
            return true;
        }

        if(msg->wParam == 102){

            // Hide settings
            if(ui->frameSettings->isHidden()){
                setWindowFlag(Qt::WindowTransparentForInput,false);
                this->show();
                ui->frameSettings->show();

            }
            else{

                //Open application with full screen and transparent background

                setWindowFlag(Qt::WindowTransparentForInput, true);
                this->show();
                ui->frameSettings->hide();
            }
            return true;
        }


    }
    if (msg->message == 0x0312) {    // Trap WM_HOTKEY
        int id = msg->wParam;
        qDebug() << id;
    }
    return false;
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason){
    case QSystemTrayIcon::Trigger:
        !isVisible() ? show() : hide();
        break;
    default:
        break;
    }
}



void MainWindow::on_checkBoxST_ComplexCalculations_clicked()
{
    if(ui->checkBoxST_ComplexCalculations->isChecked()){
        ui->label_15->show();
        ui->label_14->show();

        ui->spinBoxST_ProjectileMass->show();
        ui->spinBoxST_ProjectileCaliber->show();
    }
    else{
        ui->label_15->hide();
        ui->label_14->hide();

        ui->spinBoxST_ProjectileMass->hide();
        ui->spinBoxST_ProjectileCaliber->hide();
    }
}

