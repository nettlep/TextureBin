// ---------------------------------------------------------------------------------------------------------------------------------
//  _______           _                   ____  _           _     
// |__   __|         | |                 |  _ \(_)         | |    
//    | |    _____  __ |_ _   _ _ __  ___| |_) |_ _ __     | |__  
//    | |   / _ \ \/ / __| | | | '__|/ _ \  _ <| | '_ \    | '_ \ 
//    | |  |  __/>  <| |_| |_| | |  |  __/ |_) | | | | | _ | | | |
//    |_|   \___/_/\_\\__|\__,_|_|   \___|____/|_|_| |_|(_)|_| |_|
//
// Description:
//
//   Application entry point
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

#ifndef	_H_TEXTUREBIN
#define _H_TEXTUREBIN

// ---------------------------------------------------------------------------------------------------------------------------------
// Includes needed by all files
// ---------------------------------------------------------------------------------------------------------------------------------

#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <string>
#include <vector>
#include <list>

#ifndef _MSC_VER
#include <unistd.h>
#include <dirent.h>
#else
#include <io.h>
#endif

// Compatibility issues

#ifndef _MSC_VER
#define	stricmp strcasecmp
#endif

#endif // _H_TEXTUREBIN
// ---------------------------------------------------------------------------------------------------------------------------------
// TextureBin.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
