
#Show original versions of gcc and llvm
gcc --version
llvm-config --version

#Setup new versions
module load GCC/12.2.0
export PATH=/modules/cs325/llvm-17.0.1/bin:$PATH
export LD_LIBRARY_PATH=/modules/cs325/llvm-17.0.1/lib:$LD_LIBRARY_PATH

#show new versions of gcc and llvm
echo "Updated to new versions"
gcc --version
llvm-config --version