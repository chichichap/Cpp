//  YOU NEED TO MODIFY THIS FILE FOR YOUR INPAINTING IMPLEMENTATION
//  DO NOT MODIFY THIS FILE ANYWHERE EXCEPT WHERE EXPLICITLY NOTED!!!!

#include "psi.h"
#include <math.h>
#include <stdio.h>

patch_db::patch_db(
			const vil_image_view<vil_rgb<vxl_byte> >& im, 
			vil_image_view<bool> unfilled, int patch_radius
			)

// initialize the size of the patch_center_coords_ matrix to the larged possible number of completely full patches in the image
	: patch_center_coords_(im.ni() * im.nj(), 2)
{
	rows_ = 0;
	w_ = patch_radius;
	plen_ = (2*w_ + 1) * (2*w_ + 1);
	nplanes_ = 3;

// create a local copy of the source image
	vil_copy_deep(im, im_);

// compute the matrix of centers of the patches that are completely full (ie. contain no unfilled pixels)
	compute_patch_centers(im, unfilled);
}

void patch_db::compute_patch_centers(
			const vil_image_view<vil_rgb<vxl_byte> >& im,
			vil_image_view<bool> uf
			)
{
	int plane;
	int i, j;
	bool full;
	int pi, pj;
	int vi;

// loop over all possible patch centers
	top_ = 0;
    for (i=w_; i < im.ni() - w_; i++)
        for (j=w_; j < im.nj() - w_; j++) {
			full = true;
			patch_center_coords_(top_, 0) = i;
			patch_center_coords_(top_, 1) = j;
// loop over all pixels in a patch
            for (pi= -w_, vi=0; (pi<=w_) && full; pi++)
                for (pj= -w_; (pj<=w_) && full; pj++, vi++)
// if pixel is unfilled, the patch is not completely full so this patch cannot be used for lookup operations
                    if (uf(i+pi, j+pj) == true)
						full = false;
			if (full)
				top_ += 1;
		}
}

///////////////////////////////////////////////////////////
//     DO NOT CHANGE ANYTHING ABOVE THIS LINE            //
///////////////////////////////////////////////////////////

bool patch_db::lookup(
			const vnl_matrix<int>* target_planes, 
			int nplanes,
			const vnl_matrix<int>& target_unfilled, 
			int& source_i, 
			int& source_j
			)
{
    int i, j, match;
    double sum, min;

// if the data structures were not correctly initialized, quit the lookup operation
	if (top_ == 0)
		return false;

// if the size of the supplied matrices is NOT equal to the patch size used for contstructing the patch_db object,
// then quit the lookup operation
    for (i=0; i < nplanes; i++)
        if ((target_planes[i].rows() != 2*w_+1) || (target_planes[i].columns() != 2*w_+1) )
			return false;
		
/** ------------------------------- Added Code ----------------------------- **/
// for each full patch in the source region
    int n= 0;
    while (n < top_) {
        sum = 0.0;
// loop over all pixels in a patch
        for (int pi= -w_; pi <= w_; pi++) {
            for (int pj= -w_; pj <= w_; pj++) {
// compare pixel values if pixel in target patch is valid (filled)
                if (!target_unfilled(w_+pi, w_+pj)) {
                    i = patch_center_coords_(n, 0);
                    j = patch_center_coords_(n, 1);

                    sum += pow(target_planes[0](w_+pi, w_+pj) - im_(i+pi,j+pj).r, 2);
                    sum += pow(target_planes[1](w_+pi, w_+pj) - im_(i+pi,j+pj).g, 2);
                    sum += pow(target_planes[2](w_+pi, w_+pj) - im_(i+pi,j+pj).b, 2);
// if min has been exceeded, then we can skip the rest of the pixels and go to the next patch
                    if (sum > min && n > 0)
                        goto nextPatch;
                }
            }
        }

        if (min > sum || n == 0) {
            min = sum;
            match = n;
        }
        nextPatch:
        n++;
    }

// get row and column coordinates of patch center
    source_i = patch_center_coords_(match,0);
    source_j = patch_center_coords_(match,1);

	return true;
}

///////////////////////////////////////////////////////////
//     DO NOT CHANGE ANYTHING BELOW THIS LINE            //
///////////////////////////////////////////////////////////

void patch_db::vectorize(
			const vnl_matrix<int>& mat, 
                  vnl_vector<int>& vec)
{
	int i, j, k; 

	vec.set_size(mat.rows() * mat.columns());

    for (i=0, k=0; i < mat.rows(); i++)
        for (j=0; j < mat.columns(); j++, k++)
			vec(k) = mat(i, j);
}

/////////////////////////////////////////////////////////
