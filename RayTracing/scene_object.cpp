#include "cmath"
#include <iostream>
#include "scene_object.h"
#include <stdio.h>

bool UnitSquare::intersect( Ray3D& ray, const Matrix4x4& worldToModel, const Matrix4x4& modelToWorld ) {
    // TODO: implement intersection code for UnitSquare, which is
    // defined on the xy-plane, with vertices (0.5, 0.5, 0),
    // (-0.5, 0.5, 0), (-0.5, -0.5, 0), (0.5, -0.5, 0), and normal
    // (0, 0, 1).
    //
    // Your goal here is to fill ray.intersection with correct values
    // should an intersection occur.  This includes intersection.point,
    // intersection.normal, intersection.none, intersection.t_value.
    //
    // HINT: Remember to first transform the ray into object space
    // to simplify the intersection test.

    double t_val;

    Vector3D n(0.0, 0.0, 1.0);

    ray.origin = worldToModel*ray.origin;
    ray.dir = worldToModel*ray.dir;

    // ray is paralel to plane
    if (ray.dir.dot(n) == 0) {
        ray.origin = modelToWorld*ray.origin;
        ray.dir = modelToWorld*ray.dir;
        return false;
    }

    t_val = n.dot(Point3D() - ray.origin) / ray.dir.dot(n);

    Point3D PointOnPlane = ray.origin + t_val*ray.dir;

    if (t_val < 0.0 || std::abs(PointOnPlane[0]) > 0.5 || std::abs(PointOnPlane[1]) > 0.5) {
        ray.origin = modelToWorld*ray.origin;
        ray.dir = modelToWorld*ray.dir;
        return false;
    }

    PointOnPlane = modelToWorld*PointOnPlane;

    ray.origin = modelToWorld*ray.origin;
    ray.dir = modelToWorld*ray.dir;

    t_val = (PointOnPlane - ray.origin).length();

    if (ray.intersection.none == false && ray.intersection.t_value < t_val)
        return false;

    if (t_val < 0.01) return false;

    ray.intersection.t_value = t_val;
    ray.intersection.point = PointOnPlane;
    ray.intersection.normal = modelToWorld*n; //transNorm(worldToModel, n); //modelToWorld*n; //*** transNorm
    ray.intersection.none = false;
    return true;
}

bool UnitSphere::intersect( Ray3D& ray, const Matrix4x4& worldToModel, const Matrix4x4& modelToWorld ) {
    double t_val;

    ray.origin = worldToModel*ray.origin;
    ray.dir = worldToModel*ray.dir;

    Vector3D a = ray.origin - Point3D();

    double A = ray.dir.dot(ray.dir);
    double B = ray.dir.dot(a);
    double C = a.dot((a)) - 1.0;
    double D = B*B - A*C;

    if (D < 0) { // no intersection
        ray.origin = modelToWorld*ray.origin;
        ray.dir = modelToWorld*ray.dir;
        return false;
    } else if (D == 0) { // one intersection
        t_val = -B/A;
    } else { // two intersections
        double t_val1 = -B/A + sqrt(D)/A;
        double t_val2 = -B/A - sqrt(D)/A;

        if (t_val1 > 0 && t_val2 > 0) t_val = (t_val1 < t_val2) ? t_val1 : t_val2;
        else {
            ray.origin = modelToWorld*ray.origin;
            ray.dir = modelToWorld*ray.dir;
            return false;
        }
    }

    Point3D PointOnSphere = modelToWorld*(ray.origin + t_val*ray.dir);
    Vector3D n = (ray.origin + t_val*ray.dir) - Point3D(0,0,0);

    ray.origin = modelToWorld*ray.origin;
    ray.dir = modelToWorld*ray.dir;

    ray.dir.normalize();
    t_val = (PointOnSphere - ray.origin).length();

    if (ray.intersection.none == false && ray.intersection.t_value < t_val) return false;

    ray.intersection.t_value = t_val;
    ray.intersection.point = PointOnSphere;
    ray.intersection.normal = transNorm(worldToModel, n);
    ray.intersection.none = false;
    return true;
}

