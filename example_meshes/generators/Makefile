all: hexagon \
	 square2 \
	 square4 \
	 randcircle

hexagon:
	g++ hexagonal_tessellation_Dirac_delta.cpp -o hexagonal_tessellation_Dirac_delta

square2:
	g++ square_tessellation_2tri_Dirac_delta.cpp -o square_tessellation_2tri_Dirac_delta

square4:
	g++ square_tessellation_4tri_Dirac_delta.cpp -o square_tessellation_4tri_Dirac_delta
	
randCircle:
	g++ random_circle_Dirac_delta.cpp -o random_circle_Dirac_delta

clean:
	rm -f \
	hexagonal_tessellation_Dirac_delta \
	square_tessellation_2tri_Dirac_delta \
	square_tessellation_4tri_Dirac_delta \
	random_circle_Dirac_delta
