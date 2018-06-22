#include <stdio.h>
#include <stdlib.h> // calloc
#include <string>
#include <unistd.h>

#include "mesh.h"
#include "meshseed.h"

//#include "voxelcuboid.h"
#include "voxelfilter25d.h"

#include "image2d.h"

#include <sys/stat.h> // statistics for files

#define _DEFAULT_FEATUREGEN_RADIUS_       1.0
#define _DEFAULT_FEATUREGEN_XYZDIM_       256
#define _DEFAULT_FEATUREGEN_RADIICOUNT_   4 // power of 2

//*****************************#ifdef THREADS
// Multithreading (CPU):
// see: https://computing.llnl.gov/tutorials/pthreads/
#include <pthread.h>

#ifndef NUM_THREADS
#define NUM_THREADS           7
#endif
#define THREADS_VERTEX_BLOCK  5000
pthread_mutex_t mutexVertexPtr;

struct meshDataStruct {
	int     threadID; //!< ID of the posix thread
	// input:
	Mesh*   meshToAnalyze;
	float   radius;
	uint    xyzDim;
	uint    multiscaleRadiiSize;
	double* multiscaleRadii;
	int*    vertexOriIdxInProgress;
	int     nrOfVerticesToProcess;
	// output:
	int     ctrIgnored;
	int     ctrProcessed;
	// our most precious feature vectors (as array):
	double* patchNormal;     //!< Normal used for orientation into 2.5D representation
	double* descriptVolume;  //!< Volume descriptors
	double* descriptSurface; //!< Surface descriptors
	// and the original vertex indicies
	int*    featureIndicies;
	voxelFilter2DElements** sparseFilters;
};

