// MODIFY THIS FILE FOR YOUR INPAINTING IMPLEMENTATION
// See inpainting_eval.h for a detailed explanation of the input and output parameters of the routines you must implement

#include "inpainting_eval.h"
#include "math.h"

// When beginning to write your code, I suggest you work in three steps:
// 
//  - First, implement the routine for computing the confidence term.
//    To help with debugging, you should let compute_D() always return 1,
//    so that patches are selected purely based on their confidence term.
//    (correctness of the confidence computation routine should be relatively easy to check
//    since if it's incorrect, patches with fewer filled pixels will end up having higher priorities and will be chosen first)
//
//  - Second, implement the routine that does the patch lookup. The correctness of the lookup routine should be easy to debug,
//    since if it is incorrect the routine will be choosing patches that look nothing like the patch on the fill front.
//    Together, these two steps will allow you to get somewhat reasonable inpaintings done.
//
//  - Third, implement the data term computation routine. You should also try to do this in steps to help with debugging:
//    (1) make your compute_C() function return 1, so that patch priorities are computed according to the data term.
//    (2) make your compute_D() function return just the magnitude of the gradient computed by the compute_gradient() routine
//    ---this will help you debug gradient computatoins, since an incorrect computatoin will cause patches that contain 
//    very low intensity variation to be selected before patches with lots of intensity variations.
//    (3) once you are satisfied that gradient computatoins are correct, move on to the normal computation routine, etc.
// 
//  - Only when the above routines are correct individually should you try to compute the priorities as the product C()*D().
//    Otherwise, if the patch selections 'don't look right' you won't know what is causing this behavior.

double compute_D(psi& PSI, 
				 const vil_image_view<vxl_byte> im,
				 const vil_image_view<bool> unfilled, 
				 const vil_image_view<bool> fill_front, 
				 double alpha,
				 vnl_double_2& gradient, 
				 vnl_double_2& front_normal)
{
    vnl_double_2 grad_normal; // holds the perpendicular to the gradient

// compute the gradient at a filled pixel in the direction of the front normal
	if (compute_gradient(PSI, im, unfilled, gradient)) {

		grad_normal(0) = -gradient(1);
        grad_normal(1) =  gradient(0);

// now compute the normal of the fill front
        if (compute_normal(PSI, fill_front, front_normal))
            return fabs(dot_product(grad_normal, front_normal)) / alpha;
        else if (alpha > 0)
            return 1 / alpha;
        else
			return 0;
    } else
        return 0;
// if we cannot compute a normal, the fill boundary consists of exactly one pixel; the data term in this case is meaningless
// so we just return a default value
}

/** ----------------------- Implemented Routines -------------------------------- **/

double compute_C(      psi&                    PSI,
                 const vil_image_view<double>& C,
                 const vil_image_view<bool>&   unfilled)
{
    double sum = 0.0;

// for each pixel in the patch
    for(PSI.begin(); !PSI.end(); PSI.next()) {
        int i,j;
        PSI.image_coord(i,j);
// get the confidence value of the pixel if it's filled (if unfilled, confidence = 0)
        if (unfilled(i,j) == false)
            sum += C(i,j);
    }
    return sum / pow(PSI.sz(), 2); // since size = 2*w + 1
}

