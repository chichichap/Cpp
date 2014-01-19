#include <QtGui>
#include <QPalette>
#include <QRgb>
#include <QMenu>
#include <QMenuBar>
#include "glwidget.h"
#include "window.h"

/*  This file contains the implementation of our Window class
    It holds a description of our window and code describing
    the behaviour of the windows "widgets"
*/

Window::Window()
{
    setWindowTitle(tr("CSC 205 Assignment 1"));

    // define our drawing widget (see GLWidget.h / .cpp)
    glWidget = new GLWidget;

    menubar = new QMenuBar;
    QMenu *file = new QMenu("&File");
    QMenu *edit = new QMenu("&Edit");
    menubar->addMenu(file);
    menubar->addMenu(edit);

    QAction *quitAct = new QAction(tr("&Close"), this);
    file->addAction(quitAct);

    // define our color sliders
    hSlider = new QSlider(Qt::Horizontal);
    sSlider = new QSlider(Qt::Horizontal);
    vSlider = new QSlider(Qt::Horizontal);

    rSlider = new QSlider(Qt::Horizontal);
    gSlider = new QSlider(Qt::Horizontal);
    bSlider = new QSlider(Qt::Horizontal);

    // added slider
    bSizeSlider = new QSlider(Qt::Horizontal);

    // configure our color sliders
    hSlider->setRange(0,360);
    hSlider->setTickInterval(360/8);
    hSlider->setTickPosition(QSlider::TicksBelow);
    hSlider->setMinimumWidth(200);


    sSlider->setRange(0,100);
    sSlider->setTickInterval(100/8);
    sSlider->setTickPosition(QSlider::TicksBelow);

    vSlider->setRange(0,100);
    vSlider->setTickInterval(100/8);
    vSlider->setTickPosition(QSlider::TicksBelow);

    rSlider->setRange(0,255);
    rSlider->setTickInterval(255/8);
    rSlider->setTickPosition(QSlider::TicksBelow);

    gSlider->setRange(0,255);
    gSlider->setTickInterval(255/8);
    gSlider->setTickPosition(QSlider::TicksBelow);

    bSlider->setRange(0,255);
    bSlider->setTickInterval(255/8);
    bSlider->setTickPosition(QSlider::TicksBelow);

    bSizeSlider->setRange(2,80);
    bSizeSlider->setTickInterval(80/6);
    bSizeSlider->setTickPosition(QSlider::TicksBelow);

    // labels for our sliders
    hLabel = new QLabel("0");
    sLabel = new QLabel("0");
    vLabel = new QLabel("0");

    rLabel = new QLabel("0");
    gLabel = new QLabel("0");
    bLabel = new QLabel("0");

    bSizeLabel = new QLabel("2");


    // The main (biggest) layout for our screen is a horizontal layout
    // This means items added to this layout will be added horizontally
    QHBoxLayout *mainLayout = new QHBoxLayout;

    // add our drawing widget as the first (leftmost) widget in our horizontal layout
    mainLayout->addWidget(glWidget);

    // our sliders are going to be in a vertical column
    QVBoxLayout *sliderLayout = new QVBoxLayout;
    sliderLayout->setAlignment(Qt::AlignTop);
    sliderLayout->setSizeConstraint(QLayout::SizeConstraint());

    // each slider has 2 labels below it in a horizontal box
    QHBoxLayout *hLabels = new QHBoxLayout;
    // first is just a name that never changes
    hLabels->addWidget(new QLabel(QString("Hue")));
    // we keep a reference to the next one because it's the label we're going to use to show
    // the value of this slider, and we'll need to update it accordingly
    hLabels->addWidget(hLabel);


    QHBoxLayout *sLabels = new QHBoxLayout;
    sLabels->addWidget(new QLabel(QString("Saturation")));
    sLabels->addWidget(sLabel);

    QHBoxLayout *vLabels = new QHBoxLayout;
    vLabels->addWidget(new QLabel(QString("Value")));
    vLabels->addWidget(vLabel);

    QHBoxLayout *rLabels = new QHBoxLayout;
    rLabels->addWidget(new QLabel(QString("Red")));
    rLabels->addWidget(rLabel);

    QHBoxLayout *gLabels = new QHBoxLayout;
    gLabels->addWidget(new QLabel(QString("Green")));
    gLabels->addWidget(gLabel);

    QHBoxLayout *bLabels = new QHBoxLayout;
    bLabels->addWidget(new QLabel(QString("Blue")));
    bLabels->addWidget(bLabel);

    QHBoxLayout *bSizeLabels = new QHBoxLayout;
    bSizeLabels->addWidget(new QLabel(QString("Brush Size")));
    bSizeLabels->addWidget(bSizeLabel);


    // we're going to have a box that shows the currently selected color
    // this is that box
    colorFrame = new QFrame;
    colorFrame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    colorFrame->setLineWidth(2);
    colorFrame->setAutoFillBackground(true);
    colorFrame->setMinimumHeight(75);
    colorFrame->setMinimumWidth(75);

    // this is how we change the color of a QFrame
    QPalette pal = colorFrame->palette();
    pal.setColor(colorFrame->backgroundRole(), QColor(0,0,0));
    colorFrame->setPalette(pal);

    // add all of our sliders and our color box to our vertical slider layout
    sliderLayout->addWidget(hSlider);
    sliderLayout->addLayout(hLabels);

    sliderLayout->addWidget(sSlider);
    sliderLayout->addLayout(sLabels);

    sliderLayout->addWidget(vSlider);
    sliderLayout->addLayout(vLabels);

    sliderLayout->addWidget(colorFrame);

    sliderLayout->addWidget(rSlider);
    sliderLayout->addLayout(rLabels);

    sliderLayout->addWidget(gSlider);
    sliderLayout->addLayout(gLabels);

    sliderLayout->addWidget(bSlider);
    sliderLayout->addLayout(bLabels);

    sliderLayout->addSpacing(30);
    sliderLayout->addWidget(bSizeSlider);
    sliderLayout->addLayout(bSizeLabels);

    // add the slider layout to the main layout
    mainLayout->addLayout(sliderLayout);

    QGridLayout *buttonsLayout = new QGridLayout;
    QPushButton *rectButton[14];

    //declare buttons
    rectButton[0] = new QPushButton("Brush 1");
    rectButton[1] = new QPushButton("Brush 2");
    rectButton[2] = new QPushButton("Brush 3");
    rectButton[3] = new QPushButton("Brush 4");
    rectButton[4] = new QPushButton("Bresenham's Line");
    rectButton[5] = new QPushButton("Wu's Line");
    rectButton[6] = new QPushButton("Rectangle");
    rectButton[7] = new QPushButton("Filled Rectangle");
    rectButton[8] = new QPushButton("Circle");
    rectButton[9] = new QPushButton("Filled Circle");
    rectButton[10] = new QPushButton("Polygon");
    rectButton[11] = new QPushButton("Filled Polygon");
    rectButton[12] = new QPushButton("Edit a Vertex");
    rectButton[13] = new QPushButton("Clear");

    for (int i=0; i < 14; i++)
        buttonsLayout->addWidget(rectButton[i], i/2, i%2, Qt::AlignTop);

    sliderLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    /*  Connections between our sliders and their labels */
    connect(hSlider, SIGNAL(valueChanged(int)), hLabel, SLOT(setNum(int)));
    connect(bSizeSlider, SIGNAL(valueChanged(int)), bSizeLabel, SLOT(setNum(int)));

    // had to use a special method for saturation and value because sliders go from 0 - 100 and we want 0 - 1
    connect(sSlider, SIGNAL(valueChanged(int)), this, SLOT(updateSVal(int)));
    connect(vSlider, SIGNAL(valueChanged(int)), this, SLOT(updateVVal(int)));

    connect(rSlider, SIGNAL(valueChanged(int)), rLabel, SLOT(setNum(int)));
    connect(gSlider, SIGNAL(valueChanged(int)), gLabel, SLOT(setNum(int)));
    connect(bSlider, SIGNAL(valueChanged(int)), bLabel, SLOT(setNum(int)));

    connect(hSlider, SIGNAL(valueChanged(int)), this, SLOT(hsvChanged()));
    connect(sSlider, SIGNAL(valueChanged(int)), this, SLOT(hsvChanged()));
    connect(vSlider, SIGNAL(valueChanged(int)), this, SLOT(hsvChanged()));

    connect(rSlider, SIGNAL(valueChanged(int)), this, SLOT(rgbChanged()));
    connect(gSlider, SIGNAL(valueChanged(int)), this, SLOT(rgbChanged()));
    connect(bSlider, SIGNAL(valueChanged(int)), this, SLOT(rgbChanged()));

    connect(bSizeSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setSize(int)));

    connect(this, SIGNAL(colorChanged(RGBColor)), glWidget, SLOT(setColor(RGBColor)));

    connect(rectButton[0], SIGNAL(clicked()), glWidget, SLOT(setBrush1() ));
    connect(rectButton[1], SIGNAL(clicked()), glWidget, SLOT(setBrush2() ));
    connect(rectButton[2], SIGNAL(clicked()), glWidget, SLOT(setBrush3() ));
    connect(rectButton[3], SIGNAL(clicked()), glWidget, SLOT(setBrush4() ));
    connect(rectButton[4], SIGNAL(clicked()), glWidget, SLOT(setBresenhamLine() ));
    connect(rectButton[5], SIGNAL(clicked()), glWidget, SLOT(setWuLine() ));
    connect(rectButton[6], SIGNAL(clicked()), glWidget, SLOT(setRectangle() ));
    connect(rectButton[7], SIGNAL(clicked()), glWidget, SLOT(setFillRectangle() ));
    connect(rectButton[8], SIGNAL(clicked()), glWidget, SLOT(setCircle() ));
    connect(rectButton[9], SIGNAL(clicked()), glWidget, SLOT(setFillCircle() ));
    connect(rectButton[10], SIGNAL(clicked()), glWidget, SLOT(setPolygon() ));
    connect(rectButton[11], SIGNAL(clicked()), glWidget, SLOT(setFillPolygon() ));
    connect(rectButton[12], SIGNAL(clicked()), glWidget, SLOT(setEditVertex() ));
    connect(rectButton[13], SIGNAL(clicked()), glWidget, SLOT(clear() ));

    connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));
}


