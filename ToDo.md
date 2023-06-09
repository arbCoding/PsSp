# Updated on 9 June 2023 (too lazy to renumber daily)
---

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

16) Generic basic waveforms (for exploring effects of processing flow)

    16a) Dirac Delta function

    16b) Dirac Delta-comb function

    16c) Boxcar function

    16d) Triangle function

    16e) Gaussian

    16f) Sombrero function

17) Keyboard shortcuts for common operations

18) Tab-key navigation between components in window

19) User note's log (project wide, checkpoint notes are already implemented).

20) Don't use std::cout or std::cerr, use exceptions and then try-catch blocks to
check for exceptions

21) Plotting appropriately down-sampled seismograms

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

22) Currently, the way to prevent crashing when processing is happening is to hide the
"Sac List" window. The reason it to prevent the user selecting data that is not accessible yet.
I would prefer to gray-out the window and make the options non-selectable during processing.

23) Program logging.

24) Functionality for trimming seismograms

25) Functionality for zero-padding seismograms

26) Functionality for down-sampling seismograms

27) Functionality for up-sampling seismograms

28) Functionality for generating random noise

29) Functionality for stacking seismograms

    29a) Standard-stack
    
    29b) Slant-stack

    29c) H-k stack

30) Arrival time prediction from 1-d earth model (spherical)

    30a) That requires ray-tracing through 1-d earth model
    
    30b) That requires definition of naming of phases
        
    30bb) There are various definitions, but I want to be able to be completely explicit about a phases path
        Both source-side and receiver-side because there is ambiguity in the naming schemes
        As well as specifying depths of certain interactions (I want reflects off the Moho on the receiver-side, or from a theoretical reflection from an interface at a depth of 20-km beneath the receiver, but I don't care about making
        that modification to the 1-D model also effect the source-side, I also don't care if there is no pre-defined reflection
        from that depth, imagine it happend [treat reflection point as a source, can the specified wave get to the receiver?])
        *While non-standard, I think that would be most valuable for the end-user (even though it'll be a pain to implement)
    
    30bb*) To do this, I think ray-tracing needs to work from the end-points in to the "mid" point of the ray-path.
        From the end-point to the next internal point, is it even possible? If no, then fail. If yes, then what conditions must be met. Then within those conditions, is it possible for this internal point to connect appropriately to the next, more-internal, point. Each end does this until either one fails-out, or they match-up and we're done with this check.
        Of course, if there are multiple possible paths, we need to make sure to include them in our analysis (we take the path with least travel-time, but if paths have the same travel-time, we keep them together [like a hash function]).
        We could even, in theory, define the set of points that falls within +/- tolerance of the minimum travel-time found, which would in the future lead to being able to deal with finite-frequency (finite bandwidth, instead of infinitesimal bandwidth) effects.

31) I think that leads naturally to seismic body-wave travel-time inversion (hypocentral, or tomographic, or both).

32) I think that ray-tracing also lends itself to things like back-projection and seismic imaging (CCP stacking of PRFs for instance)

33) Instrument response needs to be dealt with. That means we need to be able to handle PZ-files, or RESP files, to generate
the instruments transfer function, which can then be deconvolved from the time-series.

    33*) That means we need to be able to handle doing a  discrete laplace transform [z-transform](https://en.wikipedia.org/wiki/Z-transform)
