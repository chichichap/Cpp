//
//  USE THIS FILE FOR YOUR MATTING-SPECIFIC CODE
//
//  DO NOT MODIFY THIS FILE ANYWHERE EXCEPT WHERE
//  EXPLICITLY NOTED!!!!
//




#include "matting.h"


//
//  The Triangulation Matting algorithm
//
//

bool matting::compute(void)
{
    if ((alpha_computed_ == true) && (outdated_ == false))
        // the results have already been computed
        return true;

    if (((bool) comp_1_ == false) ||
        ((bool) comp_2_ == false) ||
        ((bool) back_1_ == false) ||
        ((bool) back_2_ == false))
        // not enough information yet to run the algorithm
        return false;

    // Now we have enough information to proceed

    // first let us allocate space for the result images
    alpha_.set_size(ni_, nj_);
    object_.set_size(ni_, nj_);


    //////////////////////////////////////////////////
    // PLACE YOUR CODE BETWEEN THESE LINES          //
    //////////////////////////////////////////////////

    for (int i=0; i < ni_; i++) {
        for (int j=0; j < nj_; j++) {
            vnl_matrix<double> A(6,4, 0.0);
            vnl_matrix<double> b(6,1);

            A(0,0) = 1.0;
            A(1,1) = 1.0;
            A(2,2) = 1.0;
            A(0,3) = -1*back_1_(i,j).r;
            A(1,3) = -1*back_1_(i,j).g;
            A(2,3) = -1*back_1_(i,j).b;

            A(3,0) = 1.0;
            A(4,1) = 1.0;
            A(5,2) = 1.0;
            A(3,3) = -1*back_2_(i,j).r;
            A(4,3) = -1*back_2_(i,j).g;
            A(5,3) = -1*back_2_(i,j).b;

            b(0,0) = comp_1_(i,j).r - back_1_(i,j).r;
            b(1,0) = comp_1_(i,j).g - back_1_(i,j).g;
            b(2,0) = comp_1_(i,j).b - back_1_(i,j).b;

            b(3,0) = comp_2_(i,j).r - back_2_(i,j).r;
            b(4,0) = comp_2_(i,j).g - back_2_(i,j).g;
            b(5,0) = comp_2_(i,j).b - back_2_(i,j).b;

            vnl_matrix<double> y = vnl_svd<double>(A).pinverse(4);
            vnl_matrix<double> x = y * b;

            if (x(0,0) > 255)
                x(0,0) = 255;
            if (x(1,0) > 255)
                x(1,0) = 255;
            if (x(2,0) > 255)
                x(2,0) > 255;

            if (x(0,0) < 0)
                x(0,0) = 0;
            if (x(1,0) < 0)
                x(1,0) = 0;
            if (x(2,0) < 0)
                x(2,0) = 0;

            object_(i,j).r = x(0,0);
            object_(i,j).g = x(1,0);
            object_(i,j).b = x(2,0);

            if (x(3,0) > 1)
                x(3,0) = 1;

            if (x(3,0) < 0)
                x(3,0) = 0;

            alpha_(i,j) = x(3,0)*255;

        }
    }
    /////////////////////////////////////////////////

    alpha_computed_ = true;
    outdated_ = false;

    return true;
}

bool matting::compute_composite(vil_image_view<vil_rgb<vxl_byte> > input_im,
                                vil_image_view<vil_rgb<vxl_byte> > &output_im)
{
    //////////////////////////////////////////////////
    // PLACE YOUR CODE BETWEEN THESE LINES          //
    //////////////////////////////////////////////////

    if (!alpha_computed_ || outdated_)
        return false;

    output_im.set_size(ni_, nj_);
    new_back_.set_size(ni_, nj_);
    new_comp_.set_size(ni_, nj_);

    for (int i=0; i < ni_; i++) {
        for (int j=0; j < nj_; j++) {

            double a = (int)alpha_(i,j) / 255.0; // convert back to [0.0 - 1.0]

            output_im(i,j).r = a * (int)object_(i,j).r + (1 - a) * (int)input_im(i,j).r;
            output_im(i,j).g = a * (int)object_(i,j).g + (1 - a) * (int)input_im(i,j).g;
            output_im(i,j).b = a * (int)object_(i,j).b + (1 - a) * (int)input_im(i,j).b;

            new_back_(i,j).r = (int)input_im(i,j).r;
            new_back_(i,j).g = (int)input_im(i,j).g;
            new_back_(i,j).b = (int)input_im(i,j).b;

            new_comp_(i,j).r = (int)output_im(i,j).r;
            new_comp_(i,j).g = (int)output_im(i,j).g;
            new_comp_(i,j).b = (int)output_im(i,j).b;
        }
    }
    //////////////////////////////////////////////////
    new_composite_computed_ = true;

    return true;
}

//////////////////////////////////////////////////
// PLACE ANY ADDITIONAL CODE BETWEEN THESE LINES//
//////////////////////////////////////////////////

//////////////////////////////////////////////////
