# BackyardBrains LSL Interface

This application streams data to LSL from a BackyardBrains SpikerBoxPro device connected via USB.
This is a work-in-progress and not ready yet!

## Application Architecture

The application loosely follows the [standard LSL App Template](https://github.com/labstreaminglayer/AppTemplate_cpp_qt).
In this application, 

## Dependencies

1. Qt
    * See [here](https://labstreaminglayer.readthedocs.io/dev/build_env.html#qt5)
2. hidapi
    * Windows:
        * Download https://github.com/libusb/hidapi/releases/latest/download/hidapi-win.zip
        * Extract the hidapi-win folder into this repository's root.
        * Save [hidapi.h](https://raw.githubusercontent.com/libusb/hidapi/master/hidapi/hidapi.h) into hidapi-win/x64/ and/or hidapi-win/x86/
    * Linux:
        * `sudo apt-get install -y libhidapi-dev`
    * MacOS:
        * `brew install hidapi`
3. LSL
    * Windows:
        * Download https://github.com/sccn/liblsl/releases/latest/download/liblsl-1.13.0-Win32.7z
        * Create LSL folder in this repository.
        * Extract contents of 7zip file above into LSL folder.
        * cmake will require LSL_INSTALL_ROOT=LSL

## Download

See the [releases page](https://github.com/labstreaminglayer/App-BackyardBrains/releases).
(In progress)

# Build

This application can be built following general
[LSL Application build instructions](https://labstreaminglayer.readthedocs.io/dev/app_build.html).

If you have hidapi installed somewhere other than the default location then you can tell CMake where to find that folder with the cmake argument `HIDAPI_ROOT_DIR=path/to/hidapi_lib_and_h`

# License

This application has components from https://github.com/BackyardBrains/Spike-Recorder but I could not find its license information.
Until the latter professes a more restrictive license, this application is provided with MIT license.
Note that there are some additional limitations due to the use of Qt. See LICENSE.txt
