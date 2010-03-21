Building

cmake is required for the build system
The following common libraries are also needed:
libxml2
Qt (version 4 or higher)
opencv
ffmpeg

cd HandDetection
mkdir build
cd build
cmake ..
make

The executables can be found in:
build/apps
build/vidproc


Running the flesh classifier trainer:

Usage: ./fleshTrainer <feature string> <flesh Image> [...] -x <non-flesh image> [...]

The feature string is a sequence of characters used to identify the colorspace components used as features in the classifier.  I use the string "IQ" for two components from the YIQ colorspace.

The "-x" is used to mark the transition from positive example images to negative example images and should only be entered once.



Running the hand block classifier trainer (not currently needed since the results aren't good enough):

Usage: ./handTrainer <feature string> <flesh classifier file> <hand Image> [...] -x <non-hand image> [...]

The feature string follows the same pattern as with the flesh trainer, but it doesn't have to match the features used to train the specified flesh-classifier file (which is the output from the fleshTrainer program).



Running the video flesh detection program:

Usage: ./fleshblockoutline <classifier file> <video file> <output directory>

Here the classifier file is the output from the fleshTrainer program.  I've always used various kinds of avi files for the video file, but it should handle a wide range of formats.  The output directory needs to be created before running the program.



Running the video hand detection program:

Usage: ./handVidDetect <flesh classifier file> <hand classifier file> <video file> <output directory>

This is like the fleshblockoutline program, but it also runs a hand classifier on the blocks found by the flesh classifier.  So far, I don't have any satisfactory results for the hand classifier.
