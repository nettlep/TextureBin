// ---------------------------------------------------------------------------------------------------------------------------------
//  _______           _                   ____  _                            
// |__   __|         | |                 |  _ \(_)                           
//    | |    _____  __ |_ _   _ _ __  ___| |_) |_ _ __       ___ _ __  _ __  
//    | |   / _ \ \/ / __| | | | '__|/ _ \  _ <| | '_ \     / __| '_ \| '_ \ 
//    | |  |  __/>  <| |_| |_| | |  |  __/ |_) | | | | | _ | (__| |_) | |_) |
//    |_|   \___/_/\_\\__|\__,_|_|   \___|____/|_|_| |_|(_) \___| .__/| .__/ 
//                                                              | |   | |    
//                                                              |_|   |_|    
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

#include "texturebin.h"
#include "render.h"
#include "tmap.h"

// ---------------------------------------------------------------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _MSC_VER
static	const	char		fileSystemSlash = '\\';
#else
static	const	char		fileSystemSlash = '/';
#endif

static	const	unsigned int	defaultJPEGQuality = 80;
static	const	unsigned int	defaultRenderWidth = 400;
static	const	unsigned int	defaultRenderHeight = 300;
static	const	unsigned int	defaultOversampleX = 4;
static	const	unsigned int	defaultOversampleY = 4;
static	const	float		defaultKa = 0.1f;
static	const	float		defaultKd = 1;
static	const	float		defaultKs = 0.7f;
static	const	float		defaultSh = 10;
static	const	float		defaultShadowMapBias = 2;
static	const	int		defaultShadowMapRes = 1024;
static	const	Point3		defaultAmbientColor(1,1,1);
static	const	Point3		defaultSpecularColor(1,1,1);
static	const	std::string	defaultSceneFilename(std::string("scenes") + fileSystemSlash + std::string("default.3ds"));

// ---------------------------------------------------------------------------------------------------------------------------------

