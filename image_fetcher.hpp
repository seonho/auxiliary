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

namespace auxiliary
{
	/// supporting file formats
	static std::string supported_file_formats("*.bmp;*.dib;*.jpeg;*.jpg;*.jpe;*.jp2;*.png;*.pbm;*.pgm;*.ppm;*.sr;*.ras;*.tiff;*.tif;");

	/**
	 *	@brief	An implementation of image fetcher.
	 */
	class image_fetcher
	{
	public:
		image_fetcher(std::string path)
		{
			boost::filesystem::path p(path);
			if (boost::filesystem::is_directory(p)) {
				dir = path;
				std::for_each(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), [&](boost::filesystem::directory_entry& entry) {
					if (boost::filesystem::is_regular_file(entry.status()) && 
						supported_file_formats.find(entry.path().extension().string()) != std::string::npos) // match the extension
						files.push_back(entry.path().string());
				});

				pos = 0;
			} else if (boost::filesystem::is_regular_file(p)) {
				dir = p.branch_path().string();
				cap.open(path);
			} else {
				std::cerr << path << " is not exist!" << std::endl;
			}
		}

		bool grab()
		{
			if (cap.isOpened()) return cap.grab();

			return (pos < files.size());
		}

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

		inline std::string current_directory() const
		{
			return dir;
		}

	private:
		cv::VideoCapture cap;			///< video capture

		std::string dir;				///< image directory
		std::vector<std::string> files;	///< image file names
		size_t pos;						///< the frame no.
	};
}