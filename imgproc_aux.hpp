/**
 *	@file		imgproc_aux.hpp
 *	@brief		Defines the image class and several utility functions
 *	@author		seonho.oh@gmail.com
 *	@date		2013-07-01
 *	@version	1.0
 *
 *	@section	LICENSE
 *
 *		Copyright (c) 2007-2013, Seonho Oh
 *		All rights reserved. 
 * 
 *		Redistribution and use in source and binary forms, with or without  
 *		modification, are permitted provided that the following conditions are  
 *		met: 
 * 
 *		    * Redistributions of source code must retain the above copyright  
 *		    notice, this list of conditions and the following disclaimer. 
 *		    * Redistributions in binary form must reproduce the above copyright  
 *		    notice, this list of conditions and the following disclaimer in the  
 *		    documentation and/or other materials provided with the distribution. 
 *		    * Neither the name of the <ORGANIZATION> nor the names of its  
 *		    contributors may be used to endorse or promote products derived from  
 *		    this software without specific prior written permission. 
 * 
 *		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS  
 *		IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED  
 *		TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  
 *		PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER  
 *		OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  
 *		EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,  
 *		PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR  
 *		PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF  
 *		LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING  
 *		NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS  
 *		SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#pragma once

#ifdef USE_OPENCV

#ifdef _MSC_VER
#include <mycv.hpp>	// for OpenCV
#pragma comment(lib, OPENCV_LIB_EXPAND("core"))
#pragma comment(lib, OPENCV_LIB_EXPAND("imgproc"))
#pragma comment(lib, OPENCV_LIB_EXPAND("highgui"))
#pragma comment(lib, OPENCV_LIB_EXPAND("video"))
#else
#include <opencv2/opencv.hpp>
#endif

#endif

#include <armadillo>
using namespace arma;

#include "arma_ext.hpp"

namespace auxiliary
{
    /**
	 *	@brief	Template class for size type
	 *	@tparam	T	the type of width and height
	 */
	template <typename T>
	class Size : public arma::Col<T>::template fixed<2>
	{
	public:
        
		Size(T w = 0, T h = 0)
		{
			this->at(0) = w;
			this->at(1) = h;
		}
        
		T width() const     { return this->at(0); }
		T height() const    { return this->at(1); }
        
		T& width()          { return this->at(0); }
		T& height()         { return this->at(1); }
	};
    
	//!	A template image class.
	template <typename T>
	class Image : public Mat<T>
	{
	public:
		typedef T									elem_type;	///< the type of elements stored in the matrix
		typedef typename get_pod_type<T>::result	pod_type;	///< if eT is non-complex, pod_type is same as eT. otherwise, pod_type is the underlying type used by std::complex

		typedef arma::uword							size_type;

		/// Constructor
		Image(): Mat<T>() {}

		/// Constructor
		Image(const Mat<T>& m): Mat<T>(m) {}

		/// Constructor
		Image(const size_type width, const size_type height) : Mat<T>(height, width) {}
		
		/// Constructor
		template <typename DT>
		Image(const Mat<DT>& m): Mat<T>(m.n_rows, m.n_cols)
		{
			T* ptr = this->memptr();
			// type conversion
#ifdef _MSC_VER
			concurrency::parallel_for(size_type(0), m.n_elem, [&](size_type i) {
#else
            for (size_type i = 0 ; i < m.n_elem ; i++)
#endif
				ptr[i] = arma_ext::saturate_cast<T>(m(i));
#ifdef _MSC_VER
			});
#endif
		}

		/// Dummy function
		void release() {}

		/// Get image width
		inline size_type width() const { return this->n_cols; }

		/// Get image height
		inline size_type height() const { return this->n_rows; }

		/// Resize image
		inline void resize(size_type width, size_type height)
		{
			this->set_size(height, width);
		}

		// Operator overloading
		operator Mat<T>&()
		{
			return static_cast<Mat<T> >(*this);
		}
	};

	/**
	 *	@brief	Retrieves a pixel rectangle from an image with sub-pixel accuracy.
	 *	@param img source image
	 *	@param patchsize	The size of the extracted patch.
	 *	@param center		The coordinate of the center of the extracted rectangle within the source image.
	 *						The center must be inside of image.
	 *	@return	Extracted patch that has the size @c patchsize and the same format as @c img.
	 *			This function extracts pixels from src:
	 *			\f[
	 *				dst(x, y) = src(x + center.x - (dst.cols - 1) * 0.5, y + center.y - (dst.rows - 1) * 0.5)
	 *			\f]
	 *			where the values of the pixels at non-integer coordinates are retrieved using bilinear interpolation.
	 *			While the center of the rectangle must be inside the image, parts of the rectangle may be outside.
	 *			In this case, extrapolate the pixel values by replication border condition.
	 */
	template <typename pixel_type, typename vec_type>
	static arma::Mat<pixel_type> getRectSubPix(const Image<pixel_type>& img, Size<arma_ext::uword> patchsize, const vec_type center)
	{
		typedef typename vec_type::elem_type elem_type;
		arma::Mat<pixel_type> out(patchsize.height(), patchsize.width());

        typename arma::Col<elem_type>::template fixed<2> center_ = center;

		center_(0) -= (patchsize.width() - 1) * (elem_type)0.5;
		center_(1) -= (patchsize.height() - 1) * (elem_type)0.5;

		int ipx = (int)std::floor(center_(0));
		int ipy = (int)std::floor(center_(1));

		elem_type ox = center_(0) - ipx;
		elem_type oy = center_(1) - ipy;
		
		elem_type a11 = (1 - ox) * (1 - oy),
				  a12 =      ox  * (1 - oy),
				  a21 = (1 - ox) * oy,
				  a22 =      ox  * oy;

		if (0 <= ipx && ipx + patchsize.width() < img.n_cols &&
			0 <= ipy && ipy + patchsize.height() < img.n_rows) {
			// extracted rectangle is totally inside the image
#ifdef _MSC_VER
			concurrency::parallel_for(arma_ext::size_type(0), out.n_cols, [&](arma_ext::size_type j) {
#else
            for (arma_ext::size_type j = 0 ; j < out.n_cols ; j++) {
#endif
				pixel_type* ptr = out.colptr(j);
				const pixel_type* src = img.colptr(ipx + j) + ipy;
				arma_ext::size_type i;
				for (i = 0 ; i < out.n_rows ; i++) {
					// bilinear interpolation
					ptr[i] = arma_ext::saturate_cast<pixel_type>(src[i                 ] * a11 +
															src[i + 1             ] * a21 +
															src[i + img.n_rows    ] * a12 +
															src[i + img.n_rows + 1] * a22);
				}
#ifdef _MSC_VER
			});
#else
			}
#endif
		} else {
			arma::ivec4 r;

			// adjust rectangle
			int sox = 0, soy = 0;

			// -------- begin --------
			if (ipx >= 0) {
				sox += ipx;	// + ipx
				r(0) = 0;
			} else {
				r(0) = -ipx;
				if (r(0) > (int)patchsize.width())
					r(0) = patchsize.width();
			}
			
			if (ipx + (int)patchsize.width() < (int)img.n_cols)
				r(2) = patchsize.width();
			else {
				r(2) = (int)img.n_cols - ipx - 1;
				if (r(2) < 0) {
					sox += r(2);	// + width
					r(2) = 0;
				}
			}

			if (ipy >= 0) {
				soy += ipy;	// + ipy
				r(1) = 0;
			} else {
				r(1) = -ipy;
				if (r(1) > (int)patchsize.height())
					r(1) = patchsize.height();
			}

			if (ipy + (int)patchsize.height() < (int)img.n_rows)
				r(3) = patchsize.height();
			else {
				r(3) = (int)img.n_rows - ipy - 1;
				if (r(3) < 0) {
					soy += r(3);	// + height
					r(3) = 0;
				}
			}
			// --------- end ---------

			//soy -= r(1);

			elem_type b1 = (elem_type)1.0 - ox,
					  b2 = ox;

			const pixel_type* src1 = img.colptr(sox) + soy;
			for (arma_ext::size_type j = 0 ; j < out.n_cols ; j++) {
				pixel_type* ptr = out.colptr(j);
				const pixel_type* src2 = src1 + img.n_rows;

				if ((int)j < r(0) || (int)j >= r(2))
					src2 -= img.n_rows;
								
				arma_ext::size_type i = 0;
				for (; i < (arma_ext::size_type)r(1) ; i++)
					ptr[i] = arma_ext::saturate_cast<pixel_type>(src1[r(1)] * b1 + src2[r(1)] * b2);

				for ( ; i < (arma_ext::size_type)r(3) ; i++) {
					// bilinear interpolation
					ptr[i] = arma_ext::saturate_cast<pixel_type>(src1[i    ] * a11 +
															src1[i + 1] * a21 +
															src2[i    ] * a12 +
															src2[i + 1] * a22);
				}

				for ( ; i < out.n_rows ; i++)
					ptr[i] = arma_ext::saturate_cast<pixel_type>(src1[r(3)] * b1 + src2[r(3)] * b2);

				if ((int)j < r(2))
					src1 = src2;
			}
		}
                                      
		return out;
	}
		
	/**
	 *	@brief	Gaussian blur using 12x12 with given sigma.
	 *	@param	img		An input image.
	 *	@param	sigma	The sigma for gaussian kernel.
	 *	@return	The blurred image.
	 */
    template <typename pixel_type>
	Image<pixel_type> blur(const Image<pixel_type>& img, double sigma)
	{
		typedef typename Image<pixel_type>::size_type size_type;

		// gaussian kernel
		const size_type multiplier = 6;
		size_type kernel_size = size_type(multiplier * sigma);
		
		static mat h; // static gaussian kernel storage
        
        if (h.n_rows != kernel_size) {
            h.resize(kernel_size, kernel_size);
            double sz = (h.n_rows - 1) / -2.0;
            double denom = 2 * sigma * sigma;
            
    #ifdef _MSC_VER
            concurrency::parallel_for(size_type(0), h.n_cols, [&](size_type c) {
    #else
            for (size_type c = 0 ; c < h.n_cols ; c++) {
    #endif
				double* hptr = h.colptr(c);
                double x = sz + c;
                for (size_type r = 0 ; r < h.n_rows ; r++) {
                    double y = sz + r;
                    double arg = -(x * x + y * y) / denom;
                    hptr[r] = exp(arg);
                }
    #ifdef _MSC_VER
            });
    #else
            }
    #endif

			umat mask = h < as_scalar(max(max(h))) * std::numeric_limits<double>::epsilon();
			if (arma_ext::any(mask)) {
				uword* mptr = mask.memptr();
				double* hptr = h.memptr();
				for (uword i = 0 ; i < h.n_elem ; i++)
					if (mptr[i]) hptr[i] = 0;
			}

			h /= accu(h);
		}

		uword shift = (uword)std::ceil((kernel_size - 1) / 2.0);

		// convolution
		mat C = arma_ext::conv2(conv_to<mat>::from(img), h);
		C = C(span(shift, C.n_rows - shift), span(shift, C. n_cols - shift));

		return Image<pixel_type>(C);
	}

