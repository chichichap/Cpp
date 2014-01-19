#include "morphing.h"

// 
// Top-level morphing routine
//
// The routine runs the basic morphing algorithm several times in order to generate a sequence of intermediate morphed images between t=0 and t=1.
// 
// The number of intermediate images is controlled by the num_images_ parameter of the morphing class and is user-selectable
//
bool morphing::compute()
{
	int iter;
	bool ok = true;

// if num_images > 1, we compute the t_ parameter automatically before executing the morph
	if (num_images_ > 1) 
		for (iter=0; (iter<num_images_) && (ok); iter++) {
// set the t parameter value for this iteration
			set_t((iter+1)*1.0/(num_images_+1));		
	
// compute the morph for this setting of the parameter
			vcl_cerr << "Computing morph for t=" << get_t() << "\n";
			ok = ok && morph_iteration(iter);
		} 
	else {
// otherwise, we just run the algorithm once, for the current setting of the t parameter
		vcl_cerr << "Computing morph for t=" << get_t() << "\n";
		ok = morph_iteration(0);
	}
	return ok;
}

// 
// Top-level routine for the creation of a single morphed image
//
// 1. check whether a morph can be computed (eg. that both images I0 and I1 have been specified)
// 2. call the routine that implements the Beier-Neely morphing algorithm
// 3. write the results to disk, if specified by the user
//
// the variable iter runs between 0 and num_images_ and is used only for file numbering when writing images to disk
// 
bool morphing::morph_iteration(int iter)
{
	if ((morph_computed_ == false) || (outdated_ == true)) {

		if (outdated_ == true)
			morph_computed_ = false; // need to recalculate everything

		if (((bool) I0_ == false) || ((bool) I1_ == false))
			return false; // not enough information to run the algorithm yet = nothing to do

// allocate space for all images
		warped_I0_.set_size(I0_.ni(), I0_.nj());
		warped_I1_.set_size(I0_.ni(), I0_.nj());
		morph_.set_size(I0_.ni(), I0_.nj());

		compute_morph();

// update the compute-related flags
		morph_computed_ = true;
		outdated_ = false;

	} else {
		// the results have already been computed = nothing to do
	}

// write to disk
	if (write_morph_ == true) {
// build the file name in the form <basefilename>.XXX.jpg where XXX is the zero-padded iteration number
		char fname[256];
		vcl_ostringstream mfname(fname);
		mfname 
			<< morph_basename_ << "." 
			<< vcl_setfill('0') << vcl_setw(3) << iter 
			<< ".jpg" << vcl_ends;
		 
// to save, we need to access a (char *) representation of the output string stream
		vcl_cerr << "writing Morph to file:" 
			<< (mfname.str()).c_str() << "\n";
		vil_save((vil_image_view<vxl_byte>)morph_, 
			     (mfname.str()).c_str());
	}
	if (write_warped_ == true) {
		char fname0[256];
		char fname1[256];
		vcl_ostringstream w0fname(fname0);
		vcl_ostringstream w1fname(fname1);

		w0fname 
			<< morph_basename_ << "." 
			<< "W0." 
			<< vcl_setfill('0') << vcl_setw(3) << iter 
			<< ".jpg" << vcl_ends;
		w1fname 
			<< morph_basename_ << "." 
			<< "W1." 
			<< vcl_setfill('0') << vcl_setw(3) << iter 
			<< ".jpg" << vcl_ends;

		vcl_cerr << "writing WarpedI0 to file " 
			<< (w0fname.str()).c_str() << "\n";
		vil_save((vil_image_view<vxl_byte>)warped_I0_, 
			     (w0fname.str()).c_str());
		vcl_cerr << "writing WarpedI1 to file " 
			<< (w1fname.str()).c_str() << "\n";
		vil_save((vil_image_view<vxl_byte>)warped_I1_, 
			     (w1fname.str()).c_str());
	}

	return true;
}

//
// Top-level implementation of the Beier-Neely morphing algorithm
//
// The routine should call the field_warp() routine as a subroutine for warping the images stored in variables I0_ and I1_
// 
// Specifications:
//   Inputs: The routine should assume that the following private class variables contain valid data:
//
//   * The two source images, I0_ and I1_
//   * I0I1_linepairs_ holds the set of corresponding line pairs between images I0_ and I1_
//   * The class variables a_, b_, p_ holding the parameters of the field warping algorithm
//   * The parameter t_ that determines the in between warp between images I0_ and I1_
//
//   Outputs: The following private class variables are assumed to have valid data after the routine
//            returns:
//
//   * warped_I0_: the image holding the result of applying the field warp algorithm to image I0_
//   * warped_I1_: the image holding the result of applying the field warp algorithm to image I1_
//   * morph_: the image holding the result morphing images I0_ and I1_
//   * I0W0_linepairs_: the set of corresponding line pairs between images I0_ and warped_I0_

