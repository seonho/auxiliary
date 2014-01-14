/**
 *	@file		image_fetcher.hpp
 *	@brief		An image fetcher class implementation
 *	@author		seonho.oh@gmail.com
 *	@date		2013-07-01
 *	@copyright	2007-2013 seonho.oh@gmail.com
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
#include <opencv2/opencv.hpp>
#endif

#ifdef USE_BOOST
#include <boost/filesystem.hpp>	// for filesystem access
using namespace boost::filesystem;
#endif

//!	An auxiliary interface functions for armadillo library.
namespace auxiliary
{
	/// supporting file formats
	static std::string supported_file_formats("*.bmp;*.dib;*.jpeg;*.jpg;*.jpe;*.jp2;*.png;*.pbm;*.pgm;*.ppm;*.sr;*.ras;*.tiff;*.tif;");

	//!	defines fetch error exception
	class fetch_error
		: public std::runtime_error
	{
	public:
		typedef std::runtime_error _Mybase;

		explicit fetch_error(const std::string& _Message, const std::string& _File, size_t _Line, const std::string& _Func)
			: _Mybase(_Message)
		{
			std::ostringstream oss;
			oss << "Fetch error at " << _Func << std::endl;
			oss << _File << "(" << _Line << "): " << _Message;
			msg_ = oss.str();
		}

		explicit fetch_error(const char *_Message, const char *_File, size_t _Line, char *_Func)
			: _Mybase(_Message)
		{
			std::ostringstream oss;
			oss << "Fetch error at " << _Func << std::endl;
			oss << _File << "(" << _Line << "): " << _Message;
			msg_ = oss.str();
		}

		~fetch_error() throw() {}

		const char* what() const throw() { return msg_.c_str(); }

	private:
		std::string msg_;
	};

#define FETCH_ERROR(msg) throw fetch_error(msg, __FILE__, __LINE__, __FUNCTION__)

    inline char PathSeparator()
    {
#if defined(_WIN32) || defined(_WIN64)
        return '\\';
#else
        return '/';
#endif
    }
	//!	An implementation of image fetcher.
	class image_fetcher
	{
	public:
		//!	Open file or directory
		void open(std::string path)
		{
#if defined(USE_BOOST) && defined(USE_OPENCV)
			boost::filesystem::path p(path);
			if (boost::filesystem::is_directory(p)) {
				dir_ = path;
#if __cplusplus >= 201103L || defined(_MSC_VER)
				std::for_each(boost::filesystem::directory_iterator(p), 
					boost::filesystem::directory_iterator(), 
					[&](boost::filesystem::directory_entry& entry) {
#else
				boost::filesystem::directory_iterator end;
				for (boost::filesystem::directory_iterator iter(p) ; iter != end ; ++iter) {
					boost::filesystem::directory_entry& entry = *iter;
#endif
					if (boost::filesystem::is_regular_file(entry.status()) && 
						supported_file_formats.find(entry.path().extension().string()) != std::string::npos) // match the extension
						files_.push_back(entry.path().string());
#if __cplusplus >= 201103L || defined(_MSC_VER)
				});
#else
				}
#endif

				if (files_.empty())
					FETCH_ERROR("Nothing to fetch");

				pos_ = 0;
			} else if (boost::filesystem::is_regular_file(p)) {
				cap_.open(path);
                dir_ = path.substr(0, path.find_last_of(PathSeparator())); // video directory
            } else
				FETCH_ERROR("Given path does not exist!");
#elif defined(USE_OPENCV)
            cap_.open(path);
            
            if (cap_.isOpened())
                dir_ = path.substr(0, path.find_last_of(PathSeparator())); // video directory
            else
				FETCH_ERROR("Given path does not exist!");
#else
            return open_pack(path);
#endif
		}
        
        void open_pack(std::string path)
        {
            fin_.open(path, std::ios::binary);
            
            if (fin_.is_open()) {
                // try to read pack file
                cout << "Read pack file" << endl;
                fin_.read((char *)&width_, sizeof(unsigned int));
                fin_.read((char *)&height_, sizeof(unsigned int));
                fin_.read((char *)&numframes_, sizeof(unsigned int));
                dir_ = path.substr(0, path.find_last_of(PathSeparator()));
                pos_ = 0;
			} else
				FETCH_ERROR("Given path does not exist!");
        }

		//!	Connect to device
		void open(int device_id)
		{
#ifdef USE_OPENCV
			if (!cap_.open(device_id))
				FETCH_ERROR("Cannot connect camera");
            
#ifdef USE_BOOST
			boost::filesystem::path full_path( boost::filesystem::current_path() );
			dir_ = full_path.string();
#else
            
#endif
            
#endif
		}

		//!	Grabs the next frame from video file or directory.
		bool grab()
		{
#ifdef USE_OPENCV
			if (cap_.isOpened()) return cap_.grab();
#endif
			return files_.empty() ? pos_ < numframes_ : (pos_ < files_.size());
		}

		//!	Decodes and returns the grabbed video frame or image.
        template <typename pixel_type>
		void retrieve(Image<pixel_type>& image)
		{
#ifdef USE_OPENCV
			cv::Mat frame;
            
			if (cap_.isOpened()) {
				cap_.retrieve(frame);
                image = bgr2gray<pixel_type>(frame);
			} else if(!files_.empty()) {
				//std::cout << files[pos].c_str() << " "; /*std::endl;*/
				frame = cv::imread(files_[pos_++]);

#ifdef USE_16BIT_IMAGE
                // simulate different image format
                std::cout << "Simulate 16 bit image" << std::endl;
                frame.clone().convertTo(frame, CV_16U);
#endif
                image = bgr2gray<pixel_type>(frame);
			} else if (fin_.is_open()) {
#else
            if (fin_.is_open()) {
#endif
                Image<pixel_type> temp(height_, width_);
                fin_.read((char *)temp.memptr(), sizeof(pixel_type) * width_ * height_);
                image = Image<pixel_type>(temp.t());
                ++pos_;
            }
		}

		//!	Get current directory
		inline std::string current_directory() const
		{
			return dir_;
		}

	private:
#ifdef USE_OPENCV
		cv::VideoCapture cap_;              ///< video capture
#endif
        std::ifstream               fin_;   ///<
        unsigned int                width_, height_;
        unsigned int                numframes_;
        
		std::string                 dir_;   ///< the current directory
		std::vector<std::string>    files_;	///< the image file names
		size_t                      pos_;	///< the current frame number
	};
}