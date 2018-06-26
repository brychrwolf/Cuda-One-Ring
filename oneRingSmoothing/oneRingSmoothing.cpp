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
