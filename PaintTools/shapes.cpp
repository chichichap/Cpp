#include "shapes.h"

//definitions for Wu's algorithm
#define swap(a,b) {int tmp = a; a = b; b = tmp;}
#define ipart(x) ((int) x)
#define round(x) ((int)(((double)(x))+0.5))
#define fpart(x) (((double) x) - (double)ipart(x))
#define rfpart(x) (1.0 - fpart(x))

/*  This file contains the implementation of the Shape classes
    It holds a description of each shape and code describing
    the behaviour of the each sub function used in order to create a shape
*/

void Shape::FastLine(int x1, int y1, int x2, int y2) {
    bool yLonger = false;
    int incrementVal;

    int shortLen = y2-y1; //dy
    int longLen = x2-x1;  //dx

    if (abs(shortLen) > abs(longLen)) { //if the line is vertical, then swap dy and dx
        swap (shortLen, longLen);
        yLonger = true;
     }

     if (longLen < 0) incrementVal=-1;
     else incrementVal = 1;

     double divDiff;

     if (shortLen==0) divDiff=longLen;
     else divDiff = (double)longLen/(double)shortLen;

     // if y is longer, plot x and y; otherwise, swap x and y when plotting
     if (yLonger)
        for (int i=0;i!=longLen;i+=incrementVal)
          glWidget->setPixel(x1+(int)((double)i/divDiff),y1+i,color, size, BrushType);
     else
        for (int i=0;i!=longLen;i+=incrementVal)
          glWidget->setPixel(x1+i,y1+(int)((double)i/divDiff), color, size, BrushType);
}

void Shape::PolygonScanline() {
    int n = Vertices.count(); //number of polygon edges
    int i,j,k; //counters
    int y,dy,dx;
    std::vector<int> xi (n);      //x-intersects
    std::vector<float> slope (n); //slopes

    int top = Vertices[0].y();
    int bottom = top;

    //find top & bottom pixels
    for (i=0; i < n; i++) {
        if (top > Vertices[i].y()) top = Vertices[i].y();
        if (bottom < Vertices[i].y()) bottom = Vertices[i].y();
    }

    for (i=0; i < n; i++) { //note: mod n for the last case: Vertices[0] - Vertices[n-1]
        dy = Vertices[(i+1)%n].y() - Vertices[i].y();
        dx = Vertices[(i+1)%n].x() - Vertices[i].x();

        if(dy == 0) slope[i] = 1.0; //exceptions of slopes
        if(dx == 0) slope[i] = 0.0;

        if((dy!=0) && (dx!=0)) //calculate inverse slope
            slope[i] = (float) dx/dy;
    }

    for(y = top; y < bottom; y++) {
        k = 0;
        for(i=0; i < n; i++) //note: mod n for the last case: Vertices[0] - Vertices[n-1]
            if( ((Vertices[i].y() <= y) && (Vertices[(i+1)%n].y() > y)) || ((Vertices[i].y() > y)&&(Vertices[(i+1)%n].y() <= y)))
                xi[k++] = (int)(Vertices[i].x() + slope[i]*(y - Vertices[i].y()));

        for(j=0; j < k-1; j++) //arrange x-intersections in order
            for(i=0; i < k-1; i++)
                if(xi[i] > xi[i+1])
                    swap(xi[i], xi[i+1]);

        for(i=0; i < k; i += 2)
            FastLine(xi[i],y,xi[i+1]+1,y);
    }
}

void Scribble::draw() {
    pointsDrawn.append(p2); //add the last point

    p1 = pointsDrawn[0];
    if (BrushType == 1) {
        glWidget->setPixel(p1.x(), p1.y(), color, size, 1);
        for (int i = 1; i < pointsDrawn.count(); i++) {
            p2 = pointsDrawn[i];
            FastLine(p1.x(), p1.y(), p2.x(), p2.y());
            p1 = p2; //next line!
        }
    }
    else { //unlike type 1, other brush types rely mostly on drawing points
        glWidget->setPixel(p1.x(), p1.y(), color, size, BrushType);
        for (int i = 1; i < pointsDrawn.count(); i++) {
            p2 = pointsDrawn[i];
            glWidget->setPixel(p2.x(), p2.y(), color, size, BrushType);
            if (BrushType > 2)
                FastLine(p1.x(), p1.y(), p2.x(), p2.y());
            p1 = p2; //next line!
        }
    }
}

