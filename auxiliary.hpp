/**
 *	@file		auxiliary.hpp
 *	@brief		An implemenation of auxiliary namespace
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

#include "arma_ext.hpp"

/**
 *	@brief	An auxiliary classes and functions.
 *	@note	Implements several interface functions for OpenCV
 */
namespace auxiliary
{
	using namespace arma;

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
}

#include "circular_buffer.hpp"
#include "imgproc_aux.hpp"
#include "image_fetcher.hpp"
#include "integral.hpp"
#include "pyramid.hpp"