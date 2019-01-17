# set terminal pngcairo  transparent enhanced font "arial,10" fontscale 1.0 size 600, 400 
# set output 'simple.8.png'
# set key bmargin left horizontal Right noreverse enhanced autotitle box lt black linewidth 1.000 dashtype solid
#set samples 800, 800
set title "Simple Plots" 
set title  font ",20" norotate

dataDir = "../experiments/"
data1 = dataDir."Unisiegel_UAH_Ebay-Siegel_Uniarchiv_HE2066-60_010614_partial_ASCII_MSIIfuncvals_1iter_gigamesh.txt" 
data2 = dataDir."Unisiegel_UAH_Ebay-Siegel_Uniarchiv_HE2066-60_010614_partial_ASCII_funcvals_1iter_libcudaonering.txt"

#plot '< paste '.data1.' '.data2 u ($2-$4)
plot '< tail -n +12 '.data1.' | paste - '.data2 u ($2-$4)