static	void	printUsage(const char * argv0)
{
	// Locate the program filename from the path

	char *	slash = strrchr(argv0, fileSystemSlash);
	char	programName[4096];
	if (slash)	strcpy(programName, slash+1);
	else		strcpy(programName, argv0);

#ifdef _MSC_VER
	// Strip off the exe

	char *	extension = strstr(programName, ".exe");
	if (extension) *extension = 0;
#endif

	fprintf(stderr, "Usage: %s [options] <input specification [...]>\n", programName);
	fprintf(stderr, "       -dNNN store all output images in directory NNN.\n");
	fprintf(stderr, "       -h    this help\n");
	fprintf(stderr, "       -p    pause and wait for a key on error\n");
	fprintf(stderr, "       -qNNN set the output JPEG quality to NNN (0...100, default = %d)\n", defaultJPEGQuality);
	fprintf(stderr, "       -r    recurse through subdirectories of <input specification>\n");
	fprintf(stderr, "       -sNNN set the scene filename to NNN (default = %s)\n", defaultSceneFilename.c_str());
	fprintf(stderr, "       -tNNN set oversample (X direction only) to NNN (1...16, default = %d)\n", defaultOversampleX);
	fprintf(stderr, "       -uNNN set oversample (Y direction only) to NNN (1...16, default = %d)\n", defaultOversampleY);
	fprintf(stderr, "       -xNNN render width (default = %d)\n", defaultRenderWidth);
	fprintf(stderr, "       -yNNN render height (default = %d)\n", defaultRenderHeight);
	fprintf(stderr, "\n");
	fprintf(stderr, "Shadow map options:\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "	-iBNNN set shadow map bias to NNN (default = %f)\n", defaultShadowMapBias);
	fprintf(stderr, "	-iRNNN set shadow map resolution to NNN (default = %d)\n", defaultShadowMapRes);
	fprintf(stderr, "\n");
	fprintf(stderr, "Phong illumination options:\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "	-iKaNNN set ambient coefficient to NNN (default = %.1f)\n", defaultKa);
	fprintf(stderr, "	-iKdNNN set diffuse coefficient to NNN (default = %.1f)\n", defaultKd);
	fprintf(stderr, "	-iKsNNN set specular coefficient to NNN (default = %.1f)\n", defaultKs);
	fprintf(stderr, "	-iShNNN set shininess to NNN (default = %.1f)\n", defaultSh);
	fprintf(stderr, "	-iArNNN set Red ambient light color component to NNN (default = %f)\n", defaultAmbientColor.r());
	fprintf(stderr, "	-iAgNNN set Green ambient light color component to NNN (default = %f)\n", defaultAmbientColor.g());
	fprintf(stderr, "	-iAbNNN set Blue ambient light color component to NNN (default = %f)\n", defaultAmbientColor.b());
	fprintf(stderr, "	-iSrNNN set Red specular light color component to NNN (default = %f)\n", defaultSpecularColor.r());
	fprintf(stderr, "	-iSgNNN set Green specular light color component to NNN (default = %f)\n", defaultSpecularColor.g());
	fprintf(stderr, "	-iSbNNN set Blue specular light color component to NNN (default = %f)\n", defaultSpecularColor.b());
	fprintf(stderr, "\n");
	fprintf(stderr, "   *** WARNING *** all output files will be overwritten without warning!\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "   The input specification can be a directory or an input JPEG file.\n");
	fprintf(stderr, "   If a directory is given, then it will not be recursed unless the -r\n");
	fprintf(stderr, "   parameter is used. Multiple input specifications may be given.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "   Note that it is safe to always use -r (recursion), as this will be\n");
	fprintf(stderr, "   ignored if the input specification is not a directory.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "   If a directory is given, only files with the extension 'jpg' (case-\n");
	fprintf(stderr, "   insensitive) will be processed.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "   For best oversampling results, set both values to be equal. Oversampling\n");
	fprintf(stderr, "   works by rendering the image multiple times, where each render is slightly\n");
	fprintf(stderr, "   offset from one another. The results are accumulated and then averaged.\n");
	fprintf(stderr, "   Note that this can really slow things down with large values. For example,\n");
	fprintf(stderr, "   16x16 oversampling causes the image to be rendered 256 times.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "   The program will return '0' on error, and '1' on success.\n");

	// Cause an error-free immediate exit from the program

	throw std::string("");
}

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _MSC_VER
void	recursiveFindFiles(const std::string & path, const bool recurse, std::vector<std::string> & fileList)
{
	// Correct the input path to make sure it has a trailing slash

	std::string	correctPath = path;
	if (correctPath[correctPath.length() - 1] != fileSystemSlash) correctPath += fileSystemSlash;

	// We'll scan all files

	std::string	spec = correctPath;
	spec.append("*.*");

	// Being the file find process

	_finddata_t	findInfo;
	intptr_t	handle = _findfirst(spec.c_str(), &findInfo);
	if (handle == -1 || handle == ENOENT || handle == EINVAL) throw std::string("Unable to scan directory: ").append(spec);

	do
	{
		std::string	fullName = correctPath;
		fullName.append(findInfo.name);

		// Stat the file

		struct _stat statbuf;
		if (!_stat(fullName.c_str(), &statbuf))
		{
			// The current filespec

			std::string	thisFile = findInfo.name;

			// If it's a dir, keep looking

			if (statbuf.st_mode & _S_IFDIR)
			{
				// Don't bother recursing into '.' and '..'

				if (thisFile.compare(".") && thisFile.compare(".."))
				{
					std::string	newPath = correctPath;
					newPath.append(thisFile);
					if (recurse) recursiveFindFiles(newPath, recurse, fileList);
				}
			}

			// Regular file

			else
			{
				// Get the extension

				std::string::size_type	idx = thisFile.rfind('.');
				if (idx != std::string::npos)
				{
					// Is it a file we're lookin' for?

					std::string	ext = thisFile.substr(idx);
					if (!stricmp(ext.c_str(), ".jpg"))
					{
						std::string	newPath = correctPath;
						newPath.append(thisFile);
						fileList.push_back(newPath);
					}
				}
			}
		}
	} while(!_findnext(handle, &findInfo));

	_findclose(handle);
}

#else // LINUX VERSION

void	recursiveFindFiles(const std::string & path, const bool recurse, std::vector<std::string> & fileList)
{
	// Correct the input path to make sure it has a trailing slash

	std::string	correctPath = path;
	if (correctPath[correctPath.length() - 1] != fileSystemSlash) correctPath += fileSystemSlash;

	// Being the file find process

	struct	dirent *	dp;
	DIR *			dirp = opendir(correctPath.c_str());
	if (!dirp) throw std::string("Unable to scan directory: ").append(correctPath);

	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
	{
		std::string	fullName = correctPath;
		fullName.append(dp->d_name);

		// Stat the file

		struct stat statbuf;
		if (!stat(fullName.c_str(), &statbuf))
		{
			// The current filespec

			std::string	thisFile = dp->d_name;

			// If it's a dir, keep looking

			if (statbuf.st_mode & S_IFDIR)
			{
				// Don't bother recursing into '.' and '..'

				if (thisFile.compare(".") && thisFile.compare(".."))
				{
					std::string	newPath = correctPath;
					newPath.append(thisFile);
					if (recurse) recursiveFindFiles(newPath, recurse, fileList);
				}
			}

			// Regular file

			else
			{
				// Get the extension

				std::string::size_type	idx = thisFile.rfind('.');
				if (idx != std::string::npos)
				{
					// Is it a file we're lookin' for?

					std::string	ext = thisFile.substr(idx);
					if (!stricmp(ext.c_str(), ".jpg"))
					{
						std::string	newPath = correctPath;
						newPath.append(thisFile);
						fileList.push_back(newPath);
					}
				}
			}
		}
	}

	closedir(dirp);
}
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

static	void	parseInputSpecifications(const std::vector<std::string> & inputSpecifications, const bool recurse, std::vector<std::string> & processFilenames)
{
	// Loop through the input specs

	for (unsigned int i = 0; i < inputSpecifications.size(); ++i)
	{
		// The current specification

		std::string	thisSpec = inputSpecifications[i];

		// Stat the filespec

#ifdef _MSC_VER
		struct _stat	statbuf;
		if (_stat(thisSpec.c_str(), &statbuf)) throw std::string("Unable to stat the filespec: ").append(thisSpec);

		// if it's a file, just add the file

		if (!(statbuf.st_mode & _S_IFDIR))
#else
		struct stat	statbuf;
		if (stat(thisSpec.c_str(), &statbuf)) throw std::string("Unable to stat the filespec: ").append(thisSpec);

		// if it's a file, just add the file

		if (!(statbuf.st_mode & S_IFDIR))
#endif
		{
			processFilenames.push_back(thisSpec);
			continue;
		}

		// Must be a directory -- skip "." and ".."

		if (!thisSpec.compare(".") || !thisSpec.compare("..")) continue;

		// Scan the directory

		recursiveFindFiles(thisSpec, recurse, processFilenames);
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

int	main(const int argc, const char * argv[])
{
	// Command line parameters and their defaults

	bool				pauseOnError = false;
	bool				recurse = false;
	unsigned int			jpegQuality = defaultJPEGQuality;
	unsigned int			renderWidth = defaultRenderWidth;
	unsigned int			renderHeight = defaultRenderHeight;
	unsigned int			oversampleX = defaultOversampleX;
	unsigned int			oversampleY = defaultOversampleY;
	float				Ka = defaultKa;
	float				Kd = defaultKd;
	float				Ks = defaultKs;
	float				Sh = defaultSh;
	float				shadowMapBias = defaultShadowMapBias;
	int				shadowMapRes = defaultShadowMapRes;
	Point3				ambientColor = defaultAmbientColor;
	Point3				specularColor = defaultSpecularColor;
	std::string			sceneFilename = defaultSceneFilename;
	std::string			destinationDirectory;
	std::vector<std::string>	inputSpecifications;

#ifndef _MSC_VER
	setbuf(stdout, 0);
#endif

	try
	{
		// No parameters means give them help

		if (argc == 1) printUsage(argv[0]);

		// Parse the command line

		for (int i = 1; i < argc; ++i)
		{
			// Command-line switch?

			if (argv[i][0] == '-' || argv[i][0] == '/')
			{
				switch(tolower(argv[i][1]))
				{
					case 'd':
						destinationDirectory = &argv[i][2];
						if (destinationDirectory.length() && destinationDirectory[destinationDirectory.length()-1] != fileSystemSlash)
							destinationDirectory += fileSystemSlash;
						break;

					case 'h':
						printUsage(argv[0]);
						break;

					case 'i':
						if (tolower(argv[i][2]) == 'k' && tolower(argv[i][3]) == 'a')
						{
							Ka = static_cast<float>(atof(&argv[i][4]));
						}
						else if (tolower(argv[i][2]) == 'k' && tolower(argv[i][3]) == 'd')
						{
							Kd = static_cast<float>(atof(&argv[i][4]));
						}
						else if (tolower(argv[i][2]) == 'k' && tolower(argv[i][3]) == 's')
						{
							Ks = static_cast<float>(atof(&argv[i][4]));
						}
						else if (tolower(argv[i][2]) == 's' && tolower(argv[i][3]) == 'h')
						{
							Sh = static_cast<float>(atof(&argv[i][4]));
						}
						else if (tolower(argv[i][2]) == 'a' && tolower(argv[i][3]) == 'r')
						{
							ambientColor.r() = static_cast<float>(atof(&argv[i][4]));
						}
						else if (tolower(argv[i][2]) == 'a' && tolower(argv[i][3]) == 'g')
						{
							ambientColor.g() = static_cast<float>(atof(&argv[i][4]));
						}
						else if (tolower(argv[i][2]) == 'a' && tolower(argv[i][3]) == 'b')
						{
							ambientColor.b() = static_cast<float>(atof(&argv[i][4]));
						}
						else if (tolower(argv[i][2]) == 's' && tolower(argv[i][3]) == 'r')
						{
							specularColor.r() = static_cast<float>(atof(&argv[i][4]));
						}
						else if (tolower(argv[i][2]) == 's' && tolower(argv[i][3]) == 'g')
						{
							specularColor.g() = static_cast<float>(atof(&argv[i][4]));
						}
						else if (tolower(argv[i][2]) == 's' && tolower(argv[i][3]) == 'b')
						{
							specularColor.b() = static_cast<float>(atof(&argv[i][4]));
						}
						else if (tolower(argv[i][2]) == 'b')
						{
							shadowMapBias = static_cast<float>(atof(&argv[i][3]));
						}
						else if (tolower(argv[i][2]) == 'r')
						{
							shadowMapRes = atoi(&argv[i][3]);
						}
						else
						{
							fprintf(stderr, "Unknown command line option: %s\n\n", argv[i]);
							printUsage(argv[0]);
						}

						break;

					case 'p':
						pauseOnError = true;
						break;

					case 'q':
						jpegQuality = atoi(&argv[i][2]);
						break;

					case 'r':
						recurse = true;
						break;

					case 's':
						sceneFilename = &argv[i][2];
						break;

					case 't':
						oversampleX = atoi(&argv[i][2]);
						break;

					case 'u':
						oversampleY = atoi(&argv[i][2]);
						break;

					case 'x':
						renderWidth = atoi(&argv[i][2]);
						break;

					case 'y':
						renderHeight = atoi(&argv[i][2]);
						break;

					default:
						fprintf(stderr, "Unknown command line option: %s\n\n", argv[i]);
						printUsage(argv[0]);
						break;
				}
			}
			else
			{
				inputSpecifications.push_back(std::string(argv[i]));
			}
		}

		// Make sure we have an input specification

		if (!inputSpecifications.size())
		{
			fprintf(stderr, "No input specification given!\n\n");
			printUsage(argv[0]);
		}

		// Validate our oversample ranges

		if (oversampleX < 1 || oversampleX > 16)
		{
			fprintf(stderr, "Your X oversample value (%d) must be within the range 1...16!\n\n", oversampleX);
			printUsage(argv[0]);
		}

		if (oversampleY < 1 || oversampleY > 16)
		{
			fprintf(stderr, "Your Y oversample value (%d) must be within the range 1...16!\n\n", oversampleY);
			printUsage(argv[0]);
		}

		// Parse the input specifications, creating a list of actual filenames, performing recursion when requested and necessary

		std::vector<std::string>	processFilenames;
		parseInputSpecifications(inputSpecifications, recurse, processFilenames);

		// Setup the phong options

		sPHONG	phong;
		phong.Ka = Ka;
		phong.Kd = Kd;
		phong.Ks = Ks;
		phong.Sh = Sh;
		phong.ambientColor = ambientColor;
		phong.specularColor = specularColor;
		phong.shadowMapBias = shadowMapBias;
		phong.shadowMapRes = shadowMapRes;

		for (unsigned int i = 0; i < processFilenames.size(); ++i)
		{
			Render		render;
			std::string	outputName = processFilenames[i];

			if (destinationDirectory.length())
			{
				std::string::size_type	idx = outputName.rfind(fileSystemSlash);
				if (idx != std::string::npos) outputName.erase(0, idx+1);
				outputName = std::string(destinationDirectory).append(outputName);
			}

			// No destination directory, append '-rendered' into the filename

			else
			{
				outputName.insert(outputName.length() - 4, "-rendered");
			}

			render.renderScene(processFilenames[i], outputName, sceneFilename, renderWidth, renderHeight, oversampleX, oversampleY, jpegQuality, phong);
		}
	}
	catch(const std::string & err)
	{
		if (err.length()) fprintf(stderr, "\nError: %s\n", err.c_str());

		if (pauseOnError)
		{
			fprintf(stderr, "\nPress a key to continue...\n");
			fgetc(stdin);
		}

		return 0;
	}
	catch(...)
	{
		fprintf(stderr, "\nError: I don't have any idea what TYPE of error this is, let alone wtf went wrong. Sorry. :/\n");

		if (pauseOnError)
		{
			fprintf(stderr, "\nPress a key to continue...\n");
			fgetc(stdin);
		}
		return 0;
	}

	return 1;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// TextureBin.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
