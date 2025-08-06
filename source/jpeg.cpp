// ---------------------------------------------------------------------------------------------------------------------------------
//       _                                       
//      | |                                      
//      | |_ __   ___  __ _      ___ _ __  _ __  
//  _   | | '_ \ / _ \/ _` |    / __| '_ \| '_ \ 
// | |__| | |_) |  __/ (_| | _ | (__| |_) | |_) |
//  \____/| .__/ \___|\__, |(_) \___| .__/| .__/ 
//        | |          __/ |        | |   | |    
//        |_|         |___/         |_|   |_|    
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

#include "texturebin.h"
#include "jpeg.h"

// ---------------------------------------------------------------------------------------------------------------------------------

	Jpeg::Jpeg()
	: _width(0), _height(0), _stride(0), _buffer(NULL)
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

	Jpeg::Jpeg(const unsigned int w, const unsigned int h)
	: _width(w), _height(h), _stride(w*3), _buffer(NULL)
{
	buffer() = new unsigned char[height() * stride()];
	memset(buffer(), 0, height() * stride());
}

// ---------------------------------------------------------------------------------------------------------------------------------

	Jpeg::Jpeg(const std::string & filename)
	: _width(0), _height(0), _stride(0), _buffer(NULL)
{
	reset();
	read(filename);
}

// ---------------------------------------------------------------------------------------------------------------------------------

	Jpeg::~Jpeg()
{
	reset();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	Jpeg::reset()
{
	name().erase();
	width() = 0;
	height() = 0;
	delete[] buffer();
	buffer() = NULL;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	Jpeg::read(const std::string & filename)
{
	reset();
	FILE *	fp = NULL;
	
	try
	{
		// Grab the filename

		name() = filename;

		// Open the file

		fp = fopen(name().c_str(), "rb");
		if (!fp) throw std::string("Unable to open the input file: ").append(name());

		// Step 1: create a decompression object

		struct jpeg_decompress_struct decomp;
		{
			// Setup the error handler in the decompression object

			jpeg_error_mgr jerr;
			decomp.err = jpeg_std_error(&jerr);
			jerr.error_exit = errorHandler;

			// Create the compressor object

			jpeg_create_decompress(&decomp);
		}

		// Step 2: specify data source to our file pointer

		jpeg_stdio_src(&decomp, fp);

		// Step 3: read file parameters with jpeg_read_header()

		jpeg_read_header(&decomp, TRUE);

		// Step 4: start decompressor

		jpeg_start_decompress(&decomp);
		{

			// Make sure it's input we can deal with

			if (decomp.output_components != 3) throw std::string("JPEG files must be 24-bit RGB only (").append(name()).append(")");

			// Grab the width/height/stride

			width() = decomp.output_width;
			height() = decomp.output_height;
			stride() = width() * decomp.output_components;

			// Allocate our buffer

			buffer() = new unsigned char[height() * stride()];
		}

		// Step 5: read in the successive scanlines
		{
			JSAMPARRAY	scanlineBuffer = (*decomp.mem->alloc_sarray)((j_common_ptr) &decomp, JPOOL_IMAGE, stride(), 1);
			while (decomp.output_scanline < height())
			{
				jpeg_read_scanlines(&decomp, scanlineBuffer, 1);
				memcpy(buffer() + stride() * (decomp.output_scanline - 1), scanlineBuffer[0], stride());
			}
		}

		// Step 6: Finish decompression

		jpeg_finish_decompress(&decomp);

		// Step 7: Release JPEG decompression object

		jpeg_destroy_decompress(&decomp);

		// Done with this

		fclose(fp);
		fp = NULL;
	}
	catch(...)
	{
		// Cleanup

		if (fp) fclose(fp);
		reset();

		// Rethrow

		throw;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	Jpeg::write(const std::string & filename, const unsigned int quality)
{
	// Validate inputs

	if (!buffer() || !width() || !height() || !stride())
	{
		throw std::string("Cannot write the file (").append(filename).append(") because we don't have a file loaded!");
	}

	FILE *	fp = NULL;
	
	try
	{
		// The output file

		fp = fopen(filename.c_str(), "wb");
                if (!fp) throw std::string("Unable to create the output file: ").append(filename);

		// Step 1: create the compression object

		struct jpeg_compress_struct comp;
		{
			// Setup the error handler in the decompression object

			jpeg_error_mgr jerr;
			comp.err = jpeg_std_error(&jerr);
			jerr.error_exit = errorHandler;

			// Create the compressor object

			jpeg_create_compress(&comp);
		}

		// Step 2: specify the destination file

		jpeg_stdio_dest(&comp, fp);

		// Step 3: set parameters for compression
		{
			comp.image_width = width();
			comp.image_height = height();
			comp.input_components = 3;
			comp.in_color_space = JCS_RGB;
			jpeg_set_defaults(&comp);

			// Set the quality

			jpeg_set_quality(&comp, quality, TRUE);
		}

		// Step 4: Start compressor

		jpeg_start_compress(&comp, TRUE);

		// Step 5: write out the successive scanlines
		{
			JSAMPROW row_pointer[1];
			while (comp.next_scanline < height())
			{
				row_pointer[0] = &buffer()[comp.next_scanline * stride()];
				jpeg_write_scanlines(&comp, row_pointer, 1);
			}
		}

		// Step 6: Finish compression

		jpeg_finish_compress(&comp);

		// Done with this...

		fclose(fp);
		fp = NULL;

		// Step 7: release JPEG compression object

		jpeg_destroy_compress(&comp);
	}
	catch(...)
	{
		// Cleanup

		if (fp) fclose(fp);

		// Rethrow

		throw;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	Jpeg::errorHandler(j_common_ptr ptr)
{
	char msg[4096];
	(*ptr->err->format_message)(ptr, msg);
	throw std::string(msg);
}

// ---------------------------------------------------------------------------------------------------------------------------------
// Jpeg.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
