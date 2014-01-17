/**
 *	@file		integral.hpp
 *	@brief		Implementation of integral image
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

namespace auxiliary
{
	/**
	 *	@brief	Compute integral images
	 *	@param [in] img		input image
	 *	@param [out] sum	integral image
	 *	@param [out] sqsum	squared integral image
	*/
	template <typename T1, typename T2, typename T3>
	void integral(const Image<T1>& img, Image<T2>& sum, Image<T3>& sqsum)
	{
		typedef typename Image<T1>::size_type	size_type;

		// allocate images
		sum.resize(img.width(), img.height());
		sqsum.resize(img.width(), img.height());

		// simplest implementation
		const T1* ptr = img.memptr();
		T2* sumptr = sum.memptr();
		T3* sqsumptr = sqsum.memptr();

		T2 s = 0;
		T3 sq = 0;

		// image is column major,
		// 0-th column accumulated
		for (size_type y = 0 ; y < img.height() ; y++) {
			const T1 it = ptr[y];
			s += it;
			sq += (T3)it * it;
			sumptr[y] = s;
			sqsumptr[y] = sq;
		}

		s = sumptr[0];
		sq = sqsumptr[0];
		// 0-th row accumulated
		for (size_type x = 1 ; x < img.width() ; x++) {
			T2 it = img(0, x);
			s += it;
			sq += (T3)it * it;
			sum.at(0, x) = s;
			sqsum.at(0, x) = sq;
		}

		T2* sptr0 = sum.colptr(0);
		T3* sqptr0 = sqsum.colptr(0);

		for (size_type x = 1 ; x < img.width() ; x++) {
			T2 s = 0;
			T3 sq = 0;
			
			T2* sptr1 = sum.colptr(x);
			T3* sqptr1 = sqsum.colptr(x);

			for (size_type y = 0 ; y < img.height() ; y++) {
				T2 it = img(y, x);
				s += it;
				sq += (T3)it * it;
				//sum.at(y, x) = sum.at(y, x - 1) + s;
				sptr1[y] = sptr0[y] + s;
				//sqsum.at(y, x) = sqsum.at(y, x - 1) + sq;
				sqptr1[y] = sqptr0[y] + sq;
			}

			sptr0 = sptr1;
			sqptr0 = sqptr1;
		}
	}
}