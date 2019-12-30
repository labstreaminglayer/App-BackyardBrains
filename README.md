![Build Status](https://github.com/labstreaminglayer/App-BackyardBrains/workflows/Cpp-CI/badge.svg)

# BackyardBrains LSL Interface

This application streams data to LSL from a BackyardBrains SpikerBoxPro device connected via USB.
This is an untested work-in-progress!

## Use

When you first run this application, it should scan for appropriate devices connected via
USB and list them in the dropdown box. Select the device you want to stream and click "Link".

It should now be streaming data over LSL. To receive the data in another application,
see the [LabStreamingLayer documentation](https://labstreaminglayer.readthedocs.io/index.html).

### Run Dependencies

The Windows and Mac zip files should come with all of the dependencies you need. However, it seems at least the windows package is missing a couple pieces:
    * [VisualC runtime](https://aka.ms/vs/16/release/vc_redist.x64.exe)
    * [hidapi.dll from this zip](https://github.com/libusb/hidapi/releases/latest/download/hidapi-win.zip)
On Linux you'll have to install them yourself: `sudo apt-get install -y libhidapi-dev qt5-default`

## Application Architecture

The application loosely follows the [standard LSL App Template](https://github.com/labstreaminglayer/AppTemplate_cpp_qt).

## Download

See the [releases page](https://github.com/labstreaminglayer/App-BackyardBrains/releases).

## Build

This application can be built following general
[LSL Application build instructions](https://labstreaminglayer.readthedocs.io/dev/app_build.html).

### Build Dependencies

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
    * **Download from the GitHub release page: https://github.com/sccn/liblsl/releases/latest**
    * Create LSL folder in this repository.
    * Extract contents of 7zip file above into LSL folder.
    * cmake will require `-DLSL_INSTALL_ROOT=LSL`
4. Other CMake options
    * If you have hidapi installed somewhere other than the default location then you can tell CMake where to find that folder with the cmake argument `-DHIDAPI_ROOT_DIR=path/to/hidapi_lib_and_h`
    * `-DLSL_UNIXFOLDERS=Off`

## License

This application has components from https://github.com/BackyardBrains/Spike-Recorder but I could not find its license information.
Until the latter professes a more restrictive license, this application is provided with MIT license.
Note that there are some additional limitations due to the use of Qt. See LICENSE.txt