bool compute_normal(psi&                 PSI,
					vil_image_view<bool> fill_front, 
                    vnl_double_2&        normal)
{
    int w = PSI.w(), ceni = (int) PSI.p()(0), cenj = (int) PSI.p()(1);

    vcl_vector<int> boundary_i;
    vcl_vector<int> boundary_j;
    vil_trace_8con_boundary(boundary_i, boundary_j, fill_front, ceni, cenj);
    int bsize = boundary_i.size();

// single pixel on the fill boundary (used 2 because w=0 won't give much information)
    if (bsize <= 2)
        return false;
// if we have a region smaller than the patch, then we have less information
    if (w > bsize)
        w = (bsize-1) / 2;

    vnl_matrix<double> X(2*w+1, 3);
    vnl_vector<double> Ix(2*w+1);
    vnl_vector<double> Iy(2*w+1);
    vnl_matrix<double> W(2*w+1, 2*w+1, 0); //Gaussian Weight Function

// fill in values from positions -w to -1
    for (int i=0; i < w; i++) {
        Ix[i] = boundary_j[bsize-1-i];
        Iy[i] = boundary_i[bsize-1-i];
    }
// fill in values at position 0
    Ix[w] = cenj; // or boundary_j[0]
    Iy[w] = ceni; // or boundary_i[0]
// fill in values from positions 1 to w
    for (int i= 1; i <= w; i++) {
        Ix[w+i] = boundary_j[i];
        Iy[w+i] = boundary_i[i];
    }
// set up matrix X and W
    for (int i=0; i <= 2*w; i++) {
            X(i,0) = 1;
            X(i,1) = i - w;
            X(i,2) = pow(i-w, 2) / 2;
            W(i,i) = exp(-pow(i-w, 2));
    }
// apply gaussian weight function
    Ix = W * Ix;
    Iy = W * Iy;
    X  = W * X;

// solve for d to get the tangent
    vnl_matrix<double> inverseX = vnl_svd<double>(X).pinverse(3);
    vnl_vector<double> d1 = inverseX * Ix;
    vnl_vector<double> d2 = inverseX * Iy;
    double dx = d1[1];
    double dy = d2[1];
    double length = sqrt(dx*dx + dy*dy);

// calculate the normal from the tangent
    normal(0) =  dx / length;
    normal(1) = -dy / length;
    return true;
}

// return the gradient with the strongest magnitude inside the patch of radius w or return false if no gradients can be computed
bool compute_gradient(      psi&                        PSI,
                      const vil_image_view<vxl_byte>&   inpainted_grayscale,
                      const vil_image_view<bool>&       unfilled,
                            vnl_double_2&               grad)
{
    bool computable = false;
    double maxGradient = 0;
    vnl_vector<double> sobelX(9);
    vnl_vector<double> sobelY(9);
// setup sobel mask for x coordinate
    sobelX(0) = -1;
    sobelX(1) =  0;
    sobelX(2) =  1;
    sobelX(3) = -2;
    sobelX(4) =  0;
    sobelX(5) =  2;
    sobelX(6) = -1;
    sobelX(7) =  0;
    sobelX(8) =  1;
// setup sobel mask for y coordinate
    sobelY(0) = -1;
    sobelY(1) = -2;
    sobelY(2) = -1;
    sobelY(3) =  0;
    sobelY(4) =  0;
    sobelY(5) =  0;
    sobelY(6) =  1;
    sobelY(7) =  2;
    sobelY(8) =  1;

// for each pixel in the patch (excluding the ones next to image border or unfilled pixels)
    for(PSI.begin(); !PSI.end(); PSI.next()) {
        int i,j;

        PSI.image_coord(i,j);
        if (i == 0 || j == 0 || i == unfilled.ni()-1 || j == unfilled.nj()-1 ||
            unfilled(i-1,j-1) || unfilled(i-1,j) || unfilled(i-1,j+1) ||
            unfilled(i  ,j-1) || unfilled(i  ,j) || unfilled(i  ,j+1) ||
            unfilled(i+1,j-1) || unfilled(i+1,j) || unfilled(i+1,j+1))
            continue;
        else
            computable = true;

        vnl_double_2 cen;
        cen(0) = i;
        cen(1) = j;
// create a 3x3 patch
        psi p(cen, 1, unfilled.ni(), unfilled.nj());
        vnl_vector<double> I(9);

// for each pixel in the 3x3 patch
        int n = 0;
        for (p.begin(); !p.end(); p.next()) {
            int r,c;
            p.image_coord(r,c);
// get the grayscale value at that pixel in the image
            I(n++) = inpainted_grayscale(r,c);
        }

        double vx = dot_product(sobelX, I);
        double vy = dot_product(sobelY, I);
        double magnitude = sqrt(vx*vx + vy*vy);

        if (maxGradient < magnitude) {
            maxGradient = magnitude;
            grad(0) = vy / magnitude;
            grad(1) = vx / magnitude;
        }
    }
    return computable;
}
