###Acquired Meshes
declare -a mesh=("acquired_meshes/ILATO_1A_SM2066-HE5-60_070214_merged_GMO_ASCII.ply")
declare -a funcVals=("acquired_meshes/ILATO_1A_SM2066-HE5-60_070214_merged_GMO_ASCII_funcvals_distToPlane.txt")

declare -a mesh=("acquired_meshes/Unisiegel_UAH_Ebay-Siegel_Uniarchiv_HE2066-60_010614_partial_ASCII.ply")
#declare -a funcVals=("acquired_meshes/Unisiegel_UAH_Ebay-Siegel_Uniarchiv_HE2066-60_010614_partial_ASCII_funcvals_distToPlane.txt")



###Synthetic Meshes
declare -a meshes=("synthetic_meshes/hexagonal_tessellation_Dirac_delta_1_v31_f42.ply"\
				   "synthetic_meshes/hexagonal_tessellation_Dirac_delta_3_v133_f222.ply"\
				   "synthetic_meshes/hexagonal_tessellation_Dirac_delta_10_v1057_f1986.ply"\
				   "synthetic_meshes/hexagonal_tessellation_Dirac_delta_30_v8557_f16746.ply"\
				   "synthetic_meshes/hexagonal_tessellation_Dirac_delta_100_v91507_f181806.ply"\
				   "synthetic_meshes/hexagonal_tessellation_Dirac_delta_300_v814507_f1625406.ply"\
				   "synthetic_meshes/hexagonal_tessellation_Dirac_delta_1000_v9015007_f18018006.ply"\
				   "synthetic_meshes/hexagonal_tessellation_Dirac_delta_3000_v81045007_f162054006.ply"\
				   "synthetic_meshes/random_circle_tessellation_Dirac_delta_1000_v1000_f1965.ply"\
				   "synthetic_meshes/random_circle_tessellation_Dirac_delta_1000_v6004001_f12007382.ply"\
				   "synthetic_meshes/random_circle_tessellation_Dirac_delta_100_v60401_f120668.ply"\
				   "synthetic_meshes/random_circle_tessellation_Dirac_delta_10_v641_f1252.ply"\
				   "synthetic_meshes/random_circle_tessellation_Dirac_delta_1_v11_f12.ply"\
				   "synthetic_meshes/random_circle_tessellation_Dirac_delta_3000_v54012001_f108022714.ply"\
				   "synthetic_meshes/random_circle_tessellation_Dirac_delta_300_v541201_f1082132.ply"\
				   "synthetic_meshes/random_circle_tessellation_Dirac_delta_30_v5521_f10971.ply"\
				   "synthetic_meshes/random_circle_tessellation_Dirac_delta_3_v67_f119.ply"\
				   "synthetic_meshes/square_tessellation_2tri_Dirac_delta_1000_v4004001_f8000000.ply"\
				   "synthetic_meshes/square_tessellation_2tri_Dirac_delta_100_v40401_f80000.ply"\
				   "synthetic_meshes/square_tessellation_2tri_Dirac_delta_10_v441_f800.ply"\
				   "synthetic_meshes/square_tessellation_2tri_Dirac_delta_1_v9_f8.ply"\
				   "synthetic_meshes/square_tessellation_2tri_Dirac_delta_3000_v36012001_f72000000.ply"\
				   "synthetic_meshes/square_tessellation_2tri_Dirac_delta_300_v361201_f720000.ply"\
				   "synthetic_meshes/square_tessellation_2tri_Dirac_delta_30_v3721_f7200.ply"\
				   "synthetic_meshes/square_tessellation_2tri_Dirac_delta_3_v49_f72.ply"\
				   "synthetic_meshes/square_tessellation_4tri_Dirac_delta_1000_v8004001_f16000000.ply"\
				   "synthetic_meshes/square_tessellation_4tri_Dirac_delta_100_v80401_f160000.ply"\
				   "synthetic_meshes/square_tessellation_4tri_Dirac_delta_10_v841_f1600.ply"\
				   "synthetic_meshes/square_tessellation_4tri_Dirac_delta_1_v13_f16.ply"\
				   "synthetic_meshes/square_tessellation_4tri_Dirac_delta_3000_v72012001_f144000000.ply"\
				   "synthetic_meshes/square_tessellation_4tri_Dirac_delta_300_v721201_f1440000.ply"\
				   "synthetic_meshes/square_tessellation_4tri_Dirac_delta_30_v7321_f14400.ply"\
				   "synthetic_meshes/square_tessellation_4tri_Dirac_delta_3_v85_f144.ply")

declare -a mesh=("synthetic_meshes/square_tessellation_2tri_Dirac_delta_1000_v4004001_f8000000.ply")

meshCount=${#mesh[@]}
declare -a iter=(1 3 10 30 100 300 1000 3000)
for (( m=0; m < ${meshCount}; m++ )); do for i in {0..7}; do ./lib ${iter[$i]} ${mesh[$m]}; done; done

#With optirun
#for (( m=0; m < ${meshCount}; m++ )); do for i in {0..7}; do optirun ./lib ${iter[$i]} ${mesh[$m]}; done; done

#external funcvals (optirun)
#for (( m=0; m < ${meshCount}; m++ )); do for i in {0..7}; do optirun ./lib ${iter[$i]} ${mesh[$m]} ${funcVals[$m]}; done; done



#Test file existence
#for (( m=0; m < ${meshCount}; m++ )); do if [ -f ${mesh[$m]} ]; then echo "exists" ${mesh[$m]}; fi; done

