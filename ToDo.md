Updated on 15 May 2023 (too lazy to renumber daily)
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
16) Reload all data
17) Unload all data
18) Data-processing logs (probably easier than doing projects...)
19) Generic basic waveforms (for exploring effects of processing flow)
    19a) Dirac Delta function
    19b) Dirac Delta-comb function
    19c) Boxcar function
    19d) Triangle function
    19e) Gaussian
    19f) Sombrero function
20) Keyboard shortcuts for common operations
21) Tab-key navigation between components in window
22) User note's log (can write their own notes on what they're doing)
23) Datetime functionality
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