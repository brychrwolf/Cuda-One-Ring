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

	// *********************************************************************************************************
	voxelFilter2DElements* sparseFilters;
	double** voxelFilters2D = generateVoxelFilters2D( multiscaleRadiiSize, multiscaleRadii, xyzDim, &sparseFilters );

	// Visual Debuging (for one filter):
	//Image2D someImage;
	//someImage.setSilent();
	//someImage.writeTIFF( "voxelFilter2D", xyzDim, xyzDim, voxelFilters2D[9], (double)xyzDim, false );

	// Numeric Debuging and Error Esitmation:
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		// sphere volume = 4/3 * pi * r³ => 1/6 * pi * d³
		double idealSpherVol = pow( xyzDim*multiscaleRadii[i], 3.0 ) * M_PI / 6.0;
		double discreteSphereVol = sumVoxelFilter2D( voxelFilters2D[i], xyzDim );
		cout << "[GigaMesh] Sum of the filter mask: " << setprecision( 1 );
		cout.width( 10 ); 
		cout << right << discreteSphereVol;
		cout << " Ideal: ";
		cout.width( 10 ); 
		cout << right << idealSpherVol << " Error: ";
		cout.width( 6 ); 
		cout << setprecision( 3 ) << right << 100.0*(1.0-(discreteSphereVol/idealSpherVol)) << " %" << endl;
	}
	//exit( EXIT_SUCCESS );
	time_t timeStampSerial = time( NULL );

