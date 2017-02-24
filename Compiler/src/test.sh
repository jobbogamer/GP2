
# Add lib directory to the library path so building gp2run succeeds.
export LIBRARY_PATH="/Users/Josh/Dropbox/University/Project/Modified_Code/Compiler/Compiler/lib"

# Add the directory containing gp2compile to the path.
export PATH="/Users/Josh/Dropbox/University/Project/Modified_Code/Compiler/Compiler/src":$PATH

# Build the compiler.
make build

# Navigate to the location of the test GP2 program. This means the program name in the trace
# will not have the giant path at the start.
pushd /Users/Josh/Dropbox/University/Project/GP2_Programs/Invalid2ColourProgram/graphs/ > /dev/null

# Compile the GP2 program.
gp2compile -t ../programs/2colourable.gp2.tmp ./not_2_colourable.host

# Navigate to the compiled program.
pushd /tmp/gp2/

# Copy the input graph to the current directory so the program can find it.
cp /Users/Josh/Dropbox/University/Project/GP2_Programs/Invalid2ColourProgram/graphs/not_2_colourable.host ./

# Build the program.
make

# Run the program.
echo -e "\nRunning...\n"
./gp2run
echo -e "\n"

# Return to the original directory.
popd > /dev/null