void* estFeatureVectors( void* setMeshData ) {
	// as we can only pass a void pointer, we have to cast at first:
	meshDataStruct* meshData = (meshDataStruct*) setMeshData;
	int threadID = meshData->threadID+1;
	cout << "[GigaMesh] Thread " << threadID << " | START one out of " << NUM_THREADS << " threads." << endl;

	// initalize values to be returned via meshDataStruct:
	meshData->ctrIgnored   = 0;
	meshData->ctrProcessed = 0;

	// copy pointers from struct for easier access.
	double* tDescriptVolume  = meshData->descriptVolume;  //!< Volume descriptors
	double* tDescriptSurface = meshData->descriptSurface; //!< Surface descriptors

	// setup memory for rastered surface:
	uint    rasterSize = meshData->xyzDim * meshData->xyzDim;
	double* rasterArray = new double[rasterSize];
	int     nrOfVerticesToProcess = meshData->nrOfVerticesToProcess;

	time_t timeStart = time( NULL );

	// Allocate the bit arrays for vertices:
	unsigned long* vertBitArrayVisited;
	int vertNrLongs = meshData->meshToAnalyze->getBitArrayVerts( &vertBitArrayVisited );
	// Allocate the bit arrays for faces:
	unsigned long* faceBitArrayVisited;
	int faceNrLongs = meshData->meshToAnalyze->getBitArrayFaces( &faceBitArrayVisited );

	// Compute absolut radii (used to normalize the surface descriptor)
	double* absolutRadii = new double[meshData->multiscaleRadiiSize];
	for( uint i=0; i<meshData->multiscaleRadiiSize; i++ ) {
		absolutRadii[i] = (double) meshData->multiscaleRadii[i] * (double) meshData->radius;
	}

	// Step thru vertices:
	bool          endOfVertexList = false;
	int           vertexOriIdxInProgress;
	Vertex*       currentVertex;
	vector<Face*> facesInSphere; // temp variable to store local surface patches - returned by fetchSphereMarching
	while( !endOfVertexList ) {
		pthread_mutex_lock( &mutexVertexPtr );
		vertexOriIdxInProgress = (*meshData->vertexOriIdxInProgress);
		(*meshData->vertexOriIdxInProgress) += THREADS_VERTEX_BLOCK;
		pthread_mutex_unlock( &mutexVertexPtr );
		for( int i=0; (i<THREADS_VERTEX_BLOCK)&&!endOfVertexList; i++ ) {
			if( vertexOriIdxInProgress >= nrOfVerticesToProcess ) {
				endOfVertexList = true;
				continue;
			}
			currentVertex = meshData->meshToAnalyze->getVertexPos( vertexOriIdxInProgress );

			// Fetch faces within the largest sphere
			facesInSphere.clear();
			// slower for larger patches:
			//meshData->meshToAnalyze->fetchSphereMarching( currentVertex, &facesInSphere, meshData->radius, true );
			//meshData->meshToAnalyze->fetchSphereMarchingDualFront( currentVertex, &facesInSphere, meshData->radius, true );
			//meshData->meshToAnalyze->fetchSphereBitArray( currentVertex, &facesInSphere, meshData->radius, vertNrLongs, vertBitArrayVisited, faceNrLongs, faceBitArrayVisited );
			meshData->meshToAnalyze->fetchSphereBitArray1R( currentVertex, &facesInSphere, meshData->radius, vertNrLongs, vertBitArrayVisited, faceNrLongs, faceBitArrayVisited );

			// Fetch and store the normal used in fetchSphereCubeVolume25D as it is a quality measure
			vector<Face*>::iterator itFace;
			for( itFace=facesInSphere.begin(); itFace!=facesInSphere.end(); itFace++ ) {
				(*itFace)->addNormalTo( &(meshData->patchNormal[vertexOriIdxInProgress*3]) );
			}

			// Pre-compute address offset
			unsigned int descriptIndexOffset = vertexOriIdxInProgress*meshData->multiscaleRadiiSize;

			// Get volume descriptor:
			if( tDescriptVolume != NULL ) {
				meshData->meshToAnalyze->fetchSphereCubeVolume25D( currentVertex, &facesInSphere, meshData->radius, rasterArray, meshData->xyzDim );
				applyVoxelFilters2D( &(tDescriptVolume[descriptIndexOffset]), rasterArray, meshData->sparseFilters, meshData->multiscaleRadiiSize, meshData->xyzDim );
			}

			// Get surface descriptor:
			if( tDescriptSurface != NULL ) {
				Vector3D seedPosition = currentVertex->getCenterOfGravity();
				meshData->meshToAnalyze->fetchSphereArea( currentVertex, &facesInSphere, (unsigned int)meshData->multiscaleRadiiSize, absolutRadii, &(tDescriptSurface[descriptIndexOffset]) );
			}
			// Set counters:
			meshData->ctrProcessed++;
			vertexOriIdxInProgress++;
		}
		if( !endOfVertexList ) {
			// Show a time estimation:
			int   timeElapsed = time( NULL ) - timeStart;
			float percentDone = (float) vertexOriIdxInProgress / (float) nrOfVerticesToProcess;
			int   timeRemaining = round( ( (float) timeElapsed / vertexOriIdxInProgress ) * ( nrOfVerticesToProcess - vertexOriIdxInProgress ) );
			cout << "[GigaMesh] Thread " << threadID << " | " << percentDone*100 << " percent done. Time elapsed: " << timeElapsed << " - remaining: " << timeRemaining << " seconds. ";
			cout << vertexOriIdxInProgress/timeElapsed << " Vert/sec.";
			time_t timeDone = timeRemaining + (int)time( NULL );
			cout << " ETF: " << asctime( localtime( &timeDone ) );
			cout << endl;
		}
	}
	// Volume descriptor
	delete rasterArray;
	delete vertBitArrayVisited;
	delete faceBitArrayVisited;
	// Surface descriptor
	delete absolutRadii;

	cout << "[GigaMesh] Thread " << threadID << " | STOP - processed: " << meshData->ctrProcessed << " and skipped " << meshData->ctrIgnored << " vertices." << endl;
	pthread_exit( NULL );
}
//**********************#endif