//	MeshSeed* meshSeedToAnalyse;
	double* rasterArray = new double[xyzDim*xyzDim];
	int i = 0;
	int ctrIgnored = 0;
	int ctrTotal   = 0;
	int verticesNrTotal = someMesh.getVertexNr();
	// for processing time prediction
	int timeLoopStart = clock(); 
	float timeLoopElapsed;
	double percentDone;

	//set<Face*> facesInSphere; // temp variable to store local surface patches - returned by fetchSphereMarching
	vector<Face*> facesInSphere; // temp variable to store local surface patches - returned by fetchSphereMarching

	// Allocate the bit arrays for vertices:
	unsigned long* vertBitArrayVisited;
	int vertNrLongs = someMesh.getBitArrayVerts( &vertBitArrayVisited );
	// Allocate the bit arrays for faces:
	unsigned long* faceBitArrayVisited;
	int faceNrLongs = someMesh.getBitArrayFaces( &faceBitArrayVisited );

	//! \todo ADAPT for -a option (surface descriptor only => descriptVolume == NULL!)
	// Surface descriptor
	double* absolutRadii = new double[multiscaleRadiiSize];
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		absolutRadii[i] = (double) multiscaleRadii[i] * (double) radius;
	}

	// which we write to:
	fstream filestrVS;
	fstream filestrVol;
	fstream filestrSurf;
	filestrVS.open( fileNameOutVS.c_str(), fstream::out );
	filestrVol.open( fileNameOutVol.c_str(), fstream::out );
	filestrSurf.open( fileNameOutSurf.c_str(), fstream::out );
	filestrVS   << fixed << setprecision( 10 );
	filestrVol  << fixed << setprecision( 10 );
	filestrSurf << fixed << setprecision( 10 );
	Vertex* currVertex;
	for( int vertIdx=0; vertIdx<verticesNrTotal; vertIdx++ ) {
		currVertex = someMesh.getVertexPos( vertIdx );
/*
		meshSeedToAnalyse = someMesh.fetchSphereVolume2( (*itVertex), radius, xyzDim, true );
		ctrTotal++;
		if( meshSeedToAnalyse == NULL ) {
			//cout << "[GigaMesh] Border or non-manifold parts for seed Vertex " << (*itVertex)->getIndexOriginal() << " (OriIdx) found" << endl;
			ctrIgnored++;
			continue;
		}
		meshSeedToAnalyse->rasterViewFromZ( rasterArray, xyzDim, xyzDim );
*/
		ctrTotal++;
		facesInSphere.clear();
		unsigned int descriptIndexOffset = vertIdx*multiscaleRadiiSize;

		// slower for larger patches - uses set for facesInSphere
		//someMesh.fetchSphereMarching( (*itVertex), &facesInSphere, radius, true );
		//someMesh.fetchSphereMarchingDualFront( currVertex, &facesInSphere, radius );
		//someMesh.fetchSphereBitArray( currVertex, &facesInSphere, (float)radius, vertNrLongs, vertBitArrayVisited, faceNrLongs, faceBitArrayVisited );
		someMesh.fetchSphereBitArray1R( currVertex, &facesInSphere, (float)radius, vertNrLongs, vertBitArrayVisited, faceNrLongs, faceBitArrayVisited );

		// reset rasterArray - may be removed, when performed within fetchSphereCubeVolume25D
		for( uint j=0; j<(xyzDim*xyzDim); j++ ) {
			rasterArray[j] = _NOT_A_NUMBER_;
		}
		if( !someMesh.fetchSphereCubeVolume25D( currVertex, &facesInSphere, radius, rasterArray, xyzDim ) ) {
			//cout << "[GigaMesh] Border or non-manifold parts for seed Vertex " << (*itVertex)->getIndexOriginal() << " (OriIdx) found" << endl;
			ctrIgnored++;
			continue;
		}
		for( uint j=0; j<multiscaleRadiiSize; j++ ) {
			descriptVolume[j] = _NOT_A_NUMBER_;
		}

		// Get volume descriptor
		// sparse is faster:
		//applyVoxelFilters2D( featureVecs, rasterArray, multiscaleRadiiSize, voxelFilters2D, xyzDim );
		applyVoxelFilters2D( &(descriptVolume[descriptIndexOffset]), rasterArray, &sparseFilters, multiscaleRadiiSize, xyzDim );
		currVertex->assignFeatureVec( &(descriptVolume[descriptIndexOffset]), multiscaleRadiiSize );

		// Get surface descriptor:
		Vector3D seedPosition = currVertex->getCenterOfGravity();
		someMesh.fetchSphereArea( currVertex, &facesInSphere, (unsigned int)multiscaleRadiiSize, absolutRadii, &(descriptSurface[descriptIndexOffset]) );

		// Write to file(s)
		int vertIndex = currVertex->getIndexOriginal();
		filestrVS   << vertIndex;
		filestrVol  << vertIndex;
		filestrSurf << vertIndex;
		for( uint j=0; j<multiscaleRadiiSize; j++ ) {
			//cout << " " << descriptVolume[descriptIndexOffset+j];
			//cout << " " << descriptSurface[descriptIndexOffset+j];
			filestrVS   << " " << descriptVolume[descriptIndexOffset+j];
			filestrVol  << " " << descriptVolume[descriptIndexOffset+j];
			filestrSurf << " " << descriptSurface[descriptIndexOffset+j];
		}
		//cout << endl;
		// Append surface for the outputfile containing both descriptors
		for( uint j=0; j<multiscaleRadiiSize; j++ ) {
			filestrVS << " " << descriptSurface[descriptIndexOffset+j];
		}
		filestrVS   << endl;
		filestrVol  << endl;
		filestrSurf << endl;

		//! \bug causes double free problem if we generate a Mesh in between, because ~Mesh frees the same memory.
//		delete( meshSeedToAnalyse ); // if you don't do this you get nice segmentation faults
		i++; 
		timeLoopElapsed = (float)( clock() - timeLoopStart ) / CLOCKS_PER_SEC;
		percentDone = (double)ctrTotal/(double)verticesNrTotal;
		if( i % 100 == 0 ) {
			cout << "[GigaMesh] Time elapsed:   " << timeLoopElapsed << " seconds."  << endl;
			cout << "[GigaMesh] Percent done:   " << 100.0*percentDone << "%"  << endl;
			cout << "[GigaMesh] Time remaining: " << ((timeLoopElapsed/percentDone)-timeLoopElapsed)/60.0 << " minutes."  << endl;
		}
	}
	filestrVS.close();
	filestrVol.close();
	filestrSurf.close();

	time( &rawtime );
	timeinfo = localtime( &rawtime );
	cout << "[GigaMesh] End date/time is: " << asctime( timeinfo );// << endl;
	fileStrOutMeta << "End date/time is:   " << asctime( timeinfo ); // no endl required as asctime will add a linebreak
	fileStrOutMeta << "Serial processing took " << (int)( time( NULL ) - timeStampSerial ) << " seconds." << endl;

	cout << "[GigaMesh] Vertices ignored: " << ctrIgnored << endl;
	cout << "[GigaMesh] Serial processing took " << (int)( time( NULL ) - timeStampSerial ) << " seconds." << endl;
	cout << "[GigaMesh] Volume and Surface descriptors stored in: " << fileNameOutVS << "" << endl;
	cout << "[GigaMesh] Volume descriptors stored in:             " << fileNameOutVol << "" << endl;
	cout << "[GigaMesh] Surface descriptors stored in:            " << fileNameOutSurf << "" << endl;

	// Save mesh having volumetric feature vectors.
	someMesh.writeFile( fileNameOut3D );

	delete featureIndicies;
	delete descriptVolume;
	// Surface descriptor
	delete absolutRadii;
	delete descriptSurface;

	delete multiscaleRadii;

	fileStrOutMeta.close();
	exit( EXIT_SUCCESS );
}
