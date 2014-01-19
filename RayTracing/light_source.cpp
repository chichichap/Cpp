#include <cmath>
#include "light_source.h"
#include <stdio.h>

void PointLight::shade( Ray3D& ray, bool blocked ) {
    // TODO: implement this function to fill in values for ray.col
    // using phong shading.  Make sure your vectors are normalized, and
    // clamp colour values to 1.0.
    //
    // It is assumed at this point that the intersection information in ray
    // is available.  So be sure that traverseScene() is called on the ray
    // before this function.
    if (blocked) {
        for (int i=0; i < 3; i++)
            ray.col[i] = ray.col[i] + ray.intersection.mat->ambient[i]*_col_ambient[i];
        return;
    }

    Vector3D n = ray.intersection.normal;
    Vector3D s = _pos - ray.intersection.point; // direction to light source
    Vector3D m = 2*n.dot(s)*n - s; // direction of perfect mirror emitting light
    Vector3D toCam = -ray.dir;// ray.origin - ray.intersection.point;

    n.normalize();
    s.normalize();
    m.normalize();
    toCam.normalize();

    double dotProduct1 = n.dot(s); // foreshortenning term
    double dotProduct2 = m.dot(toCam);

    if (dotProduct1 < 0) dotProduct1 = 0.0;
    if (dotProduct2 < 0) dotProduct2 = 0.0;

    for (int i=0; i < 3; i++) {
        ray.col[i] = ray.col[i]
                   + ray.intersection.mat->diffuse[i]*_col_diffuse[i]*dotProduct1
                   + ray.intersection.mat->ambient[i]*_col_ambient[i]
                   + ray.intersection.mat->specular[i]*_col_specular[i]*pow(dotProduct2, ray.intersection.mat->specular_exp);
    }

    ray.col.clamp();
}

