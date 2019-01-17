set autoscale                          # scale axes automatically
#unset log                              # remove any log-scaling
set logscale x
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically

set title "Ratio of Faces to Vertices by Increasing Vertex Count"
set xlabel "Vertices in Mesh"
set ylabel "Faces / Vertices in Mesh"
#set key 0.01,100
#set label "Yield Point" at 0.003,260
#set arrow from 0.0028,250 to 0.003,280
#set xr [0:2.2]
#set yr [0:20000000]

#plot    "force.dat" using 1:2 title 'Column' with linespoints , \
#"force.dat" using 1:3 title 'Beam' with points

data = "../experiments/exampleMeshComparison.csv"

plot data using 3:($4 / $3) with lines
