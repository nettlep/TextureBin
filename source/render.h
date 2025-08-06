// ---------------------------------------------------------------------------------------------------------------------------------
//  _____                 _               _     
// |  __ \               | |             | |    
// | |__) | ___ _ __   __| | ___ _ __    | |__  
// |  _  / / _ \ '_ \ / _` |/ _ \ '__|   | '_ \ 
// | | \ \|  __/ | | | (_| |  __/ |    _ | | | |
// |_|  \_\\___|_| |_|\__,_|\___|_|   (_)|_| |_|
//                                              
//                                              
//
// Description:
//
//   Renders a textured image. Yay.
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

#ifndef	_H_RENDER
#define _H_RENDER

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

#include "primitive.h"
#include "tmap.h"

class	Jpeg;

// ---------------------------------------------------------------------------------------------------------------------------------

class Camera
{
public:

	inline	Matrix4	calcTransform()
	{
		float	aspect = static_cast<float>(width) / static_cast<float>(height);
		Matrix4	rotation = Matrix4::genLookat(direction, bank);
		Matrix4	translation = Matrix4::genTranslation(-Point4(position.x(), position.y(), position.z(), 0));
		Matrix4	view = translation >> rotation;
		Matrix4	perspectiveXform = Matrix4::genProjectPerspectiveBlinn(fov, aspect, 1, 5000);
		return view >> perspectiveXform;
	}

	Point4		position;
	Vector3		direction;
	float		bank;
	float		fov;
	unsigned int	width;
	unsigned int	height;
	unsigned int	oversampleX;
	unsigned int	oversampleY;
};

// ---------------------------------------------------------------------------------------------------------------------------------

class	ShadowMap
{
public:
	Camera		camera;
	Matrix4		xform;
	float *		zBuffer;
};

// ---------------------------------------------------------------------------------------------------------------------------------

class	Render
{
public:
	// Construction/Destruction

				Render();
virtual				~Render();

	// Operators

	// Render a scene
	//
	// Loads tht texture from 'filename' and the 3DS file from sceneFilename.

virtual		void		renderScene(const std::string & textureFilename, const std::string & imageFilename, const std::string & sceneFilename, const unsigned int width, const unsigned int height, const unsigned int oversampleX, const unsigned int oversampleY, const unsigned int quality, const sPHONG & phong);

	// Imports a scene
	//
	// The filename refers to a 3ds file. The scene is loaded and a set of primitives containing all of the geometry is generated.
	// Also, if a camera exists in the 3DS file, it's information is stored in the last four parameters. If no camera exits, a
	// default camera is generated from hard-coded constants in this routine.

static		void		importScene(const std::string & filename, std::vector<primitive<> > & primitives, std::vector<sLIGHT> & lights, Point4 & cameraPosition, Vector3 & cameraDirection, float & cameraBank, float & cameraFOV);

	// Prepares for rendering -- transforms, clips and projects polygons and returns a simple list of vertices for rendering

static		sVERT *		transformAndClip(const Camera & camera, const Matrix4 & xform, const Jpeg & texture, std::vector<primitive<> > & primitives, unsigned int & renderPolygonCount);

	// Draws stuff to the frame buffer

static		void		renderGeometry(Jpeg & image, const Camera & camera, const sPHONG & phong, const sVERT * renderVertices, const unsigned int renderPolygonCount, const std::vector<sLIGHT> & lights, const std::vector<ShadowMap> & shadowMaps, const Jpeg & texture);

	// Draws stuff to the z-buffer only for use in shadow mapping

static		void		renderShadowMap(ShadowMap & map, const Camera & camera, sVERT * renderVertices, const unsigned int renderPolygonCount);

	// Simply converts an image from 32-bits to 24-bits

static		void		convert32To24(unsigned char * dest, const unsigned int * source, const unsigned int width, const unsigned int height);

	// Simply converts an image from 24-bits to 32-bits

static		void		convert24To32(unsigned int * dest, const unsigned char * source, const unsigned int width, const unsigned int height);

	// Accumulate a buffer
	//
	// Given a standard 32-bit frame buffer, add it to the accum buffer -- a buffer containing pixels where each color component
	// is stored as a dword.

static		void		accumulateBuffer(unsigned int * accumBuffer, const unsigned int * frameBuffer, const unsigned int width, const unsigned int height);

	// Downsample an image
	//
	// The input buffer contains an oversampled image, where the total of all images are added together for each pixel.
	// Each pixel is stored as three 32-bit values (one dword per component.) All we need to do, is divide by the total
	// renders (oversampleX * oversampleY) and convert it to a standard 32-bit image.

static		void		downsample(unsigned int * buffer, const unsigned int width, const unsigned int height, const unsigned int oversampleX, const unsigned int oversampleY);
};

#endif // _H_RENDER
// ---------------------------------------------------------------------------------------------------------------------------------
// Render.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
