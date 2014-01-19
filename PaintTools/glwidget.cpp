/**
Edited by: Charuvit Wannissorn V00737846
Note: With the edited and added functions: the program can draw shapes and stuff
**/

#include <QtGui>
#include <QtOpenGL>

#include <math.h>
#include <stdio.h>
#include "glwidget.h"

#include "shapes.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent) {
}

GLWidget::~GLWidget() {
}

QSize GLWidget::minimumSizeHint() const {
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const {
    return QSize(400, 100);
}

static void qNormalizeAngle(int &angle) {
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void GLWidget::initializeGL() {
    //qglClearColor(qtPurple.dark());
    glClearColor(1.0,1.0,1.0,1.0);

    paintColor = RGBColor(0.0f,0.0f,0.0f);

    currentShape = NULL;
    drawmode = 1;
    this->setCursor(Qt::CrossCursor);
    fill = 0;
    AA = 0;
    BrushSize = 2;
    hitVertex = false;
    drawingPolygon = false;
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    for(int i = 0; i < shapeList.count(); i++){
        //fprintf(stderr, "drawing shape %d\n", i);
        shapeList[i]->draw();
    }

    if(currentShape != NULL){
        currentShape->draw();
    }

    swapBuffers();
}

void GLWidget::resizeGL(int width, int height) {
    //int side = qMin(width, height);
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();


    glOrtho(0,width,height,0,0,1);

    glMatrixMode(GL_MODELVIEW);
}


void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint mPos = event->pos();

        if (drawmode < 5) {
            currentShape = new Scribble(this, mPos, paintColor, drawmode, BrushSize); //brushType = drawmode
            setPixel(mPos.x(), mPos.y(), paintColor, BrushSize, drawmode); //the point when User clicks on the Widget
            swapBuffers();
        }
        else if (drawmode == 5) currentShape = new Line(this,mPos, paintColor, AA, BrushSize);
        else if (drawmode == 6) currentShape = new Rectangle2(this, mPos, paintColor, fill, BrushSize);
        else if (drawmode == 7) currentShape = new Circle(this, mPos, paintColor, fill, BrushSize);
        else if (drawmode == 8) {
            if (drawingPolygon) {
                currentShape->extend(mPos);
                currentShape->drawPart();
                swapBuffers();

                if (abs(mPos.x() - lastPos.x()) < 10 && abs(mPos.y() - lastPos.y()) < 10) {//if the next point is close to the first, finish the shape
                    drawingPolygon = false;
                    shapeList.append(currentShape);
                    fprintf(stderr, "completed a polygon!\n");
                    paintGL(); //the shape will be complete with draw()
                }
            } else {
                currentShape = new Polygon2(this, mPos, paintColor, fill, BrushSize);
                drawingPolygon = true;
                lastPos = mPos;
                swapBuffers();
            }
        }
        else if (drawmode == 9) {
            this->setCursor(Qt::ClosedHandCursor);
            for (int i = 0; i < shapeList.count(); i++) { // for each shape
                for (int j = 0; j < shapeList[i]->Vertices.count(); j++) { // for each vertex
                    if (abs(shapeList[i]->Vertices[j].x() - mPos.x()) < 10 && abs(shapeList[i]->Vertices[j].y() - mPos.y()) < 10) {
                        hitVertex = true;
                        x = i; //shape index
                        y = j; //vertex index
                        return;
                    }
                }
            }
        }
    }
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    QPoint mPos = event->pos();

    if (drawmode < 8) { //if drawmode 8 = polygon = draw nothing yet
        currentShape->extend(mPos);
        currentShape->drawPart();
        paintGL();
    }
    else if (drawmode == 9 && hitVertex) {
        shapeList[x]->Vertices[y].setX(mPos.x()); //change the location of the vertex
        shapeList[x]->Vertices[y].setY(mPos.y());
        shapeList[x]->draw();
        paintGL();
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    hitVertex = false;
    if (drawmode == 9)
        this->setCursor(Qt::OpenHandCursor);

    if (currentShape != NULL) {
        shapeList.append(currentShape);
        //fprintf(stderr, "completed shape!\n");
    }
}

void GLWidget::setColor(RGBColor c) {
    paintColor = c;
    paintGL();
}

void GLWidget::setSize(int size) {
    BrushSize = size;
}

RGBColor GLWidget::getPixel(int x, int y) {
    RGBColor c;
    y = this->height() - y;
    glReadPixels(x,y,1,1,GL_RGB,GL_FLOAT,&c);

    return c;
}

void GLWidget::setPixel(int x, int y, RGBColor c, double size, int type) {
    glColor3f(c.r,c.g,c.b);

    if (type == 1) //type 1: normal brush
    {
        glPointSize(size);
        glBegin(GL_POINTS);
            glVertex2i(x,y);
        glEnd();
    }
    else if (type == 2) //type 2: Bloody Brush
    {
        glPointSize(size/2);
        glBegin(GL_POINTS);
            for(int i = 1; i < 3; i++) {
                glVertex2i(x,y);
                glVertex2i(x+size/i,y-size/i);
                glVertex2i(x-size/5,y-size/i);
            }
        glEnd();

        glPointSize(2+size/10);
        glBegin(GL_POINTS);
            for(int i = 1; i < 5; i++) {
                glVertex2i(x+size/i,y-size/2);
                glVertex2i(x-size/5,y+size);
            }
        glEnd();
    }
    else if (type == 3) //type 3: Pen Brush
    {
        glColor3f(c.r,c.g,c.b);
        glPointSize(2 + size/20);
        glBegin(GL_POINTS);
            glVertex2i(x,y);
        for (int i = 1; i < 5 + size/10; i++) {
            glVertex2i(x+i, y-i);
            glVertex2i(x-i,y+i);
        }
        glEnd();
    }
    else if (type == 4) //type 4: Paint Brush
    {
        glColor3f(c.r,c.g,c.b);
        glPointSize(2 + size/30);
        glBegin(GL_POINTS);
        for (int i = 1; i < 5 + size/10; i++) {
            glVertex2i(x-size/10+3, y-i*5);
            glVertex2i(x+size/8+2,y+i*5);
            glVertex2i(x+size/8-5, y-i*4);
            glVertex2i(x-size/10+5, y+i*3);
            glVertex2i(x,y-i*2);
        }
        glEnd();
    }
}

void GLWidget::setBrush1() {
    drawmode = 1;
    this->setCursor(Qt::CrossCursor);
}

void GLWidget::setBrush2() {
    drawmode = 2;
    this->setCursor(Qt::CrossCursor);
}

void GLWidget::setBrush3() {
    drawmode = 3;
    this->setCursor(Qt::CrossCursor);
}

void GLWidget::setBrush4() {
    drawmode = 4;
    this->setCursor(Qt::CrossCursor);
}

void GLWidget::setBresenhamLine() {
    drawmode = 5;
    AA = 0;
    this->setCursor(Qt::CrossCursor);
}

void GLWidget::setWuLine() {
    drawmode = 5;
    AA = 1;
    this->setCursor(Qt::CrossCursor);
}

void GLWidget::setRectangle() {
    drawmode = 6;
    fill = 0;
    this->setCursor(Qt::CrossCursor);
}

void GLWidget::setFillRectangle() {
    drawmode = 6;
    fill = 1;
    this->setCursor(Qt::CrossCursor);
}

void GLWidget::setCircle() {
    drawmode = 7;
    fill = 0;
    this->setCursor(Qt::CrossCursor);
}

void GLWidget::setFillCircle() {
    drawmode = 7;
    fill = 1;
    this->setCursor(Qt::CrossCursor);
}

void GLWidget::setPolygon() {
    drawmode = 8;
    fill = 0;
    this->setCursor(Qt::CrossCursor);
}

void GLWidget::setFillPolygon() {
    drawmode = 8;
    fill = 1;
    this->setCursor(Qt::CrossCursor);
}

void GLWidget::setEditVertex() {
    drawmode = 9;
    this->setCursor(Qt::OpenHandCursor);
}

void GLWidget::clear() {
    currentShape = NULL;
    shapeList.clear();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    swapBuffers();
}