//! Main routine for generating an array of feature vectors
//!
//! Remark: prefer MeshSeed over Mesh as it is faster by a factor of 3+
//==========================================================================
int main( int argc, char *argv[] ) {
	string       fileNameIn;
	string       fileNameOut;
	double       radius = _DEFAULT_FEATUREGEN_RADIUS_;
	unsigned int xyzDim = _DEFAULT_FEATUREGEN_XYZDIM_;
	unsigned int radiiCount = _DEFAULT_FEATUREGEN_RADIICOUNT_;
	bool         replaceFiles = false;
	bool         areaOnly = false;

	// PARSE command line options
	//--------------------------------------------------------------------------
	opterr = 0;
	int c;
	int tmpInt;
	bool radiusSet = false;
	while( ( c = getopt( argc, argv, "f:o:r:v:n:ka" ) ) != -1 ) {
		switch( c ) {
			//! Option f: filename for input data (required)
			case 'f':
				fileNameIn = optarg;
				break;
			//! Option o: prefix for files with filter results and metadata (optional/automatic)
			case 'o':
				fileNameOut = optarg;
				break;
			//! Option r: absolut radius (in units, default: 1.0)
			case 'r':
				radius = atof( optarg );
				radiusSet = true;
				break;
			//! Option v: edge length of the voxel cube (in voxels, default: 256)
			case 'v':
				tmpInt = atof( optarg );
				if( tmpInt <= 0 ) {
					cerr << "[GigaMesh] Error: negative or zero value given: " << tmpInt << " for the number of voxels (option -v)!" << endl;
					exit( EXIT_FAILURE );
				}
				xyzDim = (unsigned int)tmpInt;
				break;
			//! Option n: 2^n scales (default: 4 => 16 scales)
			case 'n':
				tmpInt = atof( optarg );
				if( tmpInt < 0 ) {
					cerr << "[GigaMesh] Error: negative value given: " << tmpInt << " for the number of radii (option -n)!" << endl;
					exit( EXIT_FAILURE );
				}
				radiiCount = (unsigned int)tmpInt;
				break;
			//! Option k: replaces output files
			case 'k':
				cout << "[GigaMesh] Warning: files might be replaced!" << endl;
				replaceFiles = true;
				break;
			//! Option a: compute area/surface based integral onyl
			case 'a':
				cout << "[GigaMesh] Warning: Only area integrals will be computed!" << endl;
				areaOnly = true;
				break;
			case '?':
				cerr << "[GigaMesh] Error: Unknown option!" << endl;
				break;
			default:
				cerr << "[GigaMesh] Error: Unknown option '" << c << "'!" << endl;
				exit( EXIT_FAILURE );
		}
	}
	// Check argument ranges
	if( radius <= 0.0 ) {
		cerr << "[GigaMesh] Error: negative or zero radius given: " << radius << " (option -r)!" << endl;
		exit( EXIT_FAILURE );
	}
	if( !radiusSet ) {
		cout << "[GigaMesh] Warning: default radius is used (option -r missing)!" << endl;
	}
	if( fileNameIn.length() == 0 ) {
		cerr << "[GigaMesh] Error: no filename for input given (option -f)!" << endl;
		exit( EXIT_FAILURE );
	}
	// Check file extension for input file
	size_t foundDot = fileNameIn.rfind( "." );
	if( foundDot == string::npos ) {
		cerr << "[GigaMesh] Error: No extension/type for input file '" << fileNameIn << "' specified!" << endl;
		exit( EXIT_FAILURE );
	}
	// Check fileprefix for output - when not given use the name of the input file
	if( fileNameOut.length() == 0 ) {
		fileNameOut = fileNameIn.substr( 0, foundDot );
		// Warning message see a few lines below.
	}
	// Add parameters to output prefix
	char tmpBuffer[512];
	sprintf( tmpBuffer, "_r%0.2f_n%i_v%i", radius, radiiCount, xyzDim );
	fileNameOut += string( tmpBuffer );
	// Warning message, for option -o missing
	if( fileNameOut.length() == 0 ) {
		cerr << "[GigaMesh] Warning: no prefix for output given (option -o) using: '" << fileNameOut << "'!" << endl;
	}
	// Check files using file statistics
	struct stat stFileInfo;
	// Check: Input file exists?
	if( stat( fileNameIn.c_str(), &stFileInfo ) != 0 ) {
		cerr << "[GigaMesh] Error: File '" << fileNameIn << "' not found!" << endl;
		exit( EXIT_FAILURE );
	}
	// Check: Output file for normal used to rotate the local patch
	string fileNameOutPatchNormal( fileNameOut );
	fileNameOutPatchNormal += ".normal.mat";
	if( stat( fileNameOutPatchNormal.c_str(), &stFileInfo ) == 0 ) {
		if( !replaceFiles ) {
			cerr << "[GigaMesh] File '" << fileNameOutPatchNormal << "' already exists!" << endl;
			exit( EXIT_FAILURE );
		}
		cerr << "[GigaMesh] Warning: File '" << fileNameOutPatchNormal << "' will be replaced!" << endl;
	}
	string fileNameOutVS;
	string fileNameOutVol;
	if( !areaOnly ) {
		// Check: Output file for volume AND surface descriptor
		fileNameOutVS = fileNameOut;
		fileNameOutVS += ".vs.mat";
		if( stat( fileNameOutVS.c_str(), &stFileInfo ) == 0 ) {
			if( !replaceFiles ) {
				cerr << "[GigaMesh] File '" << fileNameOutVS << "' already exists!" << endl;
				exit( EXIT_FAILURE );
			}
			cerr << "[GigaMesh] Warning: File '" << fileNameOutVS << "' will be replaced!" << endl;
		}
		// Check: Output file for volume descriptor
		fileNameOutVol = fileNameOut;
		fileNameOutVol += ".volume.mat";
		if( stat( fileNameOutVol.c_str(), &stFileInfo ) == 0 ) {
			if( !replaceFiles ) {
				cerr << "[GigaMesh] File '" << fileNameOutVol << "' already exists!" << endl;
				exit( EXIT_FAILURE );
			}
			cerr << "[GigaMesh] Warning: File '" << fileNameOutVol << "' will be replaced!" << endl;
		}
	}
	// Output file for surface descriptor
	string fileNameOutSurf( fileNameOut );
	fileNameOutSurf += ".surface.mat";
	if( stat( fileNameOutSurf.c_str(), &stFileInfo ) == 0 ) {
		if( !replaceFiles ) {
			cerr << "[GigaMesh] File '" << fileNameOutSurf << "' already exists!" << endl;
			exit( EXIT_FAILURE );
		}
		cerr << "[GigaMesh] Warning: File '" << fileNameOutSurf << "' will be replaced!" << endl;
	}
	// Output file for meta-information
	string fileNameOutMeta( fileNameOut );
	fileNameOutMeta += ".info.txt";
	if( stat( fileNameOutMeta.c_str(), &stFileInfo ) == 0 ) {
		if( !replaceFiles ) {
			cerr << "[GigaMesh] File '" << fileNameOutMeta << "' already exists!" << endl;
			exit( EXIT_FAILURE );
		}
		cerr << "[GigaMesh] Warning: File '" << fileNameOutMeta << "' will be replaced!" << endl;
	}
	// Output file for 3D data including the volumetric feature vectors.
	string fileNameOut3D( fileNameOut );
	fileNameOut3D += ".ply";
	if( stat( fileNameOut3D.c_str(), &stFileInfo ) == 0 ) {
		if( !replaceFiles ) {
			cerr << "[GigaMesh] File '" << fileNameOut3D << "' already exists!" << endl;
			exit( EXIT_FAILURE );
		}
		cerr << "[GigaMesh] Warning: File '" << fileNameOut3D << "' will be replaced!" << endl;
	}
	// Invalid/unexpected paramters
	for( int i = optind; i < argc; i++ ) {
		cerr << "[GigaMesh] Warning: Non-option argument '" << argv[i] << "' given and ignored!" << endl;
	}
	// All parameters OK => infos to stdout and file with metadata  -----------------------------------------------------------
	fstream fileStrOutMeta;
	fileStrOutMeta.open( fileNameOutMeta.c_str(), fstream::out );
	cout << "[GigaMesh] File IN:         " << fileNameIn << endl;
	cout << "[GigaMesh] File OUT/Prefix: " << fileNameOut << endl;
	cout << "[GigaMesh] Radius:          " << radius << " mm (unit assumed)" << endl;
	cout << "[GigaMesh] Radii:           2^" << radiiCount << " = " << pow( (float)2.0, (float)radiiCount ) << endl;
	cout << "[GigaMesh] Rastersize:      " << xyzDim << "^3" << endl;
	fileStrOutMeta << "File IN:         " << fileNameIn << endl;
	fileStrOutMeta << "File OUT/Prefix: " << fileNameOut << endl;
	fileStrOutMeta << "Radius:          " << radius << " mm (unit assumed)" << endl;
	fileStrOutMeta << "Radii:           2^" << radiiCount << " = " << pow( (float)2.0, (float)radiiCount ) << endl;
	fileStrOutMeta << "Rastersize:      " << xyzDim << "^3" << endl;

	// Compute relative radii:
	unsigned int multiscaleRadiiSize = pow( (float)2.0, (float)radiiCount );
	double*      multiscaleRadii     = new double[multiscaleRadiiSize];
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		multiscaleRadii[i] = 1.0 - (double)i/(double)multiscaleRadiiSize;
	}

	// Set the formatting properties of the output
	cout << setprecision( 2 ) << fixed;
	fileStrOutMeta << setprecision( 2 ) << fixed;
	// Info about files
	cout << "[GigaMesh] File to write patch normal: " << fileNameOutPatchNormal << endl;
	if( !fileNameOutVS.empty() ) {
		cout << "[GigaMesh] File to write V+S:          " << fileNameOutVS << endl;
	}
	if( !fileNameOutVol.empty() ) {
		cout << "[GigaMesh] File to write Volume:       " << fileNameOutVol << endl;
	}
	if( !fileNameOutSurf.empty() ) {
		cout << "[GigaMesh] File to write Surface:      " << fileNameOutSurf << endl;
	}
	cout << "[GigaMesh] File to write Metainfo:     " << fileNameOutMeta << endl;
	cout << "[GigaMesh] Radii:                     ";
	fileStrOutMeta << "Radii:          ";
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		cout << " " << multiscaleRadii[i];
		fileStrOutMeta << " " << multiscaleRadii[i];
	}
	cout << endl;
	fileStrOutMeta << endl;

	// Prepare data structures
	//--------------------------------------------------------------------------
	Mesh someMesh;
	if( !someMesh.readFile( fileNameIn ) ) {
		cerr << "[GigaMesh] Error: Could not open file '" << fileNameIn << "'!" << endl;
		exit( EXIT_FAILURE );
	}
	someMesh.establishStructure();

	// Fetch mesh data
	Vector3D bbDim;
	someMesh.getBoundingBoxSize( bbDim );
	double bbWdith  = (double)round( bbDim.getX()*1.0 ) / 10.0;
	double bbHeight = (double)round( bbDim.getY()*1.0 ) / 10.0;
	double bbThick  = (double)round( bbDim.getZ()*1.0 ) / 10.0;
	// Area and average resolution
	double areaAcq;
	someMesh.getFaceSurfSum( &areaAcq );
	areaAcq = round( areaAcq );
	double volDXYZ[3];
	someMesh.estimateVolumeDivergence( volDXYZ );
	string modelID = someMesh.getModelID();
	string modelMat = someMesh.getModelMaterial();
	// Write data to file
	cout << "[GigaMesh] Model ID:        " << modelID << endl;
	cout << "[GigaMesh] Material:        " << modelMat << endl;
	cout << "[GigaMesh] Vertices:        " << someMesh.getVertexNr() << "" << endl;
	cout << "[GigaMesh] Faces:           " << someMesh.getFaceNr() << "" << endl;
	cout << "[GigaMesh] Bounding Box:    " << bbWdith << " x " << bbHeight << " x " << bbThick << " cm" << endl;
	cout << "[GigaMesh] Area:            " << areaAcq/100.0 << " cm^2" << endl;
	cout << "[GigaMesh] Volume (dx):     " << volDXYZ[0]/1000.0 << " cm^3" << endl;
	cout << "[GigaMesh] Volume (dy):     " << volDXYZ[1]/1000.0 << " cm^3" << endl;
	cout << "[GigaMesh] Volume (dz):     " << volDXYZ[2]/1000.0 << " cm^3" << endl;
	fileStrOutMeta << "Model ID:        " << modelID << endl;
	fileStrOutMeta << "Material:        " << modelMat << endl;
	fileStrOutMeta << "Vertices:        " << someMesh.getVertexNr() << "" << endl;
	fileStrOutMeta << "Faces:           " << someMesh.getFaceNr() << "" << endl;
	fileStrOutMeta << "Bounding Box:    " << bbWdith << " x " << bbHeight << " x " << bbThick << " cm" << endl;
	fileStrOutMeta << "Area:            " << areaAcq/100.0 << " cm^2" << endl;
	fileStrOutMeta << "Volume (dx):     " << volDXYZ[0]/1000.0 << " cm^3" << endl;
	fileStrOutMeta << "Volume (dy):     " << volDXYZ[1]/1000.0 << " cm^3" << endl;
	fileStrOutMeta << "Volume (dz):     " << volDXYZ[2]/1000.0 << " cm^3" << endl;

	int*    featureIndicies = new int[someMesh.getVertexNr()];
	double* patchNormal     = new double[someMesh.getVertexNr()*3];
	double* descriptVolume  = NULL;
	if( !areaOnly ) {
		descriptVolume  = new double[someMesh.getVertexNr()*multiscaleRadiiSize];
	}
	double* descriptSurface = NULL;
	descriptSurface = new double[someMesh.getVertexNr()*multiscaleRadiiSize];
	for( unsigned long i=0; i<someMesh.getVertexNr(); i++ ) {
		featureIndicies[i] = i;
		patchNormal[i*3]   = 0.0;
		patchNormal[i*3+1] = 0.0;
		patchNormal[i*3+2] = 0.0;
	}
	if( descriptVolume != NULL ) {
		for( uint i=0; i<someMesh.getVertexNr()*multiscaleRadiiSize; i++ ) {
			descriptVolume[i]  = _NOT_A_NUMBER_DBL_;
		}
	}
	if( descriptSurface != NULL ) {
		for( uint i=0; i<someMesh.getVertexNr()*multiscaleRadiiSize; i++ ) {
			descriptSurface[i] = _NOT_A_NUMBER_DBL_;
		}
	}

	time_t     rawtime;
	struct tm* timeinfo;
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	cout << "[GigaMesh] Start date/time is: " << asctime( timeinfo );// << endl;
	fileStrOutMeta << "Start date/time is: " << asctime( timeinfo ); // no endl required as asctime will add a linebreak

