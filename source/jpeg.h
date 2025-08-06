// ---------------------------------------------------------------------------------------------------------------------------------
//       _                      _     
//      | |                    | |    
//      | |_ __   ___  __ _    | |__  
//  _   | | '_ \ / _ \/ _` |   | '_ \ 
// | |__| | |_) |  __/ (_| | _ | | | |
//  \____/| .__/ \___|\__, |(_)|_| |_|
//        | |          __/ |          
//        |_|         |___/           
//
// Description:
//
//   JPEG file management (uses libjpeg)
//
// Notes:
//
//   Best viewed with 8-character tabs and (at least) 132 columns
//
// History:
//
//   01/26/2003 by Paul Nettle: Original creation
//
// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep
//
// Copyright 2003, Fluid Studios, all rights reserved.
// ---------------------------------------------------------------------------------------------------------------------------------

#ifndef	_H_JPEG
#define _H_JPEG

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

extern "C"
{
#ifdef _MSC_VER
#include "jpeg/jpeglib.h"
#else
#include "jpeglib.h"
#endif
}

// ---------------------------------------------------------------------------------------------------------------------------------

class	Jpeg
{
public:
	// Construction/Destruction

					Jpeg();
					Jpeg(const unsigned int w, const unsigned int h);
					Jpeg(const std::string & filename);
virtual					~Jpeg();

	// Implementation

virtual		void			reset();
virtual		void			read(const std::string & filename);
virtual		void			write(const std::string & filename, const unsigned int quality = 80);

	// Statics

static		void			errorHandler(j_common_ptr cinfo);


	// Accessors

inline		std::string &		name()		{return _name;}
inline	const	std::string		name() const	{return _name;}
inline		unsigned int &		width()		{return _width;}
inline	const	unsigned int		width() const	{return _width;}
inline		unsigned int &		height()	{return _height;}
inline	const	unsigned int		height() const	{return _height;}
inline		unsigned int &		stride()	{return _stride;}
inline	const	unsigned int		stride() const	{return _stride;}
inline		unsigned char *&	buffer()	{return _buffer;}
inline	const	unsigned char *		buffer() const	{return _buffer;}

private:
	// Explicitly disallow copying this object

					Jpeg(const Jpeg & rhs) {}
inline		Jpeg &			operator=(const Jpeg & rhs) {Jpeg * errptr = 0; return *errptr;}

	// Data members

		std::string		_name;
		unsigned int		_width;
		unsigned int		_height;
		unsigned int		_stride;
		unsigned char *		_buffer;
};

#endif // _H_JPEG
// ---------------------------------------------------------------------------------------------------------------------------------
// Jpeg.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
