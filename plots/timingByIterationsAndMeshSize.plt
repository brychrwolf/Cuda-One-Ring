reset
set datafile separator ","
data = "../experiments/exampleMeshComparison.csv"
set terminal pngcairo size 1280, 960 #enhanced font "Times New Roman, 10"
set output "exampleMeshComparison.png"



set multiplot layout 2, 2 \
title "Compute Times of Applying the One-Ring Filter for Selected Numbers of Iterations\n\
onto Acquired and Synthetic 3D Meshes of Varying Sizes" font ",28"
mSm = 0
mMd = 10
mLg = 26
set title
set tmargin 2
#set xr [0:2.2]
#set yr [0:20000000]



set key off
set lmargin mLg+mSm-mMd
set rmargin mMd
unset log
set ylabel "Compute Time (seconds)\nLinear Scale"
unset xlabel #set xlabel "One-Ring Filter Iterations\nLinear Scale"
plot for [i=3:12] data every ::0::7  using 2:i with linespoints title columnheader, \
	 for [i=3:12] data every ::8::15 using 2:i with linespoints title columnheader



set key on outside 
set lmargin mSm
set rmargin mLg
unset log
set logscale x
unset ylabel #set ylabel "Compute Time (seconds)\nLinear Scale"
unset xlabel #set xlabel "One-Ring Filter Iterations\nLog Scale"
plot for [i=3:12] data every ::0::7  using 2:i with linespoints title columnheader, \
	 for [i=3:12] data every ::8::15 using 2:i with linespoints title columnheader



set key off
set lmargin mLg+mSm-mMd
set rmargin mMd
unset log
set logscale y
set ylabel "Compute Time (seconds)\nLog Scale"
set xlabel "One-Ring Filter Iterations\nLinear Scale"
plot for [i=3:12] data every ::0::7  using 2:i with linespoints title columnheader, \
	 for [i=3:12] data every ::8::15 using 2:i with linespoints title columnheader



set key off
set lmargin mSm
set rmargin mLg
set logscale xy
unset ylabel #set ylabel "Compute Time (seconds)\nLog Scale"
set xlabel "One-Ring Filter Iterations\nLog Scale"
plot for [i=3:12] data every ::0::7  using 2:i with linespoints title columnheader, \
	 for [i=3:12] data every ::8::15 using 2:i with linespoints title columnheader
	


unset multiplot