//***************************#ifdef THREADS
	time_t timeStampParallel = time( NULL ); // clock() is not multi-threading save (to measure the non-CPU or real time ;) )
	voxelFilter2DElements* sparseFilters;
	generateVoxelFilters2D( multiscaleRadiiSize, multiscaleRadii, xyzDim, &sparseFilters );

	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;

	/* Initialize and set thread detached attribute */
	pthread_mutex_init( &mutexVertexPtr, NULL );
	pthread_attr_init( &attr );
	pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );

	int rc;
	long t;
	void *status;

	int vertexOriIdxInProgress = 0;
	meshDataStruct setMeshData[NUM_THREADS];
	for( t=0; t<NUM_THREADS; t++ ) {
		//cout << "[GigaMesh] Preparing data for thread " << t << endl;
		setMeshData[t].threadID               = t;
		setMeshData[t].meshToAnalyze          = &someMesh;
		setMeshData[t].radius                 = radius;
		setMeshData[t].xyzDim                 = xyzDim;
		setMeshData[t].multiscaleRadiiSize    = multiscaleRadiiSize;
		setMeshData[t].multiscaleRadii        = multiscaleRadii;
		setMeshData[t].sparseFilters          = &sparseFilters;
		setMeshData[t].vertexOriIdxInProgress = &vertexOriIdxInProgress;
		setMeshData[t].nrOfVerticesToProcess  = someMesh.getVertexNr();
		setMeshData[t].featureIndicies        = featureIndicies;
		setMeshData[t].patchNormal            = patchNormal;
		setMeshData[t].descriptVolume         = descriptVolume;
		setMeshData[t].descriptSurface        = descriptSurface;
	}

	for( t=0; t<NUM_THREADS; t++ ) {
		//cout << "[GigaMesh] Creating thread " << t << endl;
		rc = pthread_create( &threads[t], &attr, estFeatureVectors, (void *)&setMeshData[t] );
		if( rc ) {
			cerr << "[GigaMesh] ERROR: return code from pthread_create() is " << rc << endl;
			exit( EXIT_FAILURE );
		}
	}

	cout << "[GigaMesh] preparing threads:       " << (int) ( time( NULL ) - timeStampParallel ) << " seconds." << endl;
	timeStampParallel = time( NULL );

	/* Free attribute and wait for the other threads */
	pthread_attr_destroy( &attr );
	int ctrIgnored   = 0;
	int ctrProcessed = 0;
	for( t=0; t<NUM_THREADS; t++ ) {
		rc = pthread_join( threads[t], &status );
		if( rc ) {
			cerr << "[GigaMesh] ERROR: return code from pthread_join() is " << rc << endl;
			exit( EXIT_FAILURE );
		}
		//cout << "[GigaMesh] Thread " << t << " processed: " << setMeshData[t].ctrProcessed << " vertices." << endl;
		//if( setMeshData[t].ctrIgnored > 0 ) {
		//	cout << "... ignored:   " << setMeshData[t].ctrIgnored << " vertices." << endl;
		//}
		ctrIgnored   += setMeshData[t].ctrIgnored;
		ctrProcessed += setMeshData[t].ctrProcessed;
	}
	pthread_mutex_destroy( &mutexVertexPtr );
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	cout << "[GigaMesh] End date/time is: " << asctime( timeinfo );// << endl;
	fileStrOutMeta << "End date/time is:   " << asctime( timeinfo ); // no endl required as asctime will add a linebreak
	fileStrOutMeta << "Parallel processing took " << (int)( time( NULL ) - timeStampParallel ) << " seconds." << endl;

	cout << "[GigaMesh] Vertices processed: " << ctrProcessed << endl;
	cout << "[GigaMesh] Vertices ignored:   " << ctrIgnored << endl;
	cout << "[GigaMesh] Parallel processing took " << (int)( time( NULL ) - timeStampParallel ) << " seconds." << endl;
	cout << "[GigaMesh]               ... equals " << (int) ctrProcessed / ( time( NULL ) - timeStampParallel ) << " vertices/seconds." << endl;

	timeStampParallel = time( NULL );

