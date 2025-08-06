// ---------------------------------------------------------------------------------------------------------------------------------
//  _     _           _                 _     
// | |   | |         | |               | |    
// | |   | | ___ _ __| |_  _____  __   | |__  
//  \ \ / / / _ \ '__| __|/ _ \ \/ /   | '_ \ 
//   \ V / |  __/ |  | |_|  __/>  <  _ | | | |
//    \_/   \___|_|   \__|\___/_/\_\(_)|_| |_|
//                                            
//                                            
//
// Simplistic vertex management class
//
// Best viewed with 8-character tabs and (at least) 132 columns
//
// ---------------------------------------------------------------------------------------------------------------------------------
//
// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep
//
// ---------------------------------------------------------------------------------------------------------------------------------
// Originally created on 12/06/2000 by Paul Nettle
//
// Copyright 2000, Fluid Studios, Inc., all rights reserved.
// ---------------------------------------------------------------------------------------------------------------------------------

#ifndef	_H_VERTEX
#define	_H_VERTEX

// ---------------------------------------------------------------------------------------------------------------------------------
// Required includes
// ---------------------------------------------------------------------------------------------------------------------------------

#include "vmath"

// ---------------------------------------------------------------------------------------------------------------------------------

template<class T = float>
class	vert
{
public:
	// Construction/Destruction

inline				vert() {}
inline				vert(const vert<> & v) {*this = v;}

	// Implementation

inline		void		xform(const Matrix4 & matrix)
				{
					worldView() = matrix >> world();
					Vector4	t = normal();
					t.w() = 0;
					normalView() = matrix >> t;
					textureView() = texture();
				}

	// Accessors

inline	const	Point2 &	texture() const		{return _texture;}
inline		Point2 &	texture()		{return _texture;}
inline	const	Point4 &	world() const		{return _world;}
inline		Point4 &	world()			{return _world;}
inline	const	Point3 &	normal() const		{return _normal;}
inline		Point3 &	normal()		{return _normal;}
inline	const	Point2 &	textureView() const	{return _textureView;}
inline		Point2 &	textureView()		{return _textureView;}
inline	const	Point4 &	worldView() const	{return _worldView;}
inline		Point4 &	worldView()		{return _worldView;}
inline	const	Point3 &	normalView() const	{return _normalView;}
inline		Point3 &	normalView()		{return _normalView;}

protected:
	// Data

		Point2		_texture;
		Point4		_world;
		Point3		_normal;
		Point2		_textureView;
		Point4		_worldView;
		Point3		_normalView;
};

#endif // _H_VERTEX
// ---------------------------------------------------------------------------------------------------------------------------------
// Vertex.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------