void morphing::compute_morph()
{
	I0W0_linepairs_ = I0I1_linepairs_.interpolate(t_);
	linepairs swapLP = I0I1_linepairs_.swap();
	linepairs I1W1_linepairs_ = swapLP.interpolate(t_);

	field_warp(I0_, I1W1_linepairs_, a_, b_, p_, warped_I0_);
	field_warp(I1_, I0W0_linepairs_, a_, b_, p_, warped_I1_);

// step 3: linearly blend the morphed images
	for (int i=0; i < I0_.ni(); i++) {
		for (int j=0; j < I0_.nj(); j++) {
			morph_(i,j).r = warped_I0_(i,j).r*t_ + warped_I1_(i,j).r*(1.0-t_);
			morph_(i,j).g = warped_I0_(i,j).g*t_ + warped_I1_(i,j).g*(1.0-t_);
			morph_(i,j).b = warped_I0_(i,j).b*t_ + warped_I1_(i,j).b*(1.0-t_);
		}
	}

	// create a linepair where bot the original and the warped lines are identical and equal to the user-specified lines in image I0
	//I0W0_linepairs_ = I0I1_linepairs_.copy(0, 1);
}

// 
// Routine that implements the Beier-Neely field warping algorithm
//
// Input variables:
//   source:      the vxl image to be warped
//   linepairs:   the set of corresponding line pairs between the source image and the destination image
//   a,b,p:       the field warping parameters
//
// Output variables:
//   destination: the warped image. the routine should assume that memory for this image has already been allocated
// 														  and that the image is of identical size as the source image
// 
void morphing::field_warp(
		const vil_image_view<vil_rgb<vxl_byte> >& 	source,
										linepairs&  lps,
		double a, double b, double p,
			   vil_image_view<vil_rgb<vxl_byte> >& 	destination)
{
	int h = destination.ni(), w = destination.nj();
	double weightsum, weight, u, v, dist, x, y;

	vnl_matrix<double> P,Q,Pprime,Qprime;

	lps.get(P,Q,Pprime,Qprime);

	vnl_vector_fixed<double,2> DSUM, Xprime, D;
	vnl_vector_fixed<double,2> XP, XQ, QP, perpQP, QprimePprime, perpQprimePprime;

// for each pixel X
	for (int i=0; i < h; i++) {
		for (int j=0; j < w; j++) {
			DSUM = vnl_vector_fixed<double,2>(0.0,0.0);
			weightsum = 0.0;
// for each line (P,Q)
			for (int l=0; l < P.cols(); l++) {
				XP = vnl_vector_fixed<double,2>(i-P(0,l), j-P(1,l));
				XQ = vnl_vector_fixed<double,2>(i-Q(0,l), j-Q(1,l));
				QP = vnl_vector_fixed<double,2>(Q(0,l)-P(0,l), Q(1,l)-P(1,l));
				perpQP = vnl_vector_fixed<double,2>(QP(1), -QP(0));
				QprimePprime = vnl_vector_fixed<double,2>(Qprime(0,l)-Pprime(0,l), Qprime(1,l)-Pprime(1,l));
				perpQprimePprime = vnl_vector_fixed<double,2>(QprimePprime(1), -QprimePprime(0));

				u = dot_product(XP,	   QP) / pow(QP.magnitude(), 2);
				v = dot_product(XP,perpQP) /     QP.magnitude();

				Xprime(0) = Pprime(0,l) + u * QprimePprime(0) + v * perpQprimePprime(0) / QprimePprime.magnitude();
				Xprime(1) = Pprime(1,l) + u * QprimePprime(1) + v * perpQprimePprime(1) / QprimePprime.magnitude();

				D(0) = Xprime(0) - i;
				D(1) = Xprime(1) - j;

				if (u < 0.0)
					dist = XP.magnitude();
				else if (u > 1.0)
					dist = XQ.magnitude();
				else
					dist = abs(v);

				weight = pow(pow(QP.magnitude(), p) / (a + dist), b);

				DSUM(0) += D(0) * weight;
				DSUM(1) += D(1) * weight;
				weightsum += weight;
			}

			y = i + DSUM(0) / weightsum;
			x = j + DSUM(1) / weightsum;

			if (y > h-1)
				y = h-1;
			else
				y = floor(y+0.5);

			if (x > w-1)
				x = w-1;
			else
				x = floor(x+0.5);

			destination(i,j).r = source((int)y, (int)x).r;
			destination(i,j).g = source((int)y, (int)x).g;
			destination(i,j).b = source((int)y, (int)x).b;
		}
	}
}
