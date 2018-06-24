bool Mesh::featureVecMedianOneRing(
	unsigned int rIterations,              //!< Number of interations
	double       rFilterSize,              //!< Filter size in units of the mesh (e.g. mm)
	bool         rPreferMeanOverMedian     //!< Compute mean value instead of the median.
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
		funcName = "Feature vector elements 1-ring mean, " + to_string( rIterations ) + " iterations";
	} else {
		funcName = "Feature vector elements 1-ring median, " + to_string( rIterations ) + " iterations";
	}
	time_t timeStart = clock();
	bool retVal = true;

	// Number of vertices
	unsigned long nrOfVertices = getVertexNr();

	// Pre-compute values
	showProgressStart( funcName + " Pre-Computation" );
	vector<vector<s1RingSectorPrecomp>> oneRingSectPrecomp;
	for( unsigned long vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
		VertexOfFace* currVertex = (VertexOfFace*)getVertexPos( vertIdx );
		vector<s1RingSectorPrecomp> curr1RingSectPrecomp;
		currVertex->get1RingSectors( minDist, curr1RingSectPrecomp );
		oneRingSectPrecomp.push_back( curr1RingSectPrecomp );
		showProgress( (double)(vertIdx+1)/(double)nrOfVertices, funcName + " Pre-Computation" );
	}
	showProgressStop( funcName + " Pre-Computation" );

	showProgressStart( funcName );
	// Apply multiple times
	for( unsigned int i=0; i<rIterations; i++ ) {
		vector<vector<double>> newFeatureVectors;
		unsigned int   vertsIgnored = 0;
		// 1. Compute indipendently and write back later!
		for( unsigned long vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
			Vertex* currVertex = getVertexPos( vertIdx );
			unsigned long featureVecLenCurr = currVertex->getFeatureVectorLen();
			vector<double> featureVecSmooth(featureVecLenCurr,_NOT_A_NUMBER_DBL_);
			bool retValCurr;
			if( rPreferMeanOverMedian ) {
				retValCurr = currVertex->getFeatureVecMeanOneRing( minDist, oneRingSectPrecomp.at(vertIdx), featureVecSmooth );
				// retValCurr = currVertex->getFeatureVecMeanOneRing( featureVecSmooth, minDist );
			} else {
				retValCurr = currVertex->getFeatureVecMedianOneRing( featureVecSmooth, minDist );
			}
			if( !retValCurr ) {
				vertsIgnored++;
			}
			newFeatureVectors.push_back( featureVecSmooth );
			showProgress( ((double)(i)/(double)rIterations) + (0.75*(double)((vertIdx+1)/(double)nrOfVertices))/(double)rIterations, funcName );
		}

		// 2. Write back the new values:
		for( unsigned long vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
			Vertex* currVertex = getVertexPos( vertIdx );
			vector<double> featureVecSmooth = newFeatureVectors.at( vertIdx );
			currVertex->assignFeatureVecValues( featureVecSmooth );
			showProgress( ((double)(i)/(double)rIterations) + (0.25+0.75*(double)((vertIdx+1)/(double)nrOfVertices))/(double)rIterations, funcName );
		}

		cout << "[Mesh::" << __FUNCTION__ << "] Iteration " << (i+1) << " Vertices processed: " << getVertexNr()-vertsIgnored << endl;
		cout << "[Mesh::" << __FUNCTION__ << "] Iteration " << (i+1) << " Vertices ignored:   " << vertsIgnored << endl;
	}

	cout << "[Mesh::" << __FUNCTION__ << "] took " << (float)( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	showProgressStop( funcName );
	changedVertFeatureVectors();
	return( retVal );
}
