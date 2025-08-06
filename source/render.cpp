// ---------------------------------------------------------------------------------------------------------------------------------
//  _____                 _                                
// |  __ \               | |                               
// | |__) | ___ _ __   __| | ___ _ __      ___ _ __  _ __  
// |  _  / / _ \ '_ \ / _` |/ _ \ '__|    / __| '_ \| '_ \ 
// | | \ \|  __/ | | | (_| |  __/ |    _ | (__| |_) | |_) |
// |_|  \_\\___|_| |_|\__,_|\___|_|   (_) \___| .__/| .__/ 
//                                            | |   | |    
//                                            |_|   |_|    
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

#include "texturebin.h"
#include "render.h"
#include "jpeg.h"
#include "3ds.h"
#include "clip.h"
#include "tmap.h"

// ---------------------------------------------------------------------------------------------------------------------------------

	Render::Render()
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

	Render::~Render()
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	Render::renderScene(const std::string & textureFilename, const std::string & imageFilename, const std::string & sceneFilename, const unsigned int width, const unsigned int height, const unsigned int oversampleX, const unsigned int oversampleY, const unsigned int quality, const sPHONG & phong)
{
	// Populate these into the camera

	Camera	camera;
	camera.width = width;
	camera.height = height;
	camera.oversampleX = oversampleX;
	camera.oversampleY = oversampleY;

	// Find the short name of the file

	std::string	shortName = textureFilename;
	std::string::size_type	idx = shortName.find_last_of("\\/");
	if (idx != std::string::npos) shortName.erase(0, idx+1);
	printf("%s: ", shortName.c_str());

	Jpeg	texture;
	printf("read...");
	{
		texture.read(textureFilename);
	}

	std::vector<primitive<> >	primitives;
	std::vector<sLIGHT>		lights;
	printf("3D import...");
	{
		importScene(sceneFilename, primitives, lights, camera.position, camera.direction, camera.bank, camera.fov);
	}

	std::vector<ShadowMap>	shadowMaps;
	printf("shadows...");
	{
		// Render the shadow maps

		for (unsigned int i = 0; i < lights.size(); ++i)
		{
			sLIGHT &	light = lights[i];

			// Treat the light like a camera

			ShadowMap	map;
			map.camera.position = light.pos;
			map.camera.direction = light.dir;
			map.camera.bank = 0;
			map.camera.fov = 0.52f;
			map.camera.height = phong.shadowMapRes;
			map.camera.width = phong.shadowMapRes;
			map.camera.oversampleX = 1;
			map.camera.oversampleY = 1;
			map.xform = map.camera.calcTransform();

			// Transform and clip the polygons

			unsigned int	renderPolygonCount;
			sVERT *		renderVertices = transformAndClip(map.camera, map.xform, texture, primitives, renderPolygonCount);

			// Render the polygons

			map.zBuffer = new float[map.camera.width * map.camera.height];
			memset(map.zBuffer, 0, map.camera.width * map.camera.height * sizeof(float));
			renderShadowMap(map, map.camera, renderVertices, renderPolygonCount);

#if 0
Jpeg	foo(map.camera.width,map.camera.height);
float	zmin, zmax;
for (unsigned int j = 0; j < map.camera.width*map.camera.height; ++j)
{
	if (!j || map.zBuffer[j] < zmin) zmin = map.zBuffer[j];
	if (!j || map.zBuffer[j] > zmax) zmax = map.zBuffer[j];
}
for (unsigned int j = 0; j < map.camera.width*map.camera.height; ++j)
{
	float	zzz = (map.zBuffer[j] - zmin) / (zmax-zmin) * 255;
	int	iz = (int) zzz;
	foo.buffer()[j*3+0] = iz;
	foo.buffer()[j*3+1] = iz;
	foo.buffer()[j*3+2] = iz;
}
char	dsp[90];
sprintf(dsp, "j:\\%d_foo.jpg", i);
foo.write(dsp);
#endif

			// Add this shadow map

			shadowMaps.push_back(map);

			// Done with this

			delete[] renderVertices;
		}
	}

	Jpeg	image(width, height);
	printf("render...");
	{
		Matrix4	xform = camera.calcTransform();

		// Transform and clip the polygons

		unsigned int	renderPolygonCount;
		sVERT *		renderVertices = transformAndClip(camera, xform, texture, primitives, renderPolygonCount);

		// Render the polygons

		renderGeometry(image, camera, phong, renderVertices, renderPolygonCount, lights, shadowMaps, texture);

		// Done with this

		delete[] renderVertices;

		// Cleanup the shadow maps

		for (unsigned int i = 0; i < lights.size(); ++i)
		{
			delete[] shadowMaps[i].zBuffer;
		}
		shadowMaps.clear();
	}

	printf("write...");
	{
		image.write(imageFilename);
	}

	printf("done.\n");
	{
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	Render::importScene(const std::string & filename, std::vector<primitive<> > & primitives, std::vector<sLIGHT> & lights, Point4 & cameraPosition, Vector3 & cameraDirection, float & cameraBank, float & cameraFOV)
{
	// Load the 3DS scene file

	C3DS	loader;
	sD3DS	info;
	memset(&info, 0, sizeof(info));
	loader.load(filename.c_str(), &info);

	// Convert the primitives

	for (unsigned int i = 0; i < info.meshCount; i++)
	{
		// Our current mesh

		sMSH	&mesh = info.mesh[i];

		// Weld vertices... it seems that max doesn't always do this for some objects
		{
			sP3D *	verts = mesh.vList;
			sFACE *	faces = mesh.fList;
			for (unsigned int j = 0; j < mesh.vCount; ++j)
			{
				sP3D &	jVert = verts[j];
				for (unsigned int k = j+1; k < mesh.vCount; ++k)
				{
					if (jVert.x == verts[k].x && jVert.y == verts[k].y && jVert.z == verts[k].z)
					{
						// 'k' is a duplicate. Locate all faces using 'k' and switch them over to use 'j'

						for (unsigned int l = 0; l < mesh.fCount; ++l)
						{
							sFACE &	face = faces[l];
							if (face.a == k) face.a = j;
							if (face.b == k) face.b = j;
							if (face.c == k) face.c = j;
						}

						// Wipe out 'k' so we don't hit it again

						verts[k].x = -123;
						verts[k].y = -456;
						verts[k].z = -789;
					}
				}
			}
		}

		// We'll be putting stuff in these...

		vert<>	v0, v1, v2;

		int	startPolyIndex = static_cast<int>(primitives.size());

		for (int j = 0; j < mesh.fCount; j++)
		{
			sFACE		&f = mesh.fList[j];
			if (f.a == f.b || f.b == f.c || f.c == f.a) continue;

			primitive<>	p;
			sP3D		&a = mesh.vList[f.a];
			sP3D		&b = mesh.vList[f.b];
			sP3D		&c = mesh.vList[f.c];
			sP2D		&ta = mesh.uvList[f.a];
			sP2D		&tb = mesh.uvList[f.b];
			sP2D		&tc = mesh.uvList[f.c];

			v0.world() = Point4(a.x, a.y, a.z, 1);
			v1.world() = Point4(b.x, b.y, b.z, 1);
			v2.world() = Point4(c.x, c.y, c.z, 1);
			v0.texture() = Point2(ta.x, 1-ta.y);
			v1.texture() = Point2(tb.x, 1-tb.y);
			v2.texture() = Point2(tc.x, 1-tc.y);

			p += v0;
			p += v1;
			p += v2;
			p.calcPlane(false);
			primitives.push_back(p);
		}

		// Next, clear out the vectors so we can use them as normals

		memset(mesh.vList, 0, mesh.vCount * sizeof(sP3D));

		// Now, accumulate normals...

		int	polyIndex = startPolyIndex;
		for (int k = 0; k < mesh.fCount; k++)
		{
			sFACE		&f = mesh.fList[k];
			if (f.a == f.b || f.b == f.c || f.c == f.a) continue;

			primitive<>	&p = primitives[polyIndex++];
			sP3D		&a = mesh.vList[f.a];
			sP3D		&b = mesh.vList[f.b];
			sP3D		&c = mesh.vList[f.c];
			const Vector3	&n = p.plane().normal();

			a.x += n.x();	a.y += n.y();	a.z += n.z();
			b.x += n.x();	b.y += n.y();	b.z += n.z();
			c.x += n.x();	c.y += n.y();	c.z += n.z();
		}

		// Now normalize them

		polyIndex = startPolyIndex;
		for (int l = 0; l < mesh.fCount; l++)
		{
			sFACE		&f = mesh.fList[l];
			if (f.a == f.b || f.b == f.c || f.c == f.a) continue;

			primitive<>	&p = primitives[polyIndex++];
			sP3D		&a = mesh.vList[f.a];
			sP3D		&b = mesh.vList[f.b];
			sP3D		&c = mesh.vList[f.c];

			p[0].normal() = Vector3(a.x, a.y, a.z);	p[0].normal().normalize();
			p[1].normal() = Vector3(b.x, b.y, b.z);	p[1].normal().normalize();
			p[2].normal() = Vector3(c.x, c.y, c.z);	p[2].normal().normalize();
		}
	}

	// Generate a useful camera

	if (info.camCount)
	{
		cameraDirection.x() = info.cam[0].target.x - info.cam[0].loc.x;
		cameraDirection.y() = info.cam[0].target.y - info.cam[0].loc.y;
		cameraDirection.z() = info.cam[0].target.z - info.cam[0].loc.z;
		cameraDirection.normalize();

		cameraPosition.x() = info.cam[0].loc.x;
		cameraPosition.y() = info.cam[0].loc.y;
		cameraPosition.z() = info.cam[0].loc.z;
		cameraPosition.w() = 1;

		cameraBank = info.cam[0].bank;

		// ----------------------------------------------------------------------------
		//
		// NOTE TO THE READER:
		//
		//	It appears as though there is no definate equation for conversion
		//	between camera lens size (in mm) and FOV (Field Of View).  I have
		//	compiled a table of the most accurate range values for interpolation,
		//	which should be as accurate as possible, and the following routines
		//	interpolate these values.
		//
		//	If my assumption is wrong, please let me know.  But before you do,
		//	make sure that your equation can calculate the EXACT values in the
		//	tables used, as they are very accurate and confirmed so by many
		//	sources.
		//
		//	This also happens to be the way 3D Studio converts between these
		//	values.
		//
		// ----------------------------------------------------------------------------

		{
			enum	{INTERP_RANGE = 11};
			static	const	float	mmInterpList[INTERP_RANGE]  = { 23.90f, 28.58f, 32.55f, 24.0f, 28.0f, 36.66f, 44.09f, 60.61f, 100.36f, 158.01f, 233.34f};
			static	const	float	fovInterpList[INTERP_RANGE] = {100.00f, 83.97f, 73.74f, 84.0f, 76.4f, 65.47f, 54.43f, 35.60f,  23.91f,  15.19f,  10.29f};

			// Check the overflows

			float	mm = info.cam[0].lens;
			if (mm > mmInterpList[INTERP_RANGE-1])
			{
				cameraFOV = 2500.0f / mm;
			}
			else if (mm < mmInterpList[0])
			{
				cameraFOV = 1725.0f / mm;
			}
			else
			{
				// Interp from the table

				int i;
				for (i = 0; i < INTERP_RANGE - 1; i++)
				{
					if (mm >= mmInterpList[i] && mm <= mmInterpList[i+1])
						break;
				}

				float	mmInterpBot  = mmInterpList[i];
				float	mmInterpTop  = mmInterpList[i+1];
				float	fovInterpBot = fovInterpList[i];
				float	fovInterpTop = fovInterpList[i+1];
				float	delta = (mm - mmInterpBot) / (mmInterpTop - mmInterpBot);

				cameraFOV = (fovInterpTop - fovInterpBot) * delta + fovInterpBot;

				// Max doesn't seem to use real FOV values (their shots tend to be a bit tighter) -- this is to hack that

				cameraFOV *= 0.85f;
			}
		}
	}
	else
	{
		cameraDirection.x() = 0;
		cameraDirection.y() = 0;
		cameraDirection.z() = 1;
		cameraDirection.normalize();

		cameraPosition.x() = 0;
		cameraPosition.y() = 0;
		cameraPosition.z() = -500;
		cameraPosition.w() = 1;

		cameraBank = 0;
		cameraFOV = 60;
	}

	// Lights

	for (unsigned int i = 0; i < info.lightCount; ++i)
	{
		sLGT &	light = info.light[i];
		sLIGHT	newLight;

		// Skip disabled lights

		if (light.noLight) continue;

		// Skip non-spotlights

		if (!light.spotLight) continue;

		newLight.pos = Point4(light.loc.x, light.loc.y, light.loc.z, 1);
		newLight.dir = Point3(light.target.x - light.loc.x, light.target.y - light.loc.y, light.target.z - light.loc.z);
		newLight.dir.normalize();
		newLight.color = Point3(light.rgb.r, light.rgb.g, light.rgb.b) / 255.0f * light.intensity;
		newLight.hotspot = cosf(3.141592654f / 180.0f * (light.hotSpot/2));
		newLight.falloff = cosf(3.141592654f / 180.0f * (light.fallOff/2));
		newLight.innerRange = light.innerRange;
		newLight.outerRange = light.outerRange;

		lights.push_back(newLight);
	}

	cameraBank *= 3.14159265359f / 180.0f;
	cameraFOV *= 3.14159265359f / 180.0f;
}

// ---------------------------------------------------------------------------------------------------------------------------------

sVERT *	Render::transformAndClip(const Camera & camera, const Matrix4 & xform, const Jpeg & texture, std::vector<primitive<> > & primitives, unsigned int & renderPolygonCount)
{
	// Transform the geometry

	for (unsigned int i = 0; i < primitives.size(); i++)
	{
		primitive<> &	p = primitives[i];
		for (unsigned int j = 0; j < p.vertexCount(); j++)
		{
			p[j].xform(xform);
		}
		p.calcViewPlane(false);
	}

	// Screen center

	Point2	screenCenter(static_cast<float>(camera.width) / 2.0f, static_cast<float>(camera.height) / 2.0f);

	// Clipping operations suffer from precision issues with 32-bit IEEE floating point. So we'll simply scale
	// our output vertices so we end up with a 1-pixel gap on each side to ensure that we never draw
	// off-screen (which would crash).

	float	xPixelBorderScalar = (screenCenter.x() - 1) / screenCenter.x();
	float	yPixelBorderScalar = (screenCenter.y() - 1) / screenCenter.y();

	// Transform, project & clip the polygons into vertex arrays

	sVERT *		renderVertices = new sVERT[64 * primitives.size()];
	renderPolygonCount = 0;

	for (unsigned int i = 0; i < primitives.size(); i++)
	{
		// Note this is a copy of the primitive, not the actual primitive. This is because we're going to clip it

		primitive<>	p = primitives[i];

		// Backface culling

		if (p[0].normalView().z() >= 0 && p[1].normalView().z() >= 0 && p[2].normalView().z() >= 0) continue;

		// Transform & code the vertices

		unsigned int	codeOff = (unsigned int) -1;
		unsigned int	codeOn = 0;
		unsigned int	j;
		for (j = 0; j < p.vertexCount(); j++)
		{
			// World view

			Point4 &	wv = p[j].worldView();

			unsigned int	code =	(wv.x() >  wv.w() ?  1:0) | (wv.x() < -wv.w() ?  2:0) |
						(wv.y() >  wv.w() ?  4:0) | (wv.y() < -wv.w() ?  8:0) |
						(wv.z() <     0.0 ? 16:0) | (wv.z() >  wv.w() ? 32:0);
			codeOff &= code;
			codeOn  |= code;
		}

		// Completely coded off-screen?

		if (codeOff) continue;

		// Only bother trying to clip if it's partially off-screen

		if (codeOn && !clipPrimitive(p)) continue;

		// Project

		sVERT *		verts = renderVertices + renderPolygonCount * 64;
		for (j = 0; j < p.vertexCount(); j++)
		{
			vert<> &	v = p[j];

			float	ow = 1.0f / v.worldView().w();
			verts[j].screen.x() = screenCenter.x() + v.worldView().x() * ow * screenCenter.x() * xPixelBorderScalar;
			verts[j].screen.y() = screenCenter.y() - v.worldView().y() * ow * screenCenter.y() * yPixelBorderScalar;
			verts[j].world = v.world() * ow;
			verts[j].normal = v.normalView() * ow;
			verts[j].view.x() = v.worldView().x() * ow;
			verts[j].view.y() = v.worldView().y() * ow;
			verts[j].view.z() = v.worldView().z() * ow;
			verts[j].view.w() = ow;
			verts[j].texture.u() = v.textureView().x() * ow * texture.width();
			verts[j].texture.v() = v.textureView().y() * ow * texture.height();
			verts[j].polygonID = i;
			verts[j].next = &verts[j+1];
		}
		verts[j-1].next = NULL;

		// Count this one

		++renderPolygonCount;
	}

	return renderVertices;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	Render::renderGeometry(Jpeg & image, const Camera & camera, const sPHONG & phong, const sVERT * renderVertices, const unsigned int renderPolygonCount, const std::vector<sLIGHT> & lights, const std::vector<ShadowMap> & shadowMaps, const Jpeg & texture)
{
	// Allocate our accumulation buffer

	unsigned int *	accumBuffer = new unsigned int[camera.width * camera.height * 3];
	memset(accumBuffer, 0, camera.width * camera.height * 3 * sizeof(unsigned int));

	// Allocate the frame buffer and z-buffer

	unsigned int *	frameBuffer = new unsigned int[camera.width * camera.height];
	float *		zBuffer = new float[camera.width * camera.height];

	// Allocate a new texture for use as a 32-bit surface

	unsigned int *	textureBuffer = new unsigned int[texture.width() * texture.height()];
	convert24To32(textureBuffer, texture.buffer(), texture.width(), texture.height());

	int	renderCount = 1;
	int	totalRenders = camera.oversampleX * camera.oversampleY;
	for (unsigned int y = 0; y < camera.oversampleY; ++y)
	{
		// Our Antialiasing offset in the Y direction

		float	yAAOffset = static_cast<float>(y) / static_cast<float>(camera.oversampleY);

		for (unsigned int x = 0; x < camera.oversampleX; ++x, ++renderCount)
		{
			printf("(%03d of %03d)\b\b\b\b\b\b\b\b\b\b\b\b", renderCount, totalRenders);

			// Our Antialiasing offset in the X direction

			float	xAAOffset = static_cast<float>(x) / static_cast<float>(camera.oversampleX);

			// Clear these out...

			memset(frameBuffer, 0, camera.width * camera.height * sizeof(unsigned int));
			memset(zBuffer, 0, camera.width * camera.height * sizeof(float));

			// Render the pre-transformed polygons

			for (unsigned int i = 0; i < renderPolygonCount; i++)
			{
				// Build a set of vertices that are offset

				sVERT		offsetVerts[64];
				const sVERT *	src = renderVertices + i * 64;
				sVERT *		dst = offsetVerts;
				for (; src; src = src->next, dst = dst->next)
				{
					*dst = *src;
					dst->screen.x() += xAAOffset;
					dst->screen.y() += yAAOffset;
					dst->next = dst+1;
				}
				(dst-1)->next = NULL;

				// Draw it

				drawPerspectiveTexturedPolygon(offsetVerts, lights, shadowMaps, phong, frameBuffer, textureBuffer, zBuffer, camera.width, texture.width(), texture.height());
			}

			// Accumulate the results for antialiasing

			accumulateBuffer(accumBuffer, frameBuffer, camera.width, camera.height);
		}
	}

	// Done with these

	delete[] zBuffer;
	delete[] textureBuffer;

	// Downsample the accumulation buffer into a single standard 32-bit image buffer

	downsample(accumBuffer, camera.width, camera.height, camera.oversampleX, camera.oversampleY);

	// Create an RGB Jpeg image from the 32-bit rendered image stored in the frameBuffer

	convert32To24(image.buffer(), accumBuffer, camera.width, camera.height);

	// We're officially done with this

	delete[] accumBuffer;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	Render::renderShadowMap(ShadowMap & map, const Camera & camera, sVERT * renderVertices, const unsigned int renderPolygonCount)
{
	// Allocate the z-buffer

	memset(map.zBuffer, 0, camera.width * camera.height * sizeof(float));

	// Render the pre-transformed polygons

	for (unsigned int i = 0; i < renderPolygonCount; i++)
	{
		// Build a set of vertices that are offset

		sVERT *	verts = renderVertices + i * 64;

		// Draw it

		drawShadowMapPolygon(verts, map.zBuffer, camera.width);
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	Render::convert32To24(unsigned char * dest, const unsigned int * source, const unsigned int width, const unsigned int height)
{
	unsigned int		pixCount = width * height;
	const unsigned int *	src = source;
	unsigned char *		dst = dest;

	for (unsigned int i = 0; i < pixCount; ++i, ++src, dst += 3)
	{
		const unsigned int &	pix = *src;
		dst[0] = (pix>>16) & 0xff;
		dst[1] = (pix>> 8) & 0xff;
		dst[2] = (pix>> 0) & 0xff;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	Render::convert24To32(unsigned int * dest, const unsigned char * source, const unsigned int width, const unsigned int height)
{
	unsigned int		pixCount = width * height;
	const unsigned char *	src = source;
	unsigned int *		dst = dest;

	for (unsigned int i = 0; i < pixCount; ++i, src += 3, ++dst)
	{
		*dst = (src[0]<<16) | (src[1]<<8) | src[2];
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	Render::accumulateBuffer(unsigned int * accumBuffer, const unsigned int * frameBuffer, const unsigned int width, const unsigned int height)
{
	unsigned int		pixCount = width * height;
	unsigned int *		dst = accumBuffer;
	const unsigned int *	src = frameBuffer;
	for (unsigned int i = 0; i < pixCount; ++i, ++src)
	{
		const unsigned int &	pix = *src;
		*(dst++) += (pix>>16) & 0xff;
		*(dst++) += (pix>> 8) & 0xff;
		*(dst++) += (pix    ) & 0xff;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	Render::downsample(unsigned int * buffer, const unsigned int width, const unsigned int height, const unsigned int oversampleX, const unsigned int oversampleY)
{
	unsigned int		pixCount = width * height;
	unsigned int		totalSamples = oversampleX * oversampleY;
	const unsigned int *	src = buffer;
	unsigned int *		dst = buffer;

	for (unsigned int i = 0; i < pixCount; ++i, ++dst)
	{
		unsigned int	r = *(src++) / totalSamples;
		unsigned int	g = *(src++) / totalSamples;
		unsigned int	b = *(src++) / totalSamples;
		*dst = (r << 16) | (g << 8) | b;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------
// Render.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
