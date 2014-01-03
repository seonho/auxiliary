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

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>	// for filesystem access
using namespace boost::filesystem;

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

	//!	An implementation of image fetcher.
	class image_fetcher
	{
	public:
		//!	Open file or directory
		void open(std::string path)
		{
			boost::filesystem::path p(path);
			if (boost::filesystem::is_directory(p)) {
				dir = path;
				std::for_each(boost::filesystem::directory_iterator(p), 
					boost::filesystem::directory_iterator(), 
					[&](boost::filesystem::directory_entry& entry) {
					if (boost::filesystem::is_regular_file(entry.status()) && 
						supported_file_formats.find(entry.path().extension().string()) != std::string::npos) // match the extension
						files.push_back(entry.path().string());
				});

				if (files.empty())
					FETCH_ERROR("Nothing to fetch");

				pos = 0;
			} else if (boost::filesystem::is_regular_file(p)) {
				dir = p.branch_path().string();
				cap.open(path);
			} else
				FETCH_ERROR("Given path does not exist!");
		}

		//!	Connect to device
		void open(int device_id)
		{
			if (!cap.open(device_id))
				FETCH_ERROR("Cannot connect camera");

			boost::filesystem::path full_path( boost::filesystem::current_path() );
			dir = full_path.string();			
		}

		//!	Grabs the next frame from video file or directory.
		bool grab()
		{
			if (cap.isOpened()) return cap.grab();

			return (pos < files.size());
		}

		//!	Decodes and returns the grabbed video frame or image.
		void retrieve(Image& image)
		{
			cv::Mat frame;
			if (cap.isOpened()) {
				cap.retrieve(frame);
				//std::cout << "frame " << pos++ << " "; /*std::endl;*/
			} else {
				//std::cout << files[pos].c_str() << " "; /*std::endl;*/
				frame = cv::imread(files[pos++]);
			}

			image = bgr2gray(frame);
		}

		//!	Get current directory
		inline std::string current_directory() const
		{
			return dir;
		}

	private:
		cv::VideoCapture cap;			///< video capture

		std::string dir;				///< the current directory
		std::vector<std::string> files;	///< the image file names
		size_t pos;						///< the current frame number
	};
}