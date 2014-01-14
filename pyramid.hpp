/**
 *	@file		pyramid.hpp
 *	@brief		Functions for gaussian image pyramid
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

#include <armadillo>

namespace auxiliary
{
	/// Various border types, image boundaries are denoted with '|'
#if __cplusplus >= 201103L || defined(_MSC_VER)
	enum border_type : arma::uword {
#else
	enum border_type {
#endif
		constant,	///< iii | abcde | iii with some specified 'i'
		reflect,	///< cba | abcde | edc
		replicate,	///< aaa | abcde | eee
		warp,		///< cde | abcde | abc
		reflect101,	///< dcb | abcde | dcb
		transparent	///< not specified
	};

	/**
	 *	@brief	Computes the location of an extrapolated pixel.
	 *	@param p	0-based coordinate of the extrapolated pixel along one of the axes.
	 *	@param n	Length of the array along the corresponding axis.
	 *	@param type	Border type, one of the ::border_type.
	 */
	inline arma::uword borderInterpolate(int p, int n, border_type type = reflect101)
	{
		//return (p < 0) ? -p : ((p < n) ? p : (n + n - p - 1));
		if ((unsigned)p < (unsigned)n) return p;

		switch (type) {
		case reflect:
		case reflect101:
			{
				int delta = (type == reflect101);

				if (n == 1) p = 0;

				do {
					if (p < 0)
						p = -p - 1 + delta;
					else
						p = n - 1 - (p - n) - delta;
				} while ((unsigned)p >= (unsigned)n);
			}
			break;
		case replicate:
			p = (p < 0) ? 0 : n - 1;
			break;
		case warp:
			if (p < 0)
				p -= ((p - n + 1) / n) * n;
			if (p > n)
				p %= n;
			break;
		case constant:
			p = -1;
                break;
		default:
			break;
		};

		return p;
	}

#define castOp(x) ((x + 128) >> 8)

	/**
	 *	@brief	Blurs an image and downsamples it.<br>
	 *			This function performs the downsampling step of the Gaussian pyramid construction.<br> 
	 *			First, it convolves the source image with the kernel:
	 *			\f[
	 *				\frac{1}{256}
	 *				\begin{bmatrix}
	 *					1 &  4 &  6 &  4 & 1 \\
	 *					4 & 16 & 24 & 16 & 4 \\
	 *					6 & 24 & 36 & 24 & 6 \\
	 *					4 & 16 & 24 & 16 & 4 \\
	 *					1 &  4 &  6 &  4 & 1
	 *				\end{bmatrix}
	 *			\f]
	 *			Then, it downsamples the image by rejecting even rows and columns.
	 *	@param in
	 *	@param out
	 *	@note	This function is preliminary; it is not yet fully optimized.
	 *	@see	PyrDownVec_32s8u in pyramid.cpp of OpenCV
	 */
	template <typename T1, typename T2>
	void pyrDown(const T1& in, T2& out)
	{
		const uword KERNEL_SIZE = 5;

		//uword width = std::min((src.n_cols - SZ / 2 - 1) / 2;
		
		circular_buffer<arma::ivec> cols(KERNEL_SIZE);

		for (arma::uword i = 0 ; i < KERNEL_SIZE ; i++)
			cols.push_back(zeros<ivec>(out.n_rows));

		int sx0 = -(int)KERNEL_SIZE / 2, sx = sx0;

		arma::umat tab(KERNEL_SIZE + 2, 2);
		uword* lptr = tab.colptr(0),
			 * rptr = tab.colptr(1);
		for (uword y = 0 ; y <= KERNEL_SIZE + 1 ; y++) {
			lptr[y] = borderInterpolate((int)y + sx0, (int)in.n_rows);
			rptr[y] = borderInterpolate((int)(y + (out.n_rows - 1) * 2) + sx0, (int)in.n_rows);
		}

		// gaussian convolution with 
		for (arma::uword x = 0 ; x < out.n_cols ; x++) {
			typename T2::elem_type* dst = out.colptr(x);

			// vertical convolution and decimation
			for ( ; sx <= (int)x * 2 + 2 ; sx++) {
				ivec& col = cols.next();
				int* colptr = col.memptr();

				// interpolate border
				const typename T2::elem_type* src = in.colptr(borderInterpolate(sx, (int)in.n_cols));

				colptr[0] = src[lptr[2]] * 6 + (src[lptr[1]] + src[lptr[3]]) * 4 + (src[lptr[0]] + src[lptr[4]]);

				for (arma::uword y = 1 ; y < out.n_rows - 1; y++)
				//concurrency::parallel_for(uword(1), out.n_rows - 1, [&](uword y) {
					colptr[y] = src[y * 2] * 6 + 
							 (src[y * 2 - 1] + src[y * 2 + 1]) * 4 + 
							 (src[y * 2 - 2] + src[y * 2 + 2]);
				//});

				colptr[out.n_rows - 1] = src[rptr[2]] * 6 + 
									  (src[rptr[1]] + src[rptr[3]]) * 4 + 
									  (src[rptr[0]] + src[rptr[4]]);
			}

			const int* col0 = cols[0].memptr();
			const int* col1 = cols[1].memptr();
			const int* col2 = cols[2].memptr();
			const int* col3 = cols[3].memptr();
			const int* col4 = cols[4].memptr();

			// horizontal convolution and decimation
#if ENABLE_SSE2
			//__m128i d = _mm_set1_epi16(128);
			//uword y = 0;
			//for ( ; y <= out.n_rows - 16 ; y += 16) {
			//	__m128i c0, c1, c2, c3, c4, t0, t1;
			//	c0 = _mm_packs_epi32(_mm_load_si128((const __m128i*)(col0 + y)),
			//						 _mm_load_si128((const __m128i*)(col0 + y + 4)));
			//	c1 = _mm_packs_epi32(_mm_load_si128((const __m128i*)(col1 + y)),
			//						 _mm_load_si128((const __m128i*)(col1 + y + 4)));
			//	c2 = _mm_packs_epi32(_mm_load_si128((const __m128i*)(col2 + y)),
			//						 _mm_load_si128((const __m128i*)(col2 + y + 4)));
			//	c3 = _mm_packs_epi32(_mm_load_si128((const __m128i*)(col3 + y)),
			//						 _mm_load_si128((const __m128i*)(col3 + y + 4)));
			//	c4 = _mm_packs_epi32(_mm_load_si128((const __m128i*)(col4 + y)),
			//						 _mm_load_si128((const __m128i*)(col4 + y + 4)));

			//	c0 = _mm_add_epi16(r0, r4);
			//	c1 = _mm_add_epi16(_mm_add_epi16(c1, c3), c2);
			//}
#else
			for (arma::uword y = 0 ; y < out.n_rows ; y++)
			//concurrency::parallel_for(uword(0), out.n_rows, [&](uword y) {
				dst[y] = castOp(col2[y] * 6 + (col1[y] + col3[y]) * 4 + col0[y] + col4[y]);
			//});
#endif
		}
	}
}
