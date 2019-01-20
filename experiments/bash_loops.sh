###Acquired Meshes
declare -a mesh=("acquired_meshes/ILATO_1A_SM2066-HE5-60_070214_merged_GMO_ASCII.ply")
declare -a funcVals=("acquired_meshes/ILATO_1A_SM2066-HE5-60_070214_merged_GMO_ASCII_funcvals_distToPlane.txt")

declare -a mesh=("acquired_meshes/Unisiegel_UAH_Ebay-Siegel_Uniarchiv_HE2066-60_010614_partial_ASCII.ply")
#declare -a funcVals=("acquired_meshes/Unisiegel_UAH_Ebay-Siegel_Uniarchiv_HE2066-60_010614_partial_ASCII_funcvals_distToPlane.txt")



###Synthetic Meshes
declare -a mesh=("synthetic_meshes/hexagonal_tessellation_Dirac_delta_1000_v9015007_f18018006.ply")
declare -a meshes=("synthetic_meshes/hexagonal_tessellation_Dirac_delta_1_v31_f42.ply"\
				 "synthetic_meshes/hexagonal_tessellation_Dirac_delta_3_v133_f222.ply"\
				 "synthetic_meshes/hexagonal_tessellation_Dirac_delta_10_v1057_f1986.ply"\
				 "synthetic_meshes/hexagonal_tessellation_Dirac_delta_30_v8557_f16746.ply"\
				 "synthetic_meshes/hexagonal_tessellation_Dirac_delta_100_v91507_f181806.ply"\
				 "synthetic_meshes/hexagonal_tessellation_Dirac_delta_300_v814507_f1625406.ply"\
				 "synthetic_meshes/hexagonal_tessellation_Dirac_delta_1000_v9015007_f18018006.ply"\
				 "synthetic_meshes/hexagonal_tessellation_Dirac_delta_3000_v81045007_f162054006.ply")



meshCount=${#mesh[@]}
declare -a iter=(1 3 10 30 100 300 1000 3000)
for (( m=0; m < ${meshCount}; m++ )); do for i in {0..7}; do optirun ./lib ${iter[$i]} ${mesh[$m]} ${funcVals[$m]}; done; done
for (( m=0; m < ${meshCount}; m++ )); do for i in {0..7}; do optirun ./lib ${iter[$i]} ${mesh[$m]}; done; done



#Test file existence
#for (( m=0; m < ${meshCount}; m++ )); do if [ -f ${mesh[$m]} ]; then echo "exists" ${mesh[$m]}; fi; done