void Window::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(e);
}

void Window::setColorFrame(int r, int g, int b)
{
    QPalette pal = colorFrame->palette();
    pal.setColor(colorFrame->backgroundRole(), QColor(r,g,b));
    colorFrame->setPalette(pal);

}

/*   SIGNALS   */
void Window::updateSVal(int value)
{
    sLabel->setNum((double)value/100.0);
}

void Window::updateVVal(int value)
{
    vLabel->setNum((double)value/100.0);
}

void Window::hsvChanged()
{
    // Calculate a new RGB Color from our HSV sliders
    RGBColor newRGB = HSVtoRGB(HSVColor((GLfloat)hSlider->value(), (GLfloat)sSlider->value()/100.0f , (GLfloat)vSlider->value()/100.0f ));

    // convert GLfloats to ints for Qt
    int r = (int)floor((double)newRGB.r * 255.0);
    int g = (int)floor((double)newRGB.g * 255.0);
    int b = (int)floor((double)newRGB.b * 255.0);

    rSlider->blockSignals(true);
    gSlider->blockSignals(true);
    bSlider->blockSignals(true);
    // update RGB slider positions
    rSlider->setValue(r);
    gSlider->setValue(g);
    bSlider->setValue(b);

    rSlider->blockSignals(false);
    gSlider->blockSignals(false);
    bSlider->blockSignals(false);

    // update RGB label values
    rLabel->setNum(r);
    gLabel->setNum(g);
    bLabel->setNum(b);

    // update Color Frame
    setColorFrame(r,g,b);

    //signal the glwidget with the new color
    emit colorChanged(newRGB);
}

void Window::rgbChanged()
{
    HSVColor hsv = RGBtoHSV(RGBColor((GLfloat)rSlider->value()/255.0f, (GLfloat)gSlider->value()/255.0f, (GLfloat)bSlider->value()/255.0f));

    int h = (int)floor((double)hsv.h);


    hSlider->blockSignals(true);
    sSlider->blockSignals(true);
    vSlider->blockSignals(true);

    // update RGB slider positions
    hSlider->setValue(h);
    sSlider->setValue(hsv.s*100);
    vSlider->setValue(hsv.v*100);

    hSlider->blockSignals(false);
    sSlider->blockSignals(false);
    vSlider->blockSignals(false);

    // update RGB label values
    hLabel->setNum(h);
    sLabel->setNum((double)hsv.s);
    vLabel->setNum((double)hsv.v);

    // update color frame
    setColorFrame(rSlider->value(), gSlider->value(), bSlider->value());

    // signal the glwidget
    emit colorChanged(RGBColor((GLfloat)rSlider->value()/255.0f, (GLfloat)gSlider->value()/255.0f, (GLfloat)bSlider->value()/255.0f));

}
