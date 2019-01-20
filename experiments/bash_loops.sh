declare -a mesh=("acquired_meshes/ILATO_1A_SM2066-HE5-60_070214_merged_GMO_ASCII.ply")
meshCount=${#mesh[@]}
declare -a meshSize=(1 3 10 30 100 300 1000 3000)
declare -a iter=(1 3 10 30 100 300 1000 3000)
for (( m=0; m < ${meshCount}; m++ )); do for i in {1..2}; do optirun ./lib ${iter[$i]} ${mesh[$m]}; done; done
