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

        //qDebug() << wParam;
        //qDebug() << key;

        //if (key >= 'A' && key <= 'Z')
        //{

        if  (GetAsyncKeyState(VK_SHIFT)>= 0) key +=32;

        /*********************************************
             ***   Hotkey scope                         ***
             ***   do stuff here                        ***
             **********************************************/

        if (CTRL_key !=0 && key == 'x' )
        {
            //clear P1 P2 line
            mwReference->mainHandler(0);
            CTRL_key=0;

        }
        if (CTRL_key !=0 && key == 'z' )
        {
            //clear PS1 PS2 line
            mwReference->mainHandler(1);
            CTRL_key=0;

        }

        if (CTRL_key !=0 && key == 'z' )
        {
            //clear PS1 PS2 line
            mwReference->mainHandler(1);
            CTRL_key=0;

        }

        if (ALT_key != 0 && wParam == WM_RBUTTONDOWN )
        {
            //set coordintats P1 P2  // First click P1, Second P2
            mwReference->mainHandler(2);
            ALT_key = 0;
        }

        if (CTRL_key != 0 && wParam == WM_RBUTTONDOWN )
        {
            //set coordintats PS1 PS2  // First click P1, Second P2
            mwReference->mainHandler(3);
            CTRL_key = 0;
        }


        if (ALT_key != 0 && key == 'f' )
        {
            //set coordintats PV1 PV2  // First click P1, Second P2
            mwReference->mainHandler(4);
            ALT_key = 0;
        }


        if (ALT_key != 0 && wParam == WM_LBUTTONDOWN )
        {
            //draw trajectory on the screen
            mwReference->drawTrajectory();
            ALT_key = 0;
        }



        SHIFT_key = 0;
        CTRL_key = 0;
        ALT_key = 0;

        //}

    }
    return CallNextHookEx(NULL,    nCode,wParam,lParam);


    /*
    if (nCode == HC_ACTION)
    {
        switch (wParam)
        {
            // Pass KeyDown/KeyUp messages for Qt class to logicize
            case WM_KEYDOWN:
                mwReference->keyDown(PKBDLLHOOKSTRUCT(lParam)->vkCode);
            break;
            case WM_KEYUP:
                mwReference->keyUp(PKBDLLHOOKSTRUCT(lParam)->vkCode);
            break;

            case WM_LBUTTONDOWN:
                qDebug() << "Left click"; // Left click
            break;


        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);

    */
}

//Mouse hook

/*
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    switch( wParam )
    {
    case WM_LBUTTONDOWN:  qDebug() << "Left click"; // Left click
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
*/
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
    animationTimer->setInterval(100);

    connect(animationTimer, &QTimer::timeout, this, &MainWindow::drawTrajectory);

    //hide elements
    ui->label_15->hide();
    ui->label_14->hide();

    ui->spinBoxST_ProjectileMass->hide();
    ui->spinBoxST_ProjectileCaliber->hide();

    //tray action, quit from programm
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));


    // Регистрируем HotKey "SHIFT + E" OnTopMode
    RegisterHotKey((HWND)MainWindow::winId(),           // Устанавливаем системный идентификатор окна виджета, который будет обрабатывать HotKey
                   101,                                 // Устанавливаем идентификатор HotKey
                   MOD_ALT,                           // Устанавливаем модификаторы
                   'E');

    // Регистрируем HotKey "SHIFT + R" Hide settings
    RegisterHotKey((HWND)MainWindow::winId(),           // Устанавливаем системный идентификатор окна виджета, который будет обрабатывать HotKey
                   102,                                 // Устанавливаем идентификатор HotKey
                   MOD_ALT,                           // Устанавливаем модификаторы
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

        p1 = QPoint(0,0);
        p2 = QPoint(0,0);
        repaint();
        qDebug() << "Cleared";

        break;

        //delete poins and clear screen
    case 1:

        ps1 = QPoint(0,0);
        ps2 = QPoint(0,0);
        repaint();
        qDebug() << "Cleared";

        break;

        //Set P1 P2 and display
    case 2:

        p1 = QCursor::pos(screen0);
        ui->labelCL_P1->setText("(" + QString::number(p1.x) + ";" + QString::number(p1.y) + ")");

        if(p2.x == 0 && p2.y == 0)
            p2 = QCursor::pos(screen0);


        ui->labelCL_P2->setText("(" + QString::number(p2.x) + ";" + QString::number(p2.y) + ")");

        Vspeed = ui->spinBoxST_StartSpeed->value();
        calculateDistance();
        calculateProjectile();

        repaint();
        qDebug() << "Point set";

        break;

        //Set PS1 PS2 and display
    case 3:

        ps1 = QCursor::pos(screen0);
        ui->labelCL_P1->setText("(" + QString::number(p1.x) + ";" + QString::number(p1.y) + ")");

        if(ps2.x == 0 && ps2.y == 0)
            ps2 = QCursor::pos(screen0);

        calculateScale();

        repaint();
        qDebug() << "Point set";

        break;

        //Set PS1 PS2 and display
    case 4:

        pv1 = QCursor::pos(screen0);

        if(ui->checkBoxST_VerticalShoot->isChecked())
            pv2 = QPoint(pv1.x - 200 * cos(OangleVertDeg), pv1.y - 200 * sin(OangleVertDeg));
        else
            pv2 = QPoint(pv1.x - 200 * cos(Oangle), pv1.y - 200 * sin(Oangle));

        repaint();
        qDebug() << "Point set";

        break;


    default:
        break;
    }
}

