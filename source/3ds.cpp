// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep
//
// ** This is some old code I always use to load 3DS files. It works, sorry
// ** if it's ugly and doesn't quite fit in with the coding style of the rest
// ** of the project. I've changed coding styles since I wrote this.

#include "texturebin.h"
#include "3ds.h"

// ----------------------------------------------------------------------------

#define	SWAP(a,b,c) {c = a; a = b; b = c;}
#define	FOREVER for(;;)
//#define DEBUG_INFO

// ----------------------------------------------------------------------------

	C3DS::C3DS()
{
	memset( currentNamedObject, 0, sizeof(currentNamedObject) );
	memset( &kfInfo, 0, sizeof(kfInfo) );

	matCount = 0;
	matList = NULL;
	meshCount = 0;
	meshList = NULL;
	camCount = 0;
	camList = NULL;
	lightCount = 0;
	lightList = NULL;
	objectNodeCount = 0;
	cameraNodeCount = 0;
	targetNodeCount = 0;
	lightNodeCount = 0;
	objectNodeList = NULL;
	cameraNodeList = NULL;
	targetNodeList = NULL;
	lightNodeList = NULL;
}

// ----------------------------------------------------------------------------

	C3DS::~C3DS()
{
	for( int i = 0; i < meshCount; i++ )
	{
		free(meshList[i].vList);
		free(meshList[i].fList);
		free(meshList[i].uvList);
	}

	free(meshList);
	free(matList);
	free(lightList);
	free(camList);
}

// ----------------------------------------------------------------------------

