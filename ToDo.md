Updated on 8 June 2023 (too lazy to renumber daily)
1) Bandreject filter
2) Data-request/download from IRIS (or other server)
3) Mapping of data (implot has a nifty map example code)
4) Sorting of data in sac_deque
    4a) By filename
    4b) By component
    4c) By station
    4d) By event station distance
    4e) By azimuth/back-azimuth
    4f) By eventid
    4g) ???? Basically any header variable
5) Geometric calculations (gcarc, az, baz, dist)
6) 3C geometric calculations (rotate components)
7) Manual arrival time picking
8) Automatic arrival time picking (STA/LTA time-series, A1C, STA/LTA spectrogram)
9) Spectrogram
10) Grouping data
    10a) Three-component
    10b) Array/sub-array
    10c) Event
    10d) Manually via the user, can provide their own name
11) Help menu
12) Center windows functionality
13) Overwrite layout functionality
14) Advanced plots (record section, particle motion)
15) Deconvolution (instrument response, source wavelet)
    15a) Spectral division with water-level
    15b) Spectral division with static shift
    15c) Iterative time-domain
19) Generic basic waveforms (for exploring effects of processing flow)
    19a) Dirac Delta function
    19b) Dirac Delta-comb function
    19c) Boxcar function
    19d) Triangle function
    19e) Gaussian
    19f) Sombrero function
20) Keyboard shortcuts for common operations
21) Tab-key navigation between components in window
22) User note's log (project wide, checkpoint notes are already implemented).
24) Migrate all SAC stuff from floats to doubles (while maintaining read/write compatibility)
25) Don't use std::cout or std::cerr, use exceptions and then try-catch blocks to
check for exceptions
26) Plotting appropriately down-sampled seismograms
The plot window has a width in pixels, there is no need to show with a >1 ratio of
data-points to pixels (literally couldn't see the difference)
The xlimits on the window tell us how much of our data is shown, if the data/pixel
ratio >1, down-sample by taking every-other point, if density if <0.5 then upsample
(unless the window is bigger than the available data)
Can then tweak the ratio bounds for when the swap should happen to try to keep things
smooth and clean looking
Could possibly have 3 versions (one denser than shown, the shown, one less dense than shown)
and depending on the change in zoom level we either pop the front or the back and insert
a new one in it's place (to make the transition feel fluid instead of jagged)
The reason to even bother with this is that plotting something with 150,000 data-points
tanks my framerate from ~120 fps on my macbook pro to ~40 fps, even though at most, I can see
2560 pixels wide (retina display), and the plot is only ~60% the width of my screen tops.
So those frames are dropping for stuff I can't even see anyway!
We'll need to make sure the plotted data is within the plot-frame plus a bit extra on each side
if data exists there, we'll also need to shift it into the correct place
27) Currently, the way to prevent crashing when processing is happening is to hide the
"Sac List" window. The reason it to prevent the user selecting data that is not accessible yet.
I would prefer to gray-out the window and make the options non-selectable during processing.
28) FFTW is not thread-safe. It seems to have the possibility to be thread-safe if compiled with the
correct flag. Because on MacOS I use the default provided by homebrew, I don't have that.
Either I should look into an FFT library that is thread-safe or streamline compiling FFTW as thread-safe.
    28a) It may not be necessary, performing bandpass (FFT + gain calculation + IFFT) on 700 files on a single-thread
using FFTW takes about as long as reading in those 700 files across 7 threads. I suspect this will be a bit
more awkward on a more powerful machine?
On my Linux machine, reading 700 files takes ~2-3 seconds, doing a bandpass on them all is about 10 seconds
On my Mac laptop, reading 700 files takes ~12 seconds, doing a bandpass on them all is about 11 seconds
    28b) An FFTW plan pool may make it possible to use FFTW in a multi-threaded capacity without needing its
thread-safe compilation pattern (which they advocate against using: https:www.fftw.org/fftw3_doc/Thread-safety.html)
Idea is to have a pool of plans for different sized input vector.
If a vector needs one of a size that doesn't exist, it gets created.
If a vector needs one that does exist, if it is unused it takes it, otherwise it waits for it to be free.
At the end, all plans get destroyed. It reduces the overhead of repeatidly creating/destroying plans
and makes it thread-safe.
    28c) Possibly a wrapper class for FFTW, maybe that'll make it actually thread-safe...
29) Program logging.
??) Functionality for trimming seismograms
??) Functionality for zero-padding seismograms
??) Functionality for down-sampling seismograms
??) Functionality for up-sampling seismograms
??) Functionality for generating random noise
??) Functionality for stacking seismograms
??a) Standard-stack
??b) Slant-stack
??c) H-k stack
??) Arrival time prediction from 1-d earth model (spherical)
??a) That requires ray-tracing through 1-d earth model
??b) That requires definition of naming of phases
??bb) There are various definitions, but I want to be able to be completely explicit about a phases path
        Both source-side and receiver-side because there is ambiguity in the naming schemes
        As well as specifying depths of certain interactions (I want reflects off the Moho on the receiver-side, or from a theoretical reflection from an interface at a depth of 20-km beneath the receiver, but I don't care about making
        that modification to the 1-D model also effect the source-side, I also don't care if there is no pre-defined reflection
        from that depth, imagine it happend [treat reflection point as a source, can the specified wave get to the receiver?])
        *While non-standard, I think that would be most valuable for the end-user (even though it'll be a pain to implement)
??bb*) To do this, I think ray-tracing needs to work from the end-points in to the "mid" point of the ray-path.
        From the end-point to the next internal point, is it even possible? If no, then fail. If yes, then what conditions must be met. Then within those conditions, is it possible for this internal point to connect appropriately to the next, more-internal, point. Each end does this until either one fails-out, or they match-up and we're done with this check.
        Of course, if there are multiple possible paths, we need to make sure to include them in our analysis (we take the path with least travel-time, but if paths have the same travel-time, we keep them together [like a hash function]).
        We could even, in theory, define the set of points that falls within +/- tolerance of the minimum travel-time found, which would in the future lead to being able to deal with finite-frequency (finite bandwidth, instead of infinitesimal bandwidth) effects.
??) I think that leads naturally to seismic body-wave travel-time inversion (hypocentral, or tomographic, or both).
??) I think that ray-tracing also lends itself to things like back-projection and seismic imaging (CCP stacking of PRFs for instance)
??) Instrument response needs to be dealt with. That means we need to be able to handle PZ-files, or RESP files, to generate
the instruments transfer function, which can then be deconvolved from the time-series.
??*) That means we need to be able to handle doing a  discrete laplace transform [z-transform](https://en.wikipedia.org/wiki/Z-transform)