void MainWindow::calculateScale()
{
    //get ammout of pixels from right click
    Point RightClick;

    RightClick.x = abs(ps2.x - ps1.x);
    RightClick.y = abs(ps2.y - ps1.y);

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

    Point LeftClick = p2 - p1;

    double PixelDist = sqrt(LeftClick.x * LeftClick.x + LeftClick.y * LeftClick.y);
    double MeterDistance = PixelDist * scale;
    Ddistance = MeterDistance;


    double theta = atan2(p1.x - p2.x, p2.y - p1.y);
    if (theta < 0.0)
        theta += TWOPI;
    double azimut = RAD2DEG * theta;


    ui->labelCL_Distance->setText(QString::number(MeterDistance,  'f', 2) + " m");
    ui->labelCL_Azimut->setText(QString::number(azimut,  'f', 2) + " °");

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

        Oangle = asin((Ddistance * g) / (Vspeed * Vspeed))/2;

        double x = 0; // horizontal distance
        double y = 0; // vertical distance
        double t = 0; // time

        double v_x = Vspeed * cos(Oangle);
        double v_y = Vspeed * sin(Oangle);

        double v = sqrt(v_x * v_x + v_y * v_y);
        double Fx = -0.5 * rho * v * v * Cd * A * v_x / v;
        double Fy = -0.5 * rho * v * v * Cd * A * v_y / v;

        double ax = Fx / Projectile_mass;
        double ay = -g + Fy / Projectile_mass;

//        qDebug() << "v:" << v;
//        qDebug() << "ax:" << ax << " ay:" << ay << " t:" << t ;
//        qDebug() << "vx:" << v_x << " vy:" << v_y << " v:" << v ;
//        qDebug() << "Fx:" << Fx << " Fy:" << Fy << " t:" << t ;

      while (x <= Ddistance && y >= 0 ) {

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

        qDebug() << "ax:" << ax << " ay:" << ay << " t:" << t ;
        qDebug() << "vx:" << v_x << " vy:" << v_y << " v:" << v ;
        qDebug() << "x:" << x << " y:" << y << " t:" << t ;

        ui->labelCL_Time->setText(QString::number(t, 'f', 2) + " s");
        ui->labelCL_Angle->setText(QString::number(sin(Oangle) * RAD2DEG, 'f', 2) + " °");

    }
    else{

        if(ui->checkBoxST_VerticalShoot->isChecked()){

            Oangle = asin((Ddistance * g) / (Vspeed * Vspeed))/2;

            Tfly = (2 * Vspeed * cos(Oangle))/g;

            OangleVertDeg = pi/2 - Oangle;

            ui->labelCL_Time->setText(QString::number(Tfly, 'f', 2) + " s");
            ui->labelCL_Angle->setText(QString::number((90 - (sin(Oangle) * RAD2DEG)), 'f', 2) + " °");

        }
        else{

            Oangle = asin((Ddistance * g) / (Vspeed * Vspeed))/2;

            Tfly = (2 * Vspeed * sin(Oangle))/g;

            ui->labelCL_Time->setText(QString::number(Tfly, 'f', 2) + " s");
            ui->labelCL_Angle->setText(QString::number(sin(Oangle) * RAD2DEG, 'f', 2) + " °");
        }

    }


    qDebug() << Oangle;
    qDebug() << Tfly;
}

void MainWindow::drawTrajectory()
{

    static double t;
    double dt = 0.1;
    const double N = Tfly / dt; // Total number of steps
    const double dx = static_cast<double>(p1.x - p2.x) / N;
    const double dy = static_cast<double>(p1.y - p2.y) / N;

    qDebug() << "dx" << dx << " dy" << dy;

    if(t <= 0){

         qDebug() << "Timer started";
        animationTimer->start();

        pd2 = p2; // position of shell x
        pd1 = p1;


    }

    pd2.x += static_cast<int>(dx);
    pd2.y += static_cast<int>(dy);

    //pd2 = QPoint(pd2.x + dx * cos(Oangle), pd2.y + dy * cos(Oangle));

    t += dt;

    repaint();

    if(t > Tfly){
        animationTimer->stop();
        t = 0;
        pd2 = QPoint(0,0);
    }

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

    //pathP.addText(p1.x,p1.y, QFont("Times", 14), QString::number(Ddistance) + "\n" + QString::number(Tfly));

    pathP.moveTo(p2.x, p2.y);
    pathP.lineTo(p1.x, p1.y);

    pathPS.moveTo(ps2.x, ps2.y);
    pathPS.lineTo(ps1.x, ps1.y);

    pathPV.moveTo(pv1.x, pv1.y);
    pathPV.lineTo(pv1.x - 200, pv1.y);
    pathPV.moveTo(pv1.x, pv1.y);
    pathPV.lineTo(pv1.x, pv1.y - 200);
    pathPV.moveTo(pv1.x, pv1.y);
    pathPV.lineTo(pv2.x, pv2.y);

    //QPen pen(Qt::red, 2, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
    QPen pen(Qt::red, 2, Qt::DashDotLine, Qt::RoundCap);
    painter.setPen(pen);
    painter.drawPath(pathP);

    pen.setColor(Qt::green);
    painter.setPen(pen);
    painter.drawPath(pathPS);

    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    painter.drawEllipse(pd2.x, pd2.y, 5, 5);

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
                ui->frameSettings->show();
            }
            else{
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