void Scribble::drawPart() {
    pointsDrawn.append(p1); //add the previous point (p1) to the pointsDrawn
    p1 = p2; //and then next time, we will add the current point (p2)
}

void Line::draw() {
    if (p1.x() == p2.x() && p1.y() == p2.y()) return; //don't draw a point!

    if (AA) WuLine(p1.x(), p1.y(), p2.x(), p2.y());
    else BresenhamLine(Vertices[0].x(), Vertices[0].y(), Vertices[1].x(), Vertices[1].y());
}

void Line::drawPart() {
    if (p1.x() == p2.x() && p1.y() == p2.y()) return; //don't draw a point!

    Vertices.clear();
    Vertices.append(p1); //add vertices
    Vertices.append(p2);
}

//Notes: borrowed from the internet
void Line::BresenhamLine(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = -1, sy = -1;
    int err = dx - dy;

    if (x0 < x1) sx = 1;
    if (y0 < y1) sy = 1;

    //keep plotting until x0 reaches x1 and y0 reaches y1
    while (true) {
        glWidget->setPixel(x0, y0, color, size, 1);
        if (x0 == x1 && y0 == y1)
            return;
        int e2 = 2*err;
        if (e2 > -dy) {
            err = err - dy;
            x0 = x0 + sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

//Notes: implemented according to the pseudocode on Wikipedia
//       in case the line is more vertical, we swap x and y everytime we plot something
void Line::WuLine(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    bool steep = (abs(dx) < abs(dy));

    if (steep) {
        swap(x1, y1);
        swap(x2, y2)
        swap(dx, dy);
    }

    if (x2 < x1) {
        swap(x1, x2);
        swap(y1, y2);
    }

    double gradient = (double) dy / (double) dx;

    // handle first endpoint
    int xend = round(x1);
    double yend = y1 + gradient*(xend - x1);
    double xgap = rfpart(x1 + 0.5);
    int xpxl1 = xend; // this will be used in the main loop
    int ypxl1 = ipart(yend);
    if (steep) {
        WuPlot(ypxl1, xpxl1, rfpart(yend)*xgap);
        WuPlot(ypxl1 + 1, xpxl1, fpart(yend)*xgap);
    } else {
        WuPlot(xpxl1, ypxl1, rfpart(yend)*xgap);
        WuPlot(xpxl1, ypxl1 + 1, fpart(yend)*xgap);
    }
    double intery = yend + gradient; // first y-intersection for the main loop

    // handle second endpoint
    xend = round(x2);
    yend = y2 + gradient * (xend - x2);
    xgap = fpart(x2 + 0.5);
    int xpxl2 = xend; // this will be used in the main loop
    int ypxl2 = ipart(yend);
    if (steep) {
        WuPlot (ypxl2, xpxl2, rfpart(yend)*xgap);
        WuPlot (ypxl2 +1, xpxl2, fpart(yend)*xgap);
    } else {
        WuPlot (xpxl2, ypxl2, rfpart(yend)*xgap);
        WuPlot (xpxl2, ypxl2 +1, fpart(yend)*xgap);
    }
    // main loop
    if (steep)
        for (int x = xpxl1+1; x < xpxl2; x++) {
            WuPlot(ipart(intery), x, rfpart(intery));
            WuPlot(ipart(intery)+1, x, fpart(intery));
            intery += gradient;
        }
    else
        for (int x = xpxl1+1; x < xpxl2; x++) {
            WuPlot(x, ipart(intery), rfpart(intery));
            WuPlot(x, ipart(intery)+1, fpart(intery));
            intery += gradient;
        }
}

void Line::WuPlot(int x, int y, double c) {
    RGBColor bg = (glWidget->getPixel(x,y));
    double r,g,b;

    r = color.r*c + bg.r*(1-c); //mixing background and foreground colors
    g = color.g*c + bg.g*(1-c); //since c and 1-c add up to 1
    b = color.b*c + bg.b*(1-c); //these values add up to 255

    bg.r = r;
    bg.g = g;
    bg.b = b;

    glWidget->setPixel(x, y, bg, 1.6, 1); //fixed to be 1.6 for the best result
}


void Rectangle2::draw() {
    if (p1.x() == p2.x() && p1.y() == p2.y()) return; //don't draw a point!

    FastLine(Vertices[0].x(), Vertices[0].y(), Vertices[1].x(), Vertices[1].y()); //top left -> top right
    FastLine(Vertices[1].x(), Vertices[1].y(), Vertices[2].x(), Vertices[2].y()); //top right -> bottom right
    FastLine(Vertices[2].x(), Vertices[2].y(), Vertices[3].x(), Vertices[3].y()); //bottom right -> bottom left
    FastLine(Vertices[3].x(), Vertices[3].y(), Vertices[0].x(), Vertices[0].y()); //bottom left -> top left
    if (fill) PolygonScanline();
}

void Rectangle2::drawPart() {
    if (p1.x() == p2.x() && p1.y() == p2.y()) return; //don't draw a point!

    Vertices.clear(); //since we draw everytime the mouse is moved we want to keep only the latest vertices
    Vertices.append(p1);
    Vertices.append(QPoint(p2.x(), p1.y()));
    Vertices.append(p2);
    Vertices.append(QPoint(p1.x(), p2.y()));
}


void Circle::draw() {
    if (p1.x() == p2.x() && p1.y() == p2.y()) return; //don't draw a point!

    int a = p2.x() - p1.x();
    int b = p2.y() - p1.y();
    int radius = sqrt(pow((double) a,2) + pow((double) b,2)); //calculate the radius
    rasterCircle(p1.x(), p1.y(), radius);
    if (fill) circleFill(p1.x(), p1.y(), radius);
}

//Notes: borrowed from Wikipedia
void Circle::rasterCircle(int x0, int y0, int r) {
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;

    glWidget->setPixel(x0, y0 + r, color, size, 1); //bottom
    glWidget->setPixel(x0, y0 - r, color, size, 1); //top
    glWidget->setPixel(x0 + r, y0, color, size, 1); //right
    glWidget->setPixel(x0 - r, y0, color, size, 1); //left

    //start at x = 0 until y
    while(x < y) {
        if(f >= 0) {
            y--; //decrease the radius
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x;
        glWidget->setPixel(x0 + x, y0 + y, color, size, 1); //plot around the circle with decreasing radius
        glWidget->setPixel(x0 - x, y0 + y, color, size, 1);
        glWidget->setPixel(x0 + x, y0 - y, color, size, 1);
        glWidget->setPixel(x0 - x, y0 - y, color, size, 1);
        glWidget->setPixel(x0 + y, y0 + x, color, size, 1);
        glWidget->setPixel(x0 - y, y0 + x, color, size, 1);
        glWidget->setPixel(x0 + y, y0 - x, color, size, 1);
        glWidget->setPixel(x0 - y, y0 - x, color, size, 1);
    }
}

//Notes: borrowed from the internet
void Circle::circleFill(int xc, int yc, int r) {
    int x1, x2;
    for (int y = yc-r; y <= yc+r; ++y) {
        x1 = round(xc + sqrt(pow((double) r,2) - pow((double) y-yc,2)));
        x2 = round(xc - sqrt(pow((double) r,2) - pow((double) y-yc,2)));

        FastLine(x1, y, x2, y);
    }
}

void Polygon2::draw() {
    int n = Vertices.count();
    for (int i=1; i < n; i++) //draw all lines between all vertices
        FastLine(Vertices[i-1].x(), Vertices[i-1].y(), Vertices[i].x(), Vertices[i].y());
    FastLine(Vertices[0].x(), Vertices[0].y(), Vertices[n-1].x(), Vertices[n-1].y());
    if (fill) PolygonScanline();
}

void Polygon2::drawPart() {
    FastLine(p1.x(), p1.y(), p2.x(), p2.y()); //draw the line between the previous and current points
    Vertices.append(p1); //then add the previous point to the Vertices
    p1 = p2;
}
