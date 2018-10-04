//! Apply a median or mean filter operation on a vertex's 1-ring neighbourhood.
//! User interactive.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::funcVertMedianOneRingUI(
	bool         rPreferMeanOverMedian    //!< Compute mean value instead of the median.
) {
	bool storeDiffAsFeatureVec;
	if( !showQuestion( &storeDiffAsFeatureVec, "Store changes", "as feature vectors" ) ) {
		// User cancel
		return( false );
	}

	bool retVal = true;
	// Ask for a filter size (radius) in mesh units:
	double filterSize = 0.25;
	if( showEnterText( &filterSize ) ) {
		if( filterSize <= 0.0 ) {
			// Ask for a number of iterations, when no radius was given:
			unsigned int iterations=1;
			if( showEnterText( &iterations ) ) {
				retVal = funcVertMedianOneRing( iterations, 0.0, rPreferMeanOverMedian, storeDiffAsFeatureVec );
			} else {
				retVal = false; // i.e. user cancel or invalid values.
			}
		} else {
			retVal = funcVertMedianOneRing( 0, filterSize, rPreferMeanOverMedian, storeDiffAsFeatureVec );
		}
	}

	return( retVal );
}

//! Apply a median or mean filter operation on a vertex's 1-ring neighbourhood.
//!
//! The filter size can be defined by either
//!     providing a number of iterations by providing rIterations > 0
//! or
//!     a radius in mesh units by providing a rFilterSize > 0.0.
//! In case both valies are set (valid), the number of iterations is used.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::funcVertMedianOneRing(
	unsigned int rIterations,              //!< Number of interations
	double       rFilterSize,              //!< Filter size in units of the mesh (e.g. mm)
	bool         rPreferMeanOverMedian,    //!< Compute mean value instead of the median.
	bool         rStoreDiffAsFeatureVec    //!< Option to store the changes as feature vectors.
) {
	// 0a. Pre-Compute the minimum face altitude length
	// double minDist = getAltitudeMin();
	// cout << "[Mesh::" << __FUNCTION__ << "] Minimal altitude: " << minDist << " mm (unit assumed)." << endl;

	// 0a. Pre-Compute the minimum edge length
	double minDist = getEdgeLenMin();
	cout << "[Mesh::" << __FUNCTION__ << "] Minimal edge length: " << minDist << " mm (unit assumed)." << endl;

	// 0b. Sanity checks
	if( rIterations == 0 ) {
		if( rFilterSize <= 0.0 ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Zero iterations or invalid filter size requested!" << endl;
			return( false );
		}
		// Compute number of iterations using the given filter size:
		rIterations = round( 2.0*rFilterSize/minDist );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] " << rIterations << " iterations corresponds to a filter size (radius) of " << rIterations*minDist << " mm (unit assumed)." << endl;

	string funcName;
	if( rPreferMeanOverMedian ) {
		funcName = "Vertex function value 1-ring mean, " + to_string( rIterations ) + " iterations";
	} else {
		funcName = "Vertex function value 1-ring median, " + to_string( rIterations ) + " iterations";
	}
	time_t timeStart = clock();
	bool retVal = true;
	Vertex* currVertex;

	// Fetch number of vertices.
	unsigned long nrOfVertices = getVertexNr();

	// Option A: use the difference as feature vector for flow visualization
	// Prepare two-dimensional array as vector
	vector<double> diffFlowFTVec;
	if( rStoreDiffAsFeatureVec ) {
		cout << "[Mesh::" << __FUNCTION__ << "] Allocating memory. which requires a moment." << endl;
		diffFlowFTVec.assign( nrOfVertices*rIterations, _NOT_A_NUMBER_DBL_ );
	}

	// Time has to be counted after the memory assignment. Otherwise the estimated time is way off for larger numbers of iterations.
	showProgressStart( funcName );

	// Apply multiple times
	for( unsigned int i=0; i<rIterations; i++ ) {
		vector<double> newFuncVals;
		unsigned int   vertsIgnored = 0;
		double         akkuTrackDiff = 0.0;
		unsigned int   akkuTrackChangesCount = 0;
		// 1. Compute indipendently and write back later!
		for( unsigned long vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
			currVertex = getVertexPos( vertIdx );
			double funcVal;
			double funcValPrev;
			currVertex->getFuncValue( &funcVal );
			funcValPrev = funcVal;
			bool retValCurr;
			if( rPreferMeanOverMedian ) {
				retValCurr = currVertex->funcValMeanOneRing( &funcVal, minDist );
			} else {
				retValCurr = currVertex->funcValMedianOneRing( &funcVal, minDist );
			}
			if( !retValCurr ) {
				vertsIgnored++;
			}
			newFuncVals.push_back( funcVal );
			if( isfinite( funcVal ) && isfinite( funcValPrev ) ) {
				akkuTrackDiff += abs( funcVal - funcValPrev );
				akkuTrackChangesCount++;
			}
			showProgress( ((double)(i)/(double)rIterations) + (0.75*(double)((vertIdx+1)/(double)nrOfVertices))/(double)rIterations, funcName );
		}

		// 2. Write back the new values:
		for( unsigned long vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
			currVertex = getVertexPos( vertIdx );
			double newValue = newFuncVals.at( vertIdx );

			// Option A: use the difference as feature vector for flow visualization
			// Compute and store difference
			if( rStoreDiffAsFeatureVec ) {
				double oldValue = _NOT_A_NUMBER_DBL_;
				currVertex->getFuncValue( &oldValue );
				// Rember: Feature vector elements are expected consecutive
				diffFlowFTVec.at( vertIdx*rIterations + i ) = oldValue - newValue;
			}

			currVertex->setFuncValue( newValue );
			showProgress( ((double)(i)/(double)rIterations) + (0.25+0.75*(double)((vertIdx+1)/(double)nrOfVertices))/(double)rIterations, funcName );
		}

		cout << "[Mesh::" << __FUNCTION__ << "] Iteration " << (i+1) << " Vertices processed: " << getVertexNr()-vertsIgnored << endl;
		cout << "[Mesh::" << __FUNCTION__ << "] Iteration " << (i+1) << " Vertices ignored:   " << vertsIgnored << endl;
		cout << "[Mesh::" << __FUNCTION__ << "] Iteration " << (i+1) << " Relative changes to the function vales:   " << akkuTrackDiff / (double)akkuTrackChangesCount << endl;
	}

	// Option A: use the difference as feature vector for flow visualization
	// Store as new feature vector
	if( rStoreDiffAsFeatureVec ) {
		removeFeatureVectors();
		mFeatureVecVerticesLen = rIterations;
		mFeatureVecVertices.swap( diffFlowFTVec );
		if( !assignFeatureVectors( mFeatureVecVertices, mFeatureVecVerticesLen ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: during assignment of the feature vectors!" << endl;
			retVal = false;
		}
	}

	cout << "[Mesh::" << __FUNCTION__ << "] took " << (float)( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	showProgressStop( funcName );
	changedVertFuncVal();
	return( retVal );
}

//! Compute the minimum edge length of all faces.
//!
//! Remark 1: Calling this method often will result in low performance.
//! Remark 2: Edge lengths of polyines are ignored.
//!
//! @returns minimum edge length of all triangles within the mesh.
double Mesh::getEdgeLenMin() {
	double edgeLenMin = _INFINITE_DBL_;
	Face* currFace;
	for( unsigned long faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		double edgeLen = _INFINITE_DBL_;
		currFace->getEdgeLenMin( &edgeLen );
		if( edgeLen < edgeLenMin ) {
			edgeLenMin = edgeLen;
		}
	}
	return edgeLenMin;
}

//! Determine the actual number of faces, excluding faces marked for removal, including new faces.
//!
//! See also Mesh::getFacePos
//!
//! @returns the actual number of faces.
unsigned long Mesh::getFaceNr() {
	return mFaces.size();
}

//! Retrieves the n-th Face, excluding faces marked for removal, including new faces.
//!
//! @returns NULL in case of an error.
Face* Mesh::getFacePos( unsigned long posIdx ) {
	if( posIdx >= getFaceNr() ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Index (" << posIdx << ") too large - Max: " << getFaceNr() << "!" << endl;
		return NULL;
	}
	return mFaces.at( posIdx );
}

//! Determine the shortest edge length of this triangle.
//!
//! @returns false in case of an error i.e. null-pointer given
//!          or degenerated triangle. True otherwise.
bool Face::getEdgeLenMin( double* rLenMin ) {
	// Sanity check:
	if( rLenMin == nullptr ) {
		return( false );
	}

	// Compute shortest edge:
	double edgeLenMin = getLengthAB();
	if( getLengthBC() < edgeLenMin ) {
		edgeLenMin = getLengthBC();
	}
	if( getLengthCA() < edgeLenMin ) {
		edgeLenMin = getLengthCA();
	}

	// Do not return values from degenerated triangles:
	if( !isnormal( edgeLenMin ) ) {
		return( false );
	}

	// Everything checks out - return the value:
	(*rLenMin) = edgeLenMin;
	return( true );
}

double Face::getLengthAB() const {
	//! Get the length of the edge A->B
	Vector3D dirVec = vertB->getPositionVector() - vertA->getPositionVector();
	double lenVec = dirVec.getLength3();
	return lenVec;
}

double Face::getLengthBC() const {
	//! Get the length of the edge B->C
	Vector3D dirVec = vertC->getPositionVector() - vertB->getPositionVector();
	double lenVec = dirVec.getLength3();
	return lenVec;
}

double Face::getLengthCA() const {
	//! Get the length of the edge C->A
	Vector3D dirVec = vertA->getPositionVector() - vertC->getPositionVector();
	double lenVec = dirVec.getLength3();
	return lenVec;
}

double Vector3D::getLength3() const {
	//! Returns the length of the inhomogenous Vector3D.
	return sqrt( pow( x, 2 ) + pow( y, 2 ) + pow( z, 2 ) );
}

//! Returns the actual number of vertices, excluding vertices marked for removal, including new vertices.
//! See also Mesh::getVertexPos
unsigned long Mesh::getVertexNr() {
	return mVertices.size();
}

//! Retrieves the n-th Vertex, excluding vertices marked for removal, including new vertices.
//! @returns the nullptr in case of an error.
Vertex* Mesh::getVertexPos( unsigned long rPosIdx ) {
	if( rPosIdx >= getVertexNr() ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Index too large!" << endl;
		return( nullptr );
	}
	return( mVertices.at( rPosIdx ) );
}

inline bool Vertex::getFuncValue( double* rGetVal ) const {
	//! Method to retrieve the function value.
	//! Returns false in case of an error (or not implemented).
	*rGetVal = mFuncValue;
	return true;
}

//! Applies an 1-ring mean filter to the function value.
//!
//! See also: Face::getFuncVal1RingSector (used)
//!           Mesh::funcVertMedianOneRing (calling method)
//!
//! @returns false in case of an error. True otherwise.
bool VertexOfFace::funcValMeanOneRing(
                double* rMeanValue,    //!< to return the computed mean value
                double  rMinDist       //!< smallest edge length to be expected
) {
	set<Face*> oneRingFaces;
	getFaces( &oneRingFaces );

	double accuFuncVals = 0.0;
	double accuArea = 0.0;

	// Fetch values
	for( auto const& currFace: oneRingFaces ) {
		double currFuncVal;
#ifdef OLD1RINGSMOOTH
		currFace->getFuncVal1RingThird( this, rMinDist, &currFuncVal );
		// SWS - Seite Winkel Seite
		// Two edges (b,c) and the enclosed angle: A=b*c*sin(alpha). Because of a=b=const. skipped.
		double currArea = sin( currFace->getAngleAtVertex( this ) );
#else
		double currArea = _NOT_A_NUMBER_DBL_;
		currFace->getFuncVal1RingSector( this, rMinDist, currArea, currFuncVal );
#endif
		accuFuncVals += currFuncVal * currArea;
		accuArea += currArea;
	}

	// Finally compue the mean value:
	accuFuncVals /= accuArea;

	// Write back
	(*rMeanValue) = accuFuncVals;
	return( true );
}

//! Adds all adjacent Faces of this Vertex to the given list/set of Faces.
//! Because we use a set, there will be NO duplicate faces.
//!
//! Typically called by Mesh::removeVertices()
void VertexOfFace::getFaces( set<Face*>* someFaceList ) {
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		someFaceList->insert( mAdjacentFaces[i] );
	}
}

//! Computes a circle sector and a mean function value of
//! the corresponding prism at the center of gravity.
//!
//! See also: VertexOfFace::funcValMeanOneRing (calling method)
//!           Mesh::funcVertMedianOneRing (superior calling method)
//!
//! @returns false in case of an error. True otherwise.
bool Face::getFuncVal1RingSector(
                const Vertex* rVert1RingCenter, //!< Center of the 1-ring (p_0)
                const double& rNormDist,        //!< Filtersize/-radius typically the shortest edge within the mesh (Delta_r_min)
                double&       rSectorArea,      //!< Return value: size of the geodesic sector, which is a circle sector at p_0.
                double&       rSectorFuncVal    //!< Function value at the center of gravity of the sector
) {
	// Sanity check
	if( rVert1RingCenter == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: Null pointer given!" << endl;
		return( false );
	}

	// Values to be pre-computed
	s1RingSectorPrecomp oneRingSecPre;
	if( !get1RingSectorConst( rVert1RingCenter, rNormDist,
	                          oneRingSecPre ) ) {
		return( false );
	}

	// Fetch function values
	double funcValCenter;
	double funcValA;
	double funcValB;
	rVert1RingCenter->getFuncValue( &funcValCenter );
	oneRingSecPre.mVertOppA->getFuncValue( &funcValA );
	oneRingSecPre.mVertOppB->getFuncValue( &funcValB );

	// Interpolate
	double funcValInterpolA = funcValCenter*(1.0-oneRingSecPre.mRatioCA) + funcValA*oneRingSecPre.mRatioCA;
	double funcValInterpolB = funcValCenter*(1.0-oneRingSecPre.mRatioCB) + funcValB*oneRingSecPre.mRatioCB;

	// Compute average function value at the center of gravity of the circle sector
	rSectorFuncVal = funcValCenter*( 1.0 - oneRingSecPre.mCenterOfGravityDist ) +
	                 ( funcValInterpolA + funcValInterpolB ) * oneRingSecPre.mCenterOfGravityDist / 2.0;
	// Pass thru
	rSectorArea = oneRingSecPre.mSectorArea;
	return( true );
}

//! Pre-Computes values for a circle sector to compute a mean
//! or weigthed median for e.g. the function value or feature vector elements.
//!
//! See also: VertexOfFace::funcValMeanOneRing (calling method)
//!           Mesh::funcVertMedianOneRing (superior calling method)
//!
//! @returns false in case of an error. True otherwise.
bool Face::get1RingSectorConst(
                const Vertex*        rVert1RingCenter,
                const double&        rNormDist,
                s1RingSectorPrecomp& r1RingSecPre
) {
	// Sanity check I
	if( rVert1RingCenter == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: Null pointer given!" << endl;
		return( false );
	}

	// Fetch angle
	double alpha = getAngleAtVertex( rVert1RingCenter );
	// Sanity check II
	if( !isfinite( alpha ) || abs( alpha ) >= M_PI ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: Invalid angle of " << alpha << "!" << endl;
		return( false );
	}

	// Area - https://en.wikipedia.org/wiki/Circular_sector#Area
	r1RingSecPre.mSectorArea = rNormDist * rNormDist * alpha / 2.0; // As alpha is already in radiant.

	// Truncated prism - function value equals the height
	if( !getOposingVertices( rVert1RingCenter, r1RingSecPre.mVertOppA, r1RingSecPre.mVertOppB ) ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: Finding opposing vertices!" << endl;
		return( false );
	}

	// Function values interpolated f'_i and f'_{i+1}
	// Compute the third angle using alpha/2.0 and 90°:
	double beta = ( M_PI - alpha ) / 2.0;
	// Law of sines
	double diameterCircum = rNormDist / sin( beta ); // Constant ratio equal longest edge
	// Distances for interpolation
	double lenCenterToA = distance( rVert1RingCenter, r1RingSecPre.mVertOppA );
	double lenCenterToB = distance( rVert1RingCenter, r1RingSecPre.mVertOppB );
	r1RingSecPre.mRatioCA = diameterCircum / lenCenterToA;
	r1RingSecPre.mRatioCB = diameterCircum / lenCenterToB;
	// Circle segment, center of gravity - https://de.wikipedia.org/wiki/Geometrischer_Schwerpunkt#Kreisausschnitt
	r1RingSecPre.mCenterOfGravityDist = ( 2.0 * sin( alpha ) ) / ( 3.0 * alpha );

	return( true );
}

//! Compute the angle next to a given Vertex reference
//! in radiant.
//!
//! @returns zero if the vertex is not part of the triangle.
//!          Otherwise the angle is returned in radiant.
double Face::getAngleAtVertex( const Vertex* vertABC ) const {
	// Determine the vertex
	if( vertABC == vertA ) {
		return estimateAlpha();
	}
	if( vertABC == vertB ) {
		return estimateBeta();
	}
	if( vertABC == vertC ) {
		return estimateGamma();
	}

	// No matching vertex found:
	cerr << "[Face::" << __FUNCTION__ << "] Vertex " << vertABC->getIndex()
	     << " is not part of face [" << getIndex() << "]." << endl;
	dumpFaceInfo();
	return( 0.0 );
}

double Face::estimateAlpha() const {
	//! Estimates the angle alpha between C-A-B in radiant.
	//!
	//! http://de.wikipedia.org/wiki/Dreieck#Berechnung_eines_beliebigen_Dreiecks
	double alpha;
	double lengthEdgeA = getLengthBC();
	double lengthEdgeB = getLengthCA();
	double lengthEdgeC = getLengthAB();
	alpha = acos( ( lengthEdgeB*lengthEdgeB + lengthEdgeC*lengthEdgeC - lengthEdgeA*lengthEdgeA ) / ( 2*lengthEdgeB*lengthEdgeC ) );
	return alpha;
}

double Face::estimateBeta() const {
	//! Estimates the angle beta between A-B-C in radiant.
	double beta;
	double lengthEdgeA = getLengthBC();
	double lengthEdgeB = getLengthCA();
	double lengthEdgeC = getLengthAB();
	beta = acos( ( lengthEdgeA*lengthEdgeA + lengthEdgeC*lengthEdgeC - lengthEdgeB*lengthEdgeB ) / ( 2*lengthEdgeA*lengthEdgeC ) );
	return beta;
}

double Face::estimateGamma() const {
	//! Estimates the angle gamma between B-C-A in radiant.
	double gamma;
	double lengthEdgeA = getLengthBC();
	double lengthEdgeB = getLengthCA();
	double lengthEdgeC = getLengthAB();
	gamma = acos( ( lengthEdgeA*lengthEdgeA + lengthEdgeB*lengthEdgeB - lengthEdgeC*lengthEdgeC ) / ( 2*lengthEdgeA*lengthEdgeB ) );
	return gamma;
}

//! Determine the two opposing vertices.
//!
//! @returns false in case of an error including that the given reference vertex was not found.
//!          True otherwise.
bool Face::getOposingVertices(
                const Vertex*  rVertRef,
                      Vertex*& rVertOpp1,
                      Vertex*& rVertOpp2
) const {
	if( rVertRef == vertA ) {
		rVertOpp1 = vertB;
		rVertOpp2 = vertC;
		return( true );
	}
	if( rVertRef == vertB ) {
		rVertOpp1 = vertC;
		rVertOpp2 = vertA;
		return( true );
	}
	if( rVertRef == vertC ) {
		rVertOpp1 = vertA;
		rVertOpp2 = vertB;
		return( true );
	}
	// When we reach this point something has gone wrong - rVertRef not part of this Face.
	return( false );
}

//! Compute the distance between to vertices.
double distance( const Vertex* rVertA, const Vertex* rVertB ) {
	double dist;
	double posB[3];
	rVertB->copyXYZTo( posB );
	rVertA->getDistanceFromCenterOfGravityTo( posB, &dist );
	return( dist );
	// Faster than: return abs3( rVertA->getPositionVector() - rVertB->getPositionVector() );
}
