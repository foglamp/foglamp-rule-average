========================================
FogLAMP Average notification rule plugin
========================================

A notification plugin that triggers if the value of a datapoint is
more than a prescribed percentage different from the currently observed
average for that data point.

The plugin only monitors a single asset, but will moitor all data points
within that asset. It will trigger if any of the data points within the
asset differ by more than the configured percentage, an average is maintained
for each data point seperately.

A configuration option also allows for control of notification triggering
based on the value being above, below are either side of the average
value.

The average calculated may be either a simple moving average or an
exponential moving average. If an exponential moving average is chosen
then a second configuration parameter allows the setting of the factor
used to calculate that average.

Exponential moving averages give more weight to the recent values compare
to historical values. The smaller the EMA factor the more weight recent
values carry. A value of 1 for factor will only consider at the most recent
value.

The Average rule is not applicable to all data, only simple numeric values
are considered and those values should not deviate with an average of
0 or close to 0 if good results are required. Data points that deviate
wildly are also not suitable for this plugin.

Build
-----
To build FogLAMP "Average" notification rule C++ plugin,
in addition to FogLAMP source code, the Notification server C++
header files are required (no .cpp files or libraries needed so far)

The path with Notification server C++ header files cab be specified only via
NOTIFICATION_SERVICE_INCLUDE_DIRS environment variable.

Example:

.. code-block:: console

  $ export NOTIFICATION_SERVICE_INCLUDE_DIRS=/home/ubuntu/source/foglamp-service-notification/C/services/common/include

.. code-block:: console

  $ mkdir build
  $ cd build
  $ cmake ..
  $ make

- By default the FogLAMP develop package header files and libraries
  are expected to be located in /usr/include/foglamp and /usr/lib/foglamp
- If **FOGLAMP_ROOT** env var is set and no -D options are set,
  the header files and libraries paths are pulled from the ones under the
  FOGLAMP_ROOT directory.
  Please note that you must first run 'make' in the FOGLAMP_ROOT directory.

You may also pass one or more of the following options to cmake to override 
this default behaviour:

- **FOGLAMP_SRC** sets the path of a FogLAMP source tree
- **FOGLAMP_INCLUDE** sets the path to FogLAMP header files
- **FOGLAMP_LIB sets** the path to FogLAMP libraries
- **FOGLAMP_INSTALL** sets the installation path of Random plugin

NOTE:
 - The **FOGLAMP_INCLUDE** option should point to a location where all the FogLAMP 
   header files have been installed in a single directory.
 - The **FOGLAMP_LIB** option should point to a location where all the FogLAMP
   libraries have been installed in a single directory.
 - 'make install' target is defined only when **FOGLAMP_INSTALL** is set

Examples:

- no options

  $ cmake ..

- no options and FOGLAMP_ROOT set

  $ export FOGLAMP_ROOT=/some_foglamp_setup

  $ cmake ..

- set FOGLAMP_SRC

  $ cmake -DFOGLAMP_SRC=/home/source/develop/FogLAMP  ..

- set FOGLAMP_INCLUDE

  $ cmake -DFOGLAMP_INCLUDE=/dev-package/include ..
- set FOGLAMP_LIB

  $ cmake -DFOGLAMP_LIB=/home/dev/package/lib ..
- set FOGLAMP_INSTALL

  $ cmake -DFOGLAMP_INSTALL=/home/source/develop/FogLAMP ..

  $ cmake -DFOGLAMP_INSTALL=/usr/local/foglamp ..