#ifdef USE_OPENCV
	//!	Convert Image type to the cv::Mat type.
    template <typename pixel_type>
	cv::Mat tocvMat(const Image<pixel_type>& img)
	{
		cv::Mat out(img.n_rows, img.n_cols, cv::DataType<pixel_type>::type); //!
        Image<pixel_type> t = img.t().eval();
		memcpy(out.data, t.memptr(), sizeof(pixel_type) * t.n_elem);

		//cv::imshow(name, out);
		return out;
	}

	/**
	 *	@brief	Convert BGR image or colormap to grayscale.
	 *	@param img	the truecolor BGR image to be converted into grayscale
	 *	@return	grayscale intensity image
	 *	@see	http://www.mathworks.co.kr/kr/help/images/ref/rgb2gray.html
	 */
    template <typename pixel_type>
	Image<pixel_type>	bgr2gray(const cv::Mat& img)
	{
		arma::Mat<pixel_type> gray(img.cols, img.rows);
		pixel_type* ptr = gray.memptr();
#if 0
		concurrency::parallel_for(0, img.rows, [&](int r) {
		//for (int r = 0 ; r < img.rows ; r++) {
			const uchar* src = img.ptr(r);
			uchar* dst = ptr + gray.n_rows * r;
			for (int c = 0 ; c < img.cols ; c++, src += 3)
				dst[c] = arma_ext::round<uchar>(src[2] * 0.298936021293776 + src[1] * 0.587043074451121 + src[0] * 0.114020904255103);
		//}
		});
#else
		cv::Mat bw;
		cv::cvtColor(img, bw, CV_BGR2GRAY);
		memcpy(ptr, bw.data, sizeof(pixel_type) * bw.total());
#endif

		return Image<pixel_type>(gray.t());
	}
#endif
}
