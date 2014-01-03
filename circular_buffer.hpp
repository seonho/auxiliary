/**
 *	@file		circular_buffer.hpp
 *	@brief		A minimal implementation of circular buffer class
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
	//!	A minimal implementation of circular buffer class.
	template <typename T>
	class circular_buffer {
		typedef T			value_type;
		typedef T&			reference;
		typedef const T&	const_reference;
		typedef size_t		size_type;
	public:
		explicit circular_buffer(const size_type n): buffer_(n), size_(n), contents_size_(0), head_(0), tail_(0) {}

		///	Get next element in the circular buffer
		reference next()
		{
			assert(contents_size_ == size_);
			inc_head();
			inc_tail();
			return buffer_[tail_];
		}

		/// Add new last element.
		void push_back(const value_type& item)
		{
			if (contents_size_ == 0) {
				buffer_[tail_] = item;
				contents_size_++;
			} else {
				inc_tail();
				buffer_[tail_] = item;

				if (contents_size_ == size_)
					inc_head();
				else
					contents_size_++;
			}

			buffer_[tail_] = item;
		}

		///	Accesses the first element.
		inline reference front() { return buffer_[head_]; }

		///	Accesses the first element.
		inline const_reference front() const { return buffer_[head_]; }

		///	Accesses the last element
		inline reference back() { return buffer_[tail_]; }

		///	Access the last element
		inline const_reference back() const { return buffer_[tail_]; }

		///	Accesses the element
		inline const_reference operator[] (size_type i) const
		{
			assert(i < size_);
			return buffer_[(head_ + i) % size_];
		}

		///	Accesses the element
		inline reference operator[] (size_type i)
		{
			assert(i < size_);
			return buffer_[(head_ + i) % size_];
		}

		///	Removes all elements.
		void clear()
		{
			head_ = tail_ = contents_size_ = 0;
			buffer_.clear();
		}

		/// Counts the number of elements.
		inline size_type size() const { return size_; }

	private:
		void inc_head()
		{
			++head_;
			if (head_ == size_) head_ = 0;
		}

		void inc_tail()
		{
			++tail_;
			if (tail_ == size_) tail_ = 0;
		}

	private:
		std::vector<T>	buffer_;		///< the container
		size_type		size_;			///< the size of buffer
		size_type		contents_size_;	///< the contents size
		size_type		head_;			///< position of the first element
		size_type		tail_;			///< position of the last element
	};
}