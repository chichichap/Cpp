#ifndef SHAPES_H
#define SHAPES_H

#include "Colors.h"
#include <QPoint>
#include "glwidget.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//class GLWidget;

class Shape {
    public:
        virtual void extend(QPoint pos) {p2 = pos;} //assign a new point
        virtual void draw() {}
        virtual void drawPart() {}

        QVector<QPoint> Vertices; //used for lines

    protected:
        virtual void FastLine(int x1, int y1, int x2, int y2);
        virtual void PolygonScanline();
        virtual void init(GLWidget *w, QPoint p, RGBColor c, int t, int s){
            glWidget = w;
            p1 = p;
            p2 = p;
            color = c;
            size = s;

            BrushType = t;
            fill = t;
            AA = t;
        }

        int BrushType;
        int size;
        int fill;
        int AA;
        GLWidget *glWidget;
        QPoint p1, p2;
        RGBColor color;
        QVector<QPoint> pointsDrawn; //used for scribbling
};

class Scribble: public Shape {
    public: //2
        Scribble (GLWidget *w, QPoint p, RGBColor c, int t, int s) {init(w,p,c,t,s);} //Scribble Constructor
        virtual void draw();     //draw everthing
        virtual void drawPart(); //drawing the next part
};

class Line: public Shape {
    public: //3
        Line (GLWidget *w, QPoint a, RGBColor c, int t, int s) {init(w,a,c,t,s);} //Line Constructor
        virtual void draw();
        virtual void drawPart();
        virtual void BresenhamLine(int x0, int y0, int x1, int y1);
        virtual void WuLine(int x1, int y1, int x2, int y2);
        virtual void WuPlot(int x, int y, double c);
};

class Rectangle2: public Shape {
    public:
        Rectangle2 (GLWidget *w, QPoint p, RGBColor c, int t, int s) { //Rectangle Constructor
            init(w,p,c,t,s);
            BrushType = 1;
        }
        virtual void draw();
        virtual void drawPart();
};

class Circle: public Shape {
    public:
        Circle (GLWidget *w, QPoint p, RGBColor c, int t, int s) { //Circle Constructor
            init(w,p,c,t,s);
            BrushType = 1;
        }
        virtual void draw();
        void rasterCircle(int x0, int y0, int r);
        void circleFill(int xc, int yc, int r);
};

class Polygon2: public Shape {
    public: //4
        Polygon2 (GLWidget *w, QPoint p, RGBColor c, int t, int s) { //Polygon Constructor
            init(w,p,c,t,s);
            BrushType = 1;
            glWidget->setPixel(p1.x(), p1.y(), color, size, BrushType);
        }
        virtual void draw();
        virtual void drawPart();
};

#endif // SHAPES_H