int	C3DS::load(const char *inName, sD3DS *info)
{
	matList    = info->mat;
	meshList   = info->mesh;
	camList    = info->cam;
	lightList  = info->light;
	matCount   = info->matCount;
	meshCount  = info->meshCount;
	camCount   = info->camCount;
	lightCount = info->lightCount;

	objectNodeCount = info->objectNodeCount;
	cameraNodeCount = info->cameraNodeCount;
	targetNodeCount = info->targetNodeCount;
	lightNodeCount  = info->lightNodeCount;
	objectNodeList  = info->objectNodeList;
	cameraNodeList  = info->cameraNodeList;
	targetNodeList  = info->targetNodeList;
	lightNodeList   = info->lightNodeList;

	FILE	*inFile = fopen(inName, "rb");

	if (!inFile) throw std::string("Unable to open input scene file: ").append(inName);

	if (!read3DS(inFile))
	{
		fclose(inFile);
		return 0;
	}

	// Copy back

	info->mat   = matList;
	info->mesh  = meshList;
	info->cam   = camList;
	info->light = lightList;

	info->matCount   = matCount;
	info->meshCount  = meshCount;
	info->camCount   = camCount;
	info->lightCount = lightCount;

	memcpy( &info->kfInfo, &kfInfo, sizeof( sKFHDR ) );
	info->objectNodeCount = objectNodeCount;
	info->cameraNodeCount = cameraNodeCount;
	info->targetNodeCount = targetNodeCount;
	info->lightNodeCount  = lightNodeCount;
	info->objectNodeList  = objectNodeList;
	info->cameraNodeList  = cameraNodeList;
	info->targetNodeList  = targetNodeList;
	info->lightNodeList   = lightNodeList;

	// Now we need to actually swap the y/z coordinates of our vertices

	float	fTemp;
	int	iTemp;

	for( int i = 0; i < meshCount; i++ )
	{
		sMSH &	mesh = meshList[i];
		sP3D *	verts = mesh.vList;

		for( int j = 0; j < meshList[i].vCount; j++ )
		{
			SWAP(verts[j].y, verts[j].z, fTemp);
		}

		for( int j = 0; j < meshList[i].fCount; j++ )
		{
			SWAP(mesh.fList[j].normal.y, meshList[i].fList[j].normal.z, fTemp);
			SWAP(mesh.fList[j].a, meshList[i].fList[j].b, iTemp);
		}
	}

	for( int i = 0; i < camCount; i++ )
	{
		SWAP(camList[i].loc.y, camList[i].loc.z, fTemp);
		SWAP(camList[i].target.y, camList[i].target.z, fTemp);
	}

	for( int i = 0; i < lightCount; i++ )
	{
		SWAP(lightList[i].loc.y, lightList[i].loc.z, fTemp);
		SWAP(lightList[i].target.y, lightList[i].target.z, fTemp);
	}

	fclose(inFile);
	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::read3DS(FILE *inFile)
{
	// Get the first chunk... verify the integrity of the file...

	sCHK	chunk;

	if (!getChunkID(inFile, &chunk))
	{
		throw std::string("Invalid chunk ID");
	}

	if (chunk.ID != CHUNK_3DSFILE && chunk.ID != CHUNK_MLIFILE && chunk.ID != CHUNK_PRJFILE)
	{
		throw std::string("Not a valid 3DS/MLI/PRJ file");
	}

	return process3DS(inFile, chunk.length);
}

// ----------------------------------------------------------------------------

int	C3DS::getChunkID(FILE *file, sCHK *chunk)
{
	return myRead( chunk, sizeof(sCHK), file );
}

// ----------------------------------------------------------------------------

int	C3DS::process3DS(FILE *inFile, int chunkLength)
{
	int	endPos = chunkLength - CHUNK_HEADER_SIZE + ftell(inFile);

	FOREVER
	{
		// Done?

		if (endPos <= ftell(inFile)) break;

		sCHK	chunk;
		int	retCode = getChunkID(inFile, &chunk);

		// Are we done?

		if (!retCode) break;

		switch(chunk.ID)
		{
			case CHUNK_MESH:
				if (!readMeshChunk(inFile, chunk.length))
				{
					throw std::string("Unable to read mesh chunk");
				}
				break;

			case CHUNK_MATERIAL:
				if (!readMaterial(inFile, chunk.length))
				{
					throw std::string("Unable to read material");
				}
				break;

			case CHUNK_KFDATA:
				if (!readKeyframeChunk( inFile, chunk.length))
				{
					throw std::string("Unable to read keyframe info");
				}
				break;

			default:
				#ifdef	DEBUG_INFO
				printf( "Skipping chunk 0x%04X\n", chunk.ID );
				#endif

				skipChunk(inFile, chunk.length);
				break;
		}
	}

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readMeshChunk(FILE *inFile, int chunkLength)
{
	#ifdef	DEBUG_INFO
	printf( "Mesh chunk\n" );
	#endif

	int	endPos = chunkLength - CHUNK_HEADER_SIZE + ftell(inFile);

	FOREVER
	{
		// Done?

		if (endPos <= ftell(inFile)) break;

		sCHK	chunk;
		int	retCode = getChunkID(inFile, &chunk);

		// Make sure it's a sub-chunk...

		if (chunk.ID >> 12 >= 0xB || chunk.ID == CHUNK_MESH)
		{
			backup(inFile);
			return 1;
		}

		// Are we done?

		if (!retCode) break;

		switch(chunk.ID)
		{
			case CHUNK_NAMEDOBJECT:
				if (!readNamedObject(inFile, chunk.length))
				{
					throw std::string("Unable to read named object");
				}
				break;

//			case CHUNK_MATERIAL:
//				if (!readMaterial(inFile, chunk.length))
//				{
//					throw std::string("Unable to read material");
//				}
//				break;

			default:
				#ifdef	DEBUG_INFO
				printf( "Skipping chunk 0x%04X\n", chunk.ID );
				#endif

				skipChunk(inFile, chunk.length);
				break;
		}
	}

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readKeyframeChunk(FILE *inFile, int chunkLength)
{
	#ifdef	DEBUG_INFO
	printf( "KeyframeChunk\n" );
	#endif

	int	endPos = chunkLength - CHUNK_HEADER_SIZE + ftell(inFile);

	FOREVER
	{
		// Done?

		if (endPos <= ftell(inFile)) break;

		sCHK	chunk;
		int	retCode = getChunkID(inFile, &chunk);

		// Are we done?

		if (!retCode) break;

		switch(chunk.ID)
		{
		    	case CHUNK_KFHDR:
				if (!readKeyframeHeader( inFile, chunk.length ))
				{
					throw std::string("Unable to read keyframe header");
				}
				break;

		    	case CHUNK_KFSEG:
				#ifdef	DEBUG_INFO
				printf( "KeyframeSegment" );
				#endif
				
				if (!myRead( &kfInfo.start, sizeof( kfInfo.start ), inFile ))
					return 0;

				if (!myRead( &kfInfo.end, sizeof( kfInfo.end ), inFile ))
					return 0;
				break;

		    	case CHUNK_KFCURTIME:
				#ifdef	DEBUG_INFO
				printf( "KeyframeTime\n" );
				#endif

				if (!myRead( &kfInfo.curFrame, sizeof( kfInfo.curFrame ), inFile ))
					return 0;
				break;

		    	case CHUNK_KFOBJNODE:
				if (!readObjectNode( inFile, chunk.length ))
				{
					throw std::string("Unable to read keyframe header");
				}
				break;

		    	case CHUNK_KFCAMNODE:
				if (!readCameraNode( inFile, chunk.length ))
				{
					throw std::string("Unable to read keyframe header");
				}
				break;

		    	case CHUNK_KFTGTNODE:
				if (!readTargetNode( inFile, chunk.length ))
				{
					throw std::string("Unable to read keyframe header");
				}
				break;

		    	case CHUNK_KFLGTNODE:
				if (!readLightNode( inFile, chunk.length ))
				{
					throw std::string("Unable to read keyframe header");
				}
				break;

			default:
				#ifdef	DEBUG_INFO
				printf( "Skipping chunk 0x%04X\n", chunk.ID );
				#endif

				skipChunk(inFile, chunk.length);
				break;
		}
	}

	#ifdef	DEBUG_INFO	
	printf( "KFHDR->revision : %d\n", kfInfo.revision );
	printf( "KFHDR->filename : %s\n", kfInfo.filename );
	printf( "KFHDR->animLen  : %d\n", kfInfo.animLen );
	printf( "KFHDR->start    : %d\n", kfInfo.start );
	printf( "KFHDR->end      : %d\n", kfInfo.end );
	printf( "KFHDR->curFrame : %d\n", kfInfo.curFrame );
	#endif

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readKeyframeHeader( FILE *inFile, int chunkLength )
{
	#ifdef	DEBUG_INFO
	printf( "KeyframeHeader\n" );
	#endif

	chunkLength = chunkLength;

	if (!myRead( &kfInfo.revision, sizeof( kfInfo.revision ), inFile ))
		return 0;

	if (!getString( inFile, kfInfo.filename ))
		return 0;

	if (!myRead( &kfInfo.animLen, sizeof( kfInfo.animLen ), inFile ))
		return 0;

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readObjectNode( FILE *inFile, int chunkLength )
{
	#ifdef	DEBUG_INFO
	printf( "ObjectNode\n" );
	#endif

	sOBJNODE	*tempNode = (sOBJNODE *) realloc( objectNodeList, sizeof( sOBJNODE ) * (objectNodeCount + 1) );
	
	if (!tempNode)
		return 0;

	objectNodeList = tempNode;

	tempNode = &objectNodeList[objectNodeCount];

	int	endPos = chunkLength - CHUNK_HEADER_SIZE + ftell(inFile);

	FOREVER
	{
		// Done?

		if (endPos <= ftell(inFile)) break;

		sCHK	chunk;
		int	retCode = getChunkID(inFile, &chunk);

		// Are we done?

		if (!retCode) break;

		switch(chunk.ID)
		{
		    	case CHUNK_KFNODEHDR:
			{
				#ifdef	DEBUG_INFO
				printf( "KeyframeObjectNodeHeader\n" );
				#endif

				if (!getString( inFile, tempNode->nodeHeader.name ))
					return 0;

				for (int i = 0; i < meshCount; i++)
				{
					if (!strcmp( tempNode->nodeHeader.name, meshList[i].name ))
					{
						meshList[i].node = tempNode;
						break;
					}
					else
					{
						meshList[i].node = NULL;
					}
				}

				#ifdef	DEBUG_INFO
				printf( "ObjectNodeName: %s\n", tempNode->nodeHeader.name );
				#endif

				if (!myRead( &tempNode->nodeHeader.flags1, sizeof( tempNode->nodeHeader.flags1 ), inFile ))
					return 0;

				if (!myRead( &tempNode->nodeHeader.flags2, sizeof( tempNode->nodeHeader.flags2 ), inFile ))
					return 0;

				if (!myRead( &tempNode->nodeHeader.hierarchy, sizeof( tempNode->nodeHeader.hierarchy ), inFile ))
					return 0;

				break;
			}

			case CHUNK_KFPIVOT:
				#ifdef	DEBUG_INFO
				printf( "KeyframeObjectPivot\n" );
				#endif
				
				if (!myRead( &tempNode->pivot, sizeof( tempNode->pivot ), inFile ))
					return 0;
				break;

			case CHUNK_KFPOSTRACK:
			{
				#ifdef	DEBUG_INFO
				printf( "KeyframeObjectPosition\n" );
				#endif

				if (!myRead( &tempNode->position, sizeof( tempNode->position ) - 4, inFile ))
					return 0;
				
				sPOS	*tempPos = (sPOS *) malloc( sizeof( sPOS ) * tempNode->position.keys );

				if (!tempPos)
					return 0;

				tempNode->position.data = tempPos;

				if (!myRead( tempNode->position.data, sizeof( sPOS ) * tempNode->position.keys, inFile ))
					return 0;

				#ifdef	DEBUG_INFO
				printf( "keys: %d\n", tempNode->position.keys );
				#endif

				for (int i = 0; i < tempNode->position.keys; i++)
				{
					#ifdef	DEBUG_INFO
					printf( "data[%d]: <%3.2f, %3.2f, %3.2f>\n", i,	tempNode->position.data[i].pos.x,
					       						tempNode->position.data[i].pos.y,
					       						tempNode->position.data[i].pos.z );
					#endif
				}

				break;
			}

			case CHUNK_KFROTTRACK:
			{
				#ifdef	DEBUG_INFO
				printf( "KeyframeObjectRotation\n" );
				#endif

				if (!myRead( &tempNode->rotation, sizeof( tempNode->rotation ) - 4, inFile ))
					return 0;
				
				sROT	*tempRot = (sROT *) malloc( sizeof( sROT ) * tempNode->rotation.keys );

				if (!tempRot)
					return 0;

				tempNode->rotation.data = tempRot;

				if (!myRead( tempNode->rotation.data, sizeof( sROT ) * tempNode->rotation.keys, inFile ))
					return 0;

				#ifdef	DEBUG_INFO
				printf( "keys: %d\n", tempNode->rotation.keys );
				#endif
				
				for (int i = 0; i < tempNode->rotation.keys; i++)
				{
					#ifdef	DEBUG_INFO
					printf( "data[%d]: <%3.2f, %3.2f, %3.2f>\n", i,	tempNode->rotation.data[i].axis.x,
					       						tempNode->rotation.data[i].axis.y,
					       						tempNode->rotation.data[i].axis.z );
					#endif
				}

				break;
			}

			case CHUNK_KFSCLTRACK:
			{
				#ifdef	DEBUG_INFO
				printf( "KeyframeObjectScale\n" );
				#endif

				if (!myRead( &tempNode->scale, sizeof( tempNode->scale ) - 4, inFile ))
					return 0;
				
				sSCL	*tempScl = (sSCL *) malloc( sizeof( sSCL ) * tempNode->scale.keys );

				if (!tempScl)
					return 0;

				tempNode->scale.data = tempScl;

				if (!myRead( tempNode->scale.data, sizeof( sSCL ) * tempNode->scale.keys, inFile ))
					return 0;

				#ifdef	DEBUG_INFO
				printf( "keys: %d\n", tempNode->scale.keys );
				#endif

				for (int i = 0; i < tempNode->scale.keys; i++)
				{
					#ifdef	DEBUG_INFO
					printf( "data[%d]: <%3.2f, %3.2f, %3.2f>\n", i,	tempNode->scale.data[i].scale.x,
					       						tempNode->scale.data[i].scale.y,
					       						tempNode->scale.data[i].scale.z );
					#endif
				}

				break;
			}

			default:
				#ifdef	DEBUG_INFO
				printf( "Skipping chunk 0x%04X\n", chunk.ID );
				#endif

				skipChunk(inFile, chunk.length);
				break;
		}
	}

	objectNodeCount++;

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readCameraNode( FILE *inFile, int chunkLength )
{
	#ifdef	DEBUG_INFO
	printf( "CameraNode\n" );
	#endif

	sCAMNODE	*tempNode = (sCAMNODE *) realloc( cameraNodeList, sizeof( sCAMNODE ) * (cameraNodeCount + 1) );
	
	if (!tempNode)
		return 0;

	cameraNodeList = tempNode;

	tempNode = &cameraNodeList[cameraNodeCount];

	int	endPos = chunkLength - CHUNK_HEADER_SIZE + ftell(inFile);

	FOREVER
	{
		// Done?

		if (endPos <= ftell(inFile)) break;

		sCHK	chunk;
		int	retCode = getChunkID(inFile, &chunk);

		// Are we done?

		if (!retCode) break;

		switch(chunk.ID)
		{
		    	case CHUNK_KFNODEHDR:
				#ifdef	DEBUG_INFO
				printf( "KeyframeCameraNodeHeader\n" );
				#endif

				if (!getString( inFile, tempNode->nodeHeader.name ))
					return 0;

				if (!myRead( &tempNode->nodeHeader.flags1, sizeof( tempNode->nodeHeader.flags1 ), inFile ))
					return 0;

				if (!myRead( &tempNode->nodeHeader.flags2, sizeof( tempNode->nodeHeader.flags2 ), inFile ))
					return 0;

				if (!myRead( &tempNode->nodeHeader.hierarchy, sizeof( tempNode->nodeHeader.hierarchy ), inFile ))
					return 0;

				break;

			case CHUNK_KFPOSTRACK:
			{
				#ifdef	DEBUG_INFO
				printf( "KeyframeCameraPosition\n" );
				#endif

				if (!myRead( &tempNode->position, sizeof( tempNode->position ) - 4, inFile ))
					return 0;
				
				sPOS	*tempPos = (sPOS *) malloc( sizeof( sPOS ) * tempNode->position.keys );

				if (!tempPos)
					return 0;

				tempNode->position.data = tempPos;

				if (!myRead( tempNode->position.data, sizeof( sPOS ) * tempNode->position.keys, inFile ))
					return 0;

				#ifdef	DEBUG_INFO
				printf( "keys: %d\n", tempNode->position.keys );
				#endif

				for (int i = 0; i < tempNode->position.keys; i++)
				{
					#ifdef	DEBUG_INFO
					printf( "data[%d]: <%3.2f, %3.2f, %3.2f>\n", i,	tempNode->position.data[i].pos.x,
					       						tempNode->position.data[i].pos.y,
					       						tempNode->position.data[i].pos.z );
					#endif
				}

				break;
			}

			case CHUNK_KFFOVTRACK:
			{
				#ifdef	DEBUG_INFO
				printf( "KeyframeCameraFieldOfView\n" );
				#endif

				if (!myRead( &tempNode->fov, sizeof( tempNode->fov ) - 4, inFile ))
					return 0;
				
				sFOV	*tempFov = (sFOV *) malloc( sizeof( sFOV ) * tempNode->fov.keys );

				if (!tempFov)
					return 0;

				tempNode->fov.data = tempFov;

				if (!myRead( tempNode->fov.data, sizeof( sFOV ) * tempNode->fov.keys, inFile ))
					return 0;

				#ifdef	DEBUG_INFO
				printf( "keys: %d\n", tempNode->fov.keys );
				#endif
				
				for (int i = 0; i < tempNode->fov.keys; i++)
				{
					#ifdef	DEBUG_INFO
					printf( "data[%d]: <%3.2f>\n", i, tempNode->fov.data[i].fov );
					#endif
				}

				break;
			}

			case CHUNK_KFROLTRACK:
			{
				#ifdef	DEBUG_INFO
				printf( "KeyframeCameraRoll\n" );
				#endif

				if (!myRead( &tempNode->roll, sizeof( tempNode->roll ) - 4, inFile ))
					return 0;
				
				sROL	*tempRol = (sROL *) malloc( sizeof( sROL ) * tempNode->roll.keys );

				if (!tempRol)
					return 0;

				tempNode->roll.data = tempRol;

				if (!myRead( tempNode->roll.data, sizeof( sROL ) * tempNode->roll.keys, inFile ))
					return 0;

				#ifdef	DEBUG_INFO
				printf( "keys: %d\n", tempNode->roll.keys );
				#endif

				for (int i = 0; i < tempNode->roll.keys; i++)
				{
					#ifdef	DEBUG_INFO
					printf( "data[%d]: <%3.2f>\n", i, tempNode->roll.data[i].roll );
					#endif
				}

				break;
			}

			default:
				#ifdef	DEBUG_INFO
				printf( "Skipping chunk 0x%04X\n", chunk.ID );
				#endif

				skipChunk(inFile, chunk.length);
				break;
		}
	}

	cameraNodeCount++;

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readTargetNode( FILE *inFile, int chunkLength )
{
	#ifdef	DEBUG_INFO
	printf( "TargetNode\n" );
	#endif

	sTGTNODE	*tempNode = (sTGTNODE *) realloc( targetNodeList, sizeof( sTGTNODE ) * (targetNodeCount + 1) );
	
	if (!tempNode)
		return 0;

	targetNodeList = tempNode;

	tempNode = &targetNodeList[targetNodeCount];

	int	endPos = chunkLength - CHUNK_HEADER_SIZE + ftell(inFile);

	FOREVER
	{
		// Done?

		if (endPos <= ftell(inFile)) break;

		sCHK	chunk;
		int	retCode = getChunkID(inFile, &chunk);

		// Are we done?

		if (!retCode) break;

		switch(chunk.ID)
		{
		    	case CHUNK_KFNODEHDR:
				#ifdef	DEBUG_INFO
				printf( "KeyframeTargetNodeHeader\n" );
				#endif

				if (!getString( inFile, tempNode->nodeHeader.name ))
					return 0;

				if (!myRead( &tempNode->nodeHeader.flags1, sizeof( tempNode->nodeHeader.flags1 ), inFile ))
					return 0;

				if (!myRead( &tempNode->nodeHeader.flags2, sizeof( tempNode->nodeHeader.flags2 ), inFile ))
					return 0;

				if (!myRead( &tempNode->nodeHeader.hierarchy, sizeof( tempNode->nodeHeader.hierarchy ), inFile ))
					return 0;

				break;

			case CHUNK_KFPOSTRACK:
			{
				#ifdef	DEBUG_INFO
				printf( "KeyframeTargetPosition\n" );
				#endif

				if (!myRead( &tempNode->position, sizeof( tempNode->position ) - 4, inFile ))
					return 0;
				
				sPOS	*tempPos = (sPOS *) malloc( sizeof( sPOS ) * tempNode->position.keys );

				if (!tempPos)
					return 0;

				tempNode->position.data = tempPos;

				if (!myRead( tempNode->position.data, sizeof( sPOS ) * tempNode->position.keys, inFile ))
					return 0;

				#ifdef	DEBUG_INFO
				printf( "keys: %d\n", tempNode->position.keys );
				#endif

				for (int i = 0; i < tempNode->position.keys; i++)
				{
					#ifdef	DEBUG_INFO
					printf( "data[%d]: <%3.2f, %3.2f, %3.2f>\n", i,	tempNode->position.data[i].pos.x,
					       						tempNode->position.data[i].pos.y,
					       						tempNode->position.data[i].pos.z );
					#endif
				}

				break;
			}

			default:
				#ifdef	DEBUG_INFO
				printf( "Skipping chunk 0x%04X\n", chunk.ID );
				#endif

				skipChunk(inFile, chunk.length);
				break;
		}
	}

	targetNodeCount++;

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readLightNode( FILE *inFile, int chunkLength )
{
	#ifdef	DEBUG_INFO
	printf( "LightNode\n" );
	#endif

	sLGTNODE	*tempNode = (sLGTNODE *) realloc( lightNodeList, sizeof( sLGTNODE ) * (lightNodeCount + 1) );
	
	if (!tempNode)
		return 0;

	lightNodeList = tempNode;

	tempNode = &lightNodeList[lightNodeCount];

	int	endPos = chunkLength - CHUNK_HEADER_SIZE + ftell(inFile);

	FOREVER
	{
		// Done?

		if (endPos <= ftell(inFile)) break;

		sCHK	chunk;
		int	retCode = getChunkID(inFile, &chunk);

		// Are we done?

		if (!retCode) break;

		switch(chunk.ID)
		{
		    	case CHUNK_KFNODEHDR:
				#ifdef	DEBUG_INFO
				printf( "KeyframeLightNodeHeader\n" );
				#endif

				if (!getString( inFile, tempNode->nodeHeader.name ))
					return 0;

				if (!myRead( &tempNode->nodeHeader.flags1, sizeof( tempNode->nodeHeader.flags1 ), inFile ))
					return 0;

				if (!myRead( &tempNode->nodeHeader.flags2, sizeof( tempNode->nodeHeader.flags2 ), inFile ))
					return 0;

				if (!myRead( &tempNode->nodeHeader.hierarchy, sizeof( tempNode->nodeHeader.hierarchy ), inFile ))
					return 0;

				break;

			case CHUNK_KFPOSTRACK:
			{
				#ifdef	DEBUG_INFO
				printf( "KeyframeLightPosition\n" );
				#endif

				if (!myRead( &tempNode->position, sizeof( tempNode->position ) - 4, inFile ))
					return 0;
				
				sPOS	*tempPos = (sPOS *) malloc( sizeof( sPOS ) * tempNode->position.keys );

				if (!tempPos)
					return 0;

				tempNode->position.data = tempPos;

				if (!myRead( tempNode->position.data, sizeof( sPOS ) * tempNode->position.keys, inFile ))
					return 0;

				#ifdef	DEBUG_INFO
				printf( "keys: %d\n", tempNode->position.keys );
				#endif

				for (int i = 0; i < tempNode->position.keys; i++)
				{
					#ifdef	DEBUG_INFO
					printf( "data[%d]: <%3.2f, %3.2f, %3.2f>\n", i,	tempNode->position.data[i].pos.x,
					       						tempNode->position.data[i].pos.y,
					       						tempNode->position.data[i].pos.z );
					#endif
				}

				break;
			}

			case CHUNK_KFCOLTRACK:
			{
				#ifdef	DEBUG_INFO
				printf( "KeyframeLightColor\n" );
				#endif

				if (!myRead( &tempNode->color, sizeof( tempNode->color ) - 4, inFile ))
					return 0;
				
				sCOL	*tempCol = (sCOL *) malloc( sizeof( sCOL ) * tempNode->color.keys );

				if (!tempCol)
					return 0;

				tempNode->color.data = tempCol;

				if (!myRead( tempNode->color.data, sizeof( sCOL ) * tempNode->color.keys, inFile ))
					return 0;

				#ifdef	DEBUG_INFO
				printf( "keys: %d\n", tempNode->color.keys );
				#endif

				break;
			}

			default:
				#ifdef	DEBUG_INFO
				printf( "Skipping chunk 0x%04X\n", chunk.ID );
				#endif

				skipChunk(inFile, chunk.length);
				break;
		}
	}

	lightNodeCount++;

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readMaterial(FILE *inFile, int chunkLength)
{
	#ifdef	DEBUG_INFO
	printf( "Material\n" );
	#endif

	matList = (sMAT *) realloc(matList, (matCount+1) * sizeof(sMAT));

	if (!matList)
	{
		throw std::string("Out of memory for materials list");
	}

	memset( &matList[matCount], 0, sizeof(sMAT) );

	int	endPos = chunkLength - CHUNK_HEADER_SIZE + ftell(inFile);

	FOREVER
	{
		// Done?

		if (endPos <= ftell(inFile)) break;

		sCHK	chunk;
		int	retCode = getChunkID(inFile, &chunk);

		// Make sure it's a sub-chunk...

		if ((chunk.ID >> 12 != 0xA && chunk.ID >> 12) || chunk.ID == CHUNK_MATERIAL)
		{
			backup(inFile);
			break;
		}

		// Are we done?

		if (!retCode) break;

		switch(chunk.ID)
		{
			case CHUNK_MATNAME:
				if (!readMaterialName(inFile, chunk.length))
				{
					throw std::string("Unable to read material name");
				}
				break;

			case CHUNK_MATTEXTURE:
				if (!readMaterialTexture(inFile, chunk.length))
				{
					throw std::string("Unable to read texture");
				}
				break;

			case CHUNK_MATAMBIENT:
				if (!readMaterialAmbient(inFile, chunk.length))
				{
					throw std::string("Unable to read material ambient");
				}
				break;

			case CHUNK_MATDIFFUSE:
				if (!readMaterialColor(inFile, chunk.length))
				{
					throw std::string("Unable to read material color");
				}
				break;

			case CHUNK_MATSHINE:
				if (!readMaterialShine(inFile, chunk.length))
				{
					throw std::string("Unable to read material shine");
				}
				break;

			case CHUNK_MATTRANS:
				if (!readMaterialTrans(inFile, chunk.length))
				{
					throw std::string("Unable to read material transparency");
				}
				break;

			default:
				#ifdef	DEBUG_INFO
				printf( "Skipping chunk 0x%04X\n", chunk.ID );
				#endif

				skipChunk(inFile, chunk.length);
				break;
		}
	}

	matCount++;

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readMaterialName(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "Material Name\n" );
	#endif

	return getString( inFile, matList[matCount].name );
}

// ----------------------------------------------------------------------------

int	C3DS::readMaterialAmbient(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "Material Ambient\n" );
	#endif

	char	tempStr[80];
	if (!myRead(tempStr, 6, inFile ))
		return 0;

	sRGB	ambient;
	if (!myRead(&ambient, sizeof(ambient), inFile ))
		return 0;

	matList[matCount].ambient = ambient;

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readMaterialColor(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "Material Color\n" );
	#endif

	char	tempStr[80];
	if (!myRead(tempStr, 6, inFile ))
		return 0;

	sRGB	color;
	if (!myRead(&color, sizeof(color), inFile ))
		return 0;

	matList[matCount].diffuse = color;

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readMaterialShine(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "Material Shine\n" );
	#endif

	char	tempStr[80];
	if (!myRead(tempStr, 6, inFile ))
		return 0;

	int	shine = 0;
	if (!myRead(&shine, 1, inFile ))
		return 0;

	if (!myRead(tempStr, 1, inFile ))
		return 0;

	matList[matCount].shininess = (float) (shine / 100.0);

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readMaterialTrans(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "Material Transparency\n" );
	#endif

	char	tempStr[80];
	if (!myRead(tempStr, 6, inFile ))
		return 0;

	int	transparency = 0;
	if (!myRead(&transparency, 1, inFile ))
		return 0;

	if (!myRead(tempStr, 1, inFile ))
		return 0;

	matList[matCount].transparent = (unsigned char) (transparency ? 1:0);

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readMaterialTexture(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "Material Texture\n" );
	#endif

	char	tempStr[80];

	if (!myRead(tempStr, 14, inFile ))
		return 0;

	if (!getString( inFile, tempStr ))
		return 0;

	tempStr[strcspn( tempStr, "." )] = '\0';

	strcpy(matList[matCount].texture, tempStr);

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readNamedObject(FILE *inFile, int chunkLength)
{
	int	endPos = chunkLength - CHUNK_HEADER_SIZE + ftell(inFile);

	if (!getString( inFile, currentNamedObject ))
		return 0;

	#ifdef	DEBUG_INFO
	printf( "Named Object:  %s\n", currentNamedObject );
	#endif

	FOREVER
	{
		// Done?

		if (endPos <= ftell(inFile)) break;

		sCHK	chunk;
		int	retCode = getChunkID(inFile, &chunk);

		// Make sure it's a sub-chunk...

		if (chunk.ID >> 12 != 4 || chunk.ID == CHUNK_NAMEDOBJECT )
		{
			backup(inFile);
			break;
		}

		// Are we done?

		if (!retCode) break;

		switch(chunk.ID)
		{
			case CHUNK_TRIMESH:
				if (!readTriMesh(inFile, chunk.length))
				{
					throw std::string("Unable to read tri mesh");
				}
				break;

			case CHUNK_LIGHT:
				if (!readLight(inFile, chunk.length))
				{
					throw std::string("Unable to read light");
				}
				break;

			case CHUNK_NOLIGHT:
				if (!readNoLight(inFile, chunk.length))
				{
					throw std::string("Unable to read 'no light' flag");
				}
				break;

			case CHUNK_CAMERA:
				if (!readCamera(inFile, chunk.length))
				{
					throw std::string("Unable to read camera");
				}
				break;

			default:
				#ifdef	DEBUG_INFO
				printf( "Skipping chunk 0x%04X\n", chunk.ID );
				#endif

				skipChunk(inFile, chunk.length);
				break;
		}
	}

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readTriMesh(FILE *inFile, int chunkLength)
{
	#ifdef	DEBUG_INFO
	printf( "TriMesh\n" );
	#endif

	meshList = (sMSH *) realloc( meshList, (meshCount+1) * sizeof(sMSH) );

	if (!meshList)
	{
		throw std::string("Out of memory for mesh list");
	}

	memset( &meshList[meshCount], 0, sizeof(sMSH) );

	strcpy(meshList[meshCount].name, currentNamedObject );

	int	endPos = chunkLength - CHUNK_HEADER_SIZE + ftell(inFile);

	FOREVER
	{
		// Done?

		if (endPos <= ftell(inFile)) break;

		sCHK	chunk;
		int	retCode = getChunkID(inFile, &chunk);

		// Make sure it's a sub-chunk...

		if (chunk.ID >> 8 != 0x41 || chunk.ID == CHUNK_TRIMESH )
		{
			backup(inFile);
			break;
		}

		// Are we done?

		if (!retCode) break;

		switch(chunk.ID)
		{
			case CHUNK_VERTEXLIST:
				if (!readVertexList(inFile, chunk.length))
				{
					throw std::string("Unable to read vertex list");
				}
				break;

			case CHUNK_FACELIST:
				if (!readFaceList(inFile, chunk.length))
				{
					throw std::string("Unable to read face list");
				}
				break;

//			case CHUNK_MATERIALAPP:
//				if (!readMaterialApp(inFile, chunk.length))
//				{
//					throw std::string("Unable to read material application");
//				}
//				break;

			case CHUNK_UVLIST:
				if (!readUVList(inFile, chunk.length))
				{
					throw std::string("Unable to read UV list");
				}
				break;

			default:
				#ifdef	DEBUG_INFO
				printf( "Skipping chunk 0x%04X\n", chunk.ID );
				#endif

				skipChunk(inFile, chunk.length);
				break;
		}
	}

	meshCount++;
	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readVertexList(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "vList\n" );
	#endif

	if (!myRead(&meshList[meshCount].vCount, sizeof(meshList[meshCount].vCount), inFile ))
	{
		throw std::string("Unable to read vertex list");
	}

	meshList[meshCount].vList = (sP3D *) malloc(sizeof(sP3D) * meshList[meshCount].vCount);

	if (!meshList[meshCount].vList)
	{
		throw std::string("Unable to allocate RAM for the vertex list");
	}

	if (!myRead( meshList[meshCount].vList, sizeof(sP3D) * meshList[meshCount].vCount, inFile ))
	{
		throw std::string("Unable to read vertex list");
	}

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readFaceList(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "FaceList\n" );
	#endif

	if (!myRead(&meshList[meshCount].fCount, sizeof(meshList[meshCount].fCount), inFile ))
	{
		throw std::string("Unable to read face count");
	}

	meshList[meshCount].fList = (sFACE *) malloc(sizeof(sFACE) * meshList[meshCount].fCount);

	if (!meshList[meshCount].fList)
	{
		throw std::string("Unable to allocate RAM for the face list");
	}

	memset( meshList[meshCount].fList, 0, sizeof(sFACE) * meshList[meshCount].fCount );

	for( int i = 0; i < meshList[meshCount].fCount; i++)
	{
		meshList[meshCount].fList[i].materialIndex = 0;

		if (!myRead( &meshList[meshCount].fList[i].a, sizeof(unsigned short), inFile ))
		{
			throw std::string("Unable to read vertex index");
		}

		if (!myRead( &meshList[meshCount].fList[i].b, sizeof(unsigned short), inFile ))
		{
			throw std::string("Unable to read vertex index");
		}

		if (!myRead( &meshList[meshCount].fList[i].c, sizeof(unsigned short), inFile ))
		{
			throw std::string("Unable to read vertex index");
		}

		if (!myRead( &meshList[meshCount].fList[i].flags, sizeof(unsigned short), inFile ))
		{
			throw std::string("Unable to read flags");
		}
	}

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readMaterialApp(FILE *inFile, int chunkLength)
{
	#ifdef	DEBUG_INFO
	printf( "MaterialApp\n" );
	#endif

	char	name[DEF_STR_LEN+1];

	if (!getString( inFile, name ))
	{
		throw std::string("Unable to read material application name");
	}

	// Unknown word...

	unsigned short	faceNumber;

	if (!myRead( &faceNumber, sizeof(faceNumber), inFile ))
	{
		throw std::string("Unable to read material application face number");
	}

	// Find the material index for this name

	short	index = -1;

	for(int i = 0; i < matCount; i++)
	{
		if (!stricmp(name, matList[i].name))
			index = (unsigned short) i;
	}

	if (index == -1)
	{
		throw std::string("Unable to find material in list");
	}

	int	count = (chunkLength - CHUNK_HEADER_SIZE - (int) strlen(name) - 3) / 2;

	while( count-- )
	{
		if (!myRead( &faceNumber, sizeof(faceNumber), inFile ))
		{
			throw std::string("Unable to read material application face number");
		}

		if (faceNumber > meshList[meshCount].fCount)
		{
			throw std::string("Material application face number out of range");
		}

		meshList[meshCount].fList[faceNumber].materialIndex = (unsigned short) (index + 1);
	}

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readUVList(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "uvList\n" );
	#endif

	if (!myRead(&meshList[meshCount].uvCount, sizeof(meshList[meshCount].uvCount), inFile ))
	{
		throw std::string("Unable to read material application UV count");
	}

	meshList[meshCount].uvList = (sP2D *) malloc(sizeof(sP2D) * meshList[meshCount].uvCount);

	if (!meshList[meshCount].uvList)
	{
		throw std::string("Unable to allocate RAM for the UV list");
	}

	memset( meshList[meshCount].uvList, 0, sizeof(sP2D) * meshList[meshCount].uvCount );

	if (!myRead( meshList[meshCount].uvList, sizeof(sP2D) * meshList[meshCount].uvCount, inFile ))
	{
		throw std::string("Unable to read material application UV list");
	}

	float	*fTemp;

	for( int i = 0; i < meshList[meshCount].uvCount; i++ )
	{
		fTemp = &meshList[meshCount].uvList[i].x;
		if (*fTemp > (float) 99.9 || *fTemp < (float) -99.0) *fTemp = (float) 0.0;
		fTemp = &meshList[meshCount].uvList[i].y;
		if (*fTemp > (float) 99.9 || *fTemp < (float) -99.0) *fTemp = (float) 0.0;
	}

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readLight(FILE *inFile, int chunkLength)
{
	lightList = (sLGT *) realloc( lightList, (lightCount+1) * sizeof(sLGT) );

	if (!lightList)
	{
		throw std::string("Out of memory for light list");
	}

	memset( &lightList[lightCount], 0, sizeof(sLGT) );

	strcpy(lightList[lightCount].name, currentNamedObject );
	lightList[lightCount].innerRange = 1.0f;
	lightList[lightCount].outerRange = 100.0f;
	lightList[lightCount].intensity  = 1.0f;

	int	endPos = chunkLength - CHUNK_HEADER_SIZE + ftell(inFile);

	#ifdef	DEBUG_INFO
	printf( "Light\n" );
	#endif

	if (!myRead(&lightList[lightCount].loc, sizeof(sP3D), inFile ))
	{
		throw std::string("Unable to read light location");
	}

	FOREVER
	{
		// Done?

		if (endPos <= ftell(inFile)) break;

		sCHK	chunk;
		int	retCode = getChunkID(inFile, &chunk);

		// Make sure it's a sub-chunk...

		if ((chunk.ID >> 8 != 0x46 && chunk.ID >> 8 != 0x00) || chunk.ID == CHUNK_LIGHT )
		{
			backup(inFile);
			break;
		}

		// Are we done?

		if (!retCode) break;

		switch(chunk.ID)
		{
			case CHUNK_RGB:
				if (!readLightRGB(inFile, chunk.length))
				{
					throw std::string("Unable to read light RGB");
				}
				break;

			case CHUNK_24BIT:
				if (!readLight24Bit(inFile, chunk.length))
				{
					throw std::string("Unable to read light 24B");
				}
				break;

			case CHUNK_SPOTLIGHT:
				if (!readSpotLight(inFile, chunk.length))
				{
					throw std::string("Unable to read spot light");
				}
				break;

			case CHUNK_NOLIGHT:
				if (!readNoLight(inFile, chunk.length))
				{
					throw std::string("Unable to read 'no light' flag");
				}
				break;

			case CHUNK_INNER_RANGE:
				if (!readInnerRange(inFile, chunk.length))
				{
					throw std::string("Unable to read inner range");
				}
				break;

			case CHUNK_OUTER_RANGE:
				if (!readOuterRange(inFile, chunk.length))
				{
					throw std::string("Unable to read outer range");
				}
				break;

			case CHUNK_MULTIPLIER:
				if (!readMultiplier(inFile, chunk.length))
				{
					throw std::string("Unable to read multiplier");
				}
				break;

			default:
				#ifdef	DEBUG_INFO
				printf( "Skipping chunk 0x%04X\n", chunk.ID );
				#endif

				skipChunk(inFile, chunk.length);
				break;
		}
	}

	lightCount++;
	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readLightRGB(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "RGB\n" );
	#endif

	float	r, g, b;

	if (!myRead(&r, sizeof(float), inFile ))
	{
		throw std::string("Unable to read light RGB");
	}
	if (!myRead(&g, sizeof(float), inFile ))
	{
		throw std::string("Unable to read light RGB");
	}
	if (!myRead(&b, sizeof(float), inFile ))
	{
		throw std::string("Unable to read light RGB");
	}

	lightList[lightCount].rgb.r = r * 255;
	lightList[lightCount].rgb.g = g * 255;
	lightList[lightCount].rgb.b = b * 255;
	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readLight24Bit(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "24-Bit\n" );
	#endif

	sRGB	color;

	if (!myRead(&color, sizeof(color), inFile ))
	{
		throw std::string("Unable to read light 24B");
	}

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readSpotLight(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "SpotLight\n" );
	#endif

	lightList[lightCount].spotLight = 1;

	if (!myRead(&lightList[lightCount].target, sizeof(sPoint3), inFile))
	{
		throw std::string("Unable to read spotLight target");
	}

	if (!myRead(&lightList[lightCount].hotSpot, sizeof(lightList[lightCount].hotSpot), inFile))
	{
		throw std::string("Unable to read spotLight hotspot");
	}

	if (!myRead(&lightList[lightCount].fallOff, sizeof(lightList[lightCount].fallOff), inFile))
	{
		throw std::string("Unable to read spotLight fallOff");
	}

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readNoLight(FILE *inFile, int chunkLength)
{
	#ifdef	DEBUG_INFO
	printf( "NoLight\n" );
	#endif

	lightList[lightCount].noLight = 1;

	// This one's unused...

	skipChunk(inFile, chunkLength);

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readInnerRange(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "inner range\n" );
	#endif

	if (!myRead(&lightList[lightCount].innerRange, sizeof(lightList[lightCount].innerRange), inFile ))
	{
		throw std::string("Unable to inner range");
	}

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readOuterRange(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "outer range\n" );
	#endif

	if (!myRead(&lightList[lightCount].outerRange, sizeof(lightList[lightCount].outerRange), inFile ))
	{
		throw std::string("Unable to outer range");
	}

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readMultiplier(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "multiplier\n" );
	#endif

	if (!myRead(&lightList[lightCount].intensity, sizeof(lightList[lightCount].intensity), inFile ))
	{
		throw std::string("Unable to intensity");
	}

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::readCamera(FILE *inFile, int chunkLength)
{
	chunkLength = chunkLength;

	#ifdef	DEBUG_INFO
	printf( "Camera\n" );
	#endif

	camList = (sCAM *) realloc( camList, (camCount+1) * sizeof(sCAM) );

	if (!camList)
	{
		throw std::string("Unable to allocate RAM for camera list");
	}

	if (!myRead(&camList[camCount].loc, sizeof(sP3D), inFile ))
	{
		throw std::string("Unable to read camera location");
	}

	if (!myRead(&camList[camCount].target, sizeof(sP3D), inFile ))
	{
		throw std::string("Unable to read camera target");
	}

	if (!myRead(&camList[camCount].bank, sizeof(float), inFile ))
	{
		throw std::string("Unable to read camera bank");
	}

	if (!myRead(&camList[camCount].lens, sizeof(float), inFile ))
	{
		throw std::string("Unable to read camera lens");
	}

	strcpy(camList[camCount].name, currentNamedObject );

	camCount++;
	return 1;
}

// ----------------------------------------------------------------------------

void	C3DS::skipChunk(FILE *File, unsigned long length)
{
	fseek( File, length + ftell(File) - CHUNK_HEADER_SIZE, SEEK_SET );
}

// ----------------------------------------------------------------------------

void	C3DS::backup(FILE *File)
{
	fseek( File, ftell(File) - CHUNK_HEADER_SIZE, SEEK_SET );
}

// ----------------------------------------------------------------------------

int	C3DS::myRead( void *buffer, int length, FILE *file)
{
	int	Count = (int) fread(buffer, length, 1, file);

	if (Count == 0 && feof(file))
		return 0;

	if (Count != 1)
	{
		throw std::string("Unable to read from input file");
	}

	return 1;
}

// ----------------------------------------------------------------------------

int	C3DS::getString( FILE *inFile, char *string )
{
	for( int i = 0; i < DEF_STR_LEN; i++)
	{
		int	chr = fgetc(inFile);

		if (chr == ' ')
			chr = '_';

		string[i] = (char) chr;

		if (!chr)
			return 1;
	}

	return 0;
}