#ifndef GIGAMESH_PUBLIC_METHODS_ONLY
	// File for normal estimated:
	fstream filestrNormal;
	filestrNormal.open( fileNameOutPatchNormal.c_str(), fstream::out );
	filestrNormal << fixed << setprecision( 10 );
	for( unsigned long i=0; i<someMesh.getVertexNr(); i++ ) {
		if( featureIndicies[i] < 0 ) {
			//cout << "[GigaMesh] skip" << endl;
			continue;
		}
		filestrNormal << featureIndicies[i];
		// Normal - always three elements
		filestrNormal << " " << patchNormal[i*3];
		filestrNormal << " " << patchNormal[i*3+1];
		filestrNormal << " " << patchNormal[i*3+2];
		filestrNormal << endl;
	}
	filestrNormal.close();
	cout << "[GigaMesh] Patch normal stored in:                   " << fileNameOutPatchNormal << endl;
#endif

#ifdef GIGAMESH_PUBLIC_METHODS_ONLY
	delete descriptSurface;
	descriptSurface = NULL;
#endif

	// Feature vector file for BOTH descriptors (volume and surface)
	if( ( descriptSurface != NULL ) && ( descriptVolume != NULL ) ) {
		fstream filestrVS;
		filestrVS.open( fileNameOutVS.c_str(), fstream::out );
		filestrVS << fixed << setprecision( 10 );
		for( unsigned long i=0; i<someMesh.getVertexNr(); i++ ) {
			if( featureIndicies[i] < 0 ) {
				//cout << "skip" << endl;
				continue;
			}
			filestrVS << featureIndicies[i];
			// Scales - Volume:
			for( uint j=0; j<multiscaleRadiiSize; j++ ) {
				filestrVS << " " << descriptVolume[i*multiscaleRadiiSize+j];
			}
			// Scales - Surface:
			for( uint j=0; j<multiscaleRadiiSize; j++ ) {
				filestrVS << " " << descriptSurface[i*multiscaleRadiiSize+j];
			}
			filestrVS << endl;
		}
		filestrVS.close();
		cout << "[GigaMesh] Volume and surface descriptors stored in: " << fileNameOutVS << endl;
	}

	// Feature vector file for volume descriptor
	fstream filestrVol;
	if( descriptVolume != NULL ) {
		filestrVol.open( fileNameOutVol.c_str(), fstream::out );
		filestrVol << fixed << setprecision( 10 );
		for( unsigned long i=0; i<someMesh.getVertexNr(); i++ ) {
			if( featureIndicies[i] < 0 ) {
				//cout << "[GigaMesh] skip" << endl;
				continue;
			}
			Vertex* currVert = someMesh.getVertexPos( i );
			currVert->assignFeatureVec( &descriptVolume[i*multiscaleRadiiSize], multiscaleRadiiSize );
			filestrVol << featureIndicies[i];
			// Scales:
			for( uint j=0; j<multiscaleRadiiSize; j++ ) {
				filestrVol << " " << descriptVolume[i*multiscaleRadiiSize+j];
			}
			filestrVol << endl;
		}
		filestrVol.close();
		cout << "[GigaMesh] Volume descriptors stored in:             " << fileNameOutVol << endl;
		// Save mesh having volumetric feature vectors.
		someMesh.writeFile( fileNameOut3D );
	}

	// Feature vector file for surface descriptor
	if( descriptSurface != NULL ) {
		fstream filestrSurf;
		filestrSurf.open( fileNameOutSurf.c_str(), fstream::out );
		filestrSurf << fixed << setprecision( 10 );
		for( unsigned long i=0; i<someMesh.getVertexNr(); i++ ) {
			if( featureIndicies[i] < 0 ) {
				//cout << "[GigaMesh] skip" << endl;
				continue;
			}
			//Vertex* currVert = someMesh.getVertexPos( i );
			//currVert->assignFeatureVec( &descriptVolume[i*multiscaleRadiiSize], multiscaleRadiiSize );
			filestrSurf << featureIndicies[i];
			// Scales:
			for( uint j=0; j<multiscaleRadiiSize; j++ ) {
				filestrSurf << " " << descriptSurface[i*multiscaleRadiiSize+j];
			}
			filestrSurf << endl;
		}
		filestrSurf.close();
		cout << "[GigaMesh] Surface descriptors stored in:            " << fileNameOutSurf << endl;
	}
	cout << "[GigaMesh]                            " << (int)( time( NULL ) - timeStampParallel ) << " seconds." << endl;

	delete featureIndicies;
	if( descriptVolume != NULL ) {
		delete descriptVolume;
	}
	if( descriptSurface != NULL ) {
		delete descriptSurface;
	}
	delete multiscaleRadii;

	fileStrOutMeta.close();

	pthread_exit( NULL );
//*****************************#else
	exit( EXIT_SUCCESS );
}
