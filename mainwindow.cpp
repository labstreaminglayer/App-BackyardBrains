/*: # mainwindow.cpp ([source](../appskeleton/mainwindows.cpp))
 * mainwindow.cpp contains the implementations of everything your window does. */
/*: The next two includes are our own headers that define the interfaces for
 * our window class and the recording device */
#include "mainwindow.h"
/*: `ui_mainwindow.h` is automatically generated from `mainwindow.ui`.
 * It defines the `Ui::MainWindow` class with the widgets as members. */
#include "ui_mainwindow.h"


//: Qt headers
#include <QCloseEvent>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
//: standard C++ headers
#include <fstream>
#include <lsl_cpp.h>
#include <string>
#include <vector>

const int DEFAULT_SAMPLE_RATE = 44100;


/*: The constructor mainly sets up the `Ui::MainWindow` class and creates the
 * connections between signals (e.g. 'button X was clicked') and slots
 * (e.g. 'close the window') or functions (e.g. 'save the configuration')
 */
MainWindow::MainWindow(QWidget *parent, const char *config_file)
	: QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);
	/*: C++11 has anonymous functions [lambdas](http://en.cppreference.com/w/cpp/language/lambda)
	 * that can get defined once where they are needed. They are mainly useful
	 * for simple actions as a result of an event */
	connect(ui->actionLoad_Configuration, &QAction::triggered, [this]() {
		load_config(QFileDialog::getOpenFileName(
			this, "Load Configuration File", "", "Configuration Files (*.cfg)"));
	});
	connect(ui->actionSave_Configuration, &QAction::triggered, [this]() {
		save_config(QFileDialog::getSaveFileName(
			this, "Save Configuration File", "", "Configuration Files (*.cfg)"));
	});
	connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
	connect(ui->actionAbout, &QAction::triggered, [this]() {
		QString infostr = QStringLiteral("LSL library version: ") +
						  QString::number(lsl::library_version()) +
						  "\nLSL library info:" + lsl::lsl_library_info();
		QMessageBox::about(this, "About this app", infostr);
	});
	connect(ui->linkButton, &QPushButton::clicked, this, &MainWindow::toggleRecording);

	refresh_devices();

	//: At the end of the constructor, we load the supplied config file or find it
	//: in one of the default paths
	QString cfgfilepath = find_config_file(config_file);
	load_config(cfgfilepath);
}


/*: ## Loading / saving the configuration
 * Most apps have some kind of configuration parameters, e.g. which device to
 * use, how to name the channels, ...
 *
 * The settings are mostly saved in `.ini` files. Qt ships with a parser and
 * writer for these kinds of files ([QSettings](http://doc.qt.io/qt-5/qsettings.html)).
 * The general format is `settings.value("key", "default value").toType()`*/
void MainWindow::load_config(const QString &filename) {
	QSettings settings(filename, QSettings::Format::IniFormat);
	// TODO: Iterate over items in ui->comboBox_device, trying to find text match with
	// settings.value("BYB/path", "???").toString()
}


//: Save function, same as above
void MainWindow::save_config(const QString &filename) {
	QSettings settings(filename, QSettings::Format::IniFormat);
	settings.beginGroup("BYB");
	settings.setValue("path", ui->comboBox_device->currentText());
	settings.sync();
}


/*: ## The close event
 * to avoid accidentally closing the window, we can ignore the close event
 * when there's a recording in progress */
void MainWindow::closeEvent(QCloseEvent *ev) {
	if (reader) {
		QMessageBox::warning(this, "Recording still running", "Can't quit while recording");
		ev->ignore();
	}
}


void MainWindow::refresh_devices() {
	BackyardBrains::HIDUsbManager _hidUsbManager;
	_hidUsbManager.getAllDevicesList();
	_devices.clear();
	_devices = std::vector<BackyardBrains::HIDManagerDevice>{
		std::begin(_hidUsbManager.list), std::end(_hidUsbManager.list)};
	ui->comboBox_device->clear();
	for (BackyardBrains::HIDManagerDevice dev : _devices) {
		ui->comboBox_device->addItem(QString::fromStdString(dev.devicePath));
	}
}


/*: ## The recording thread
 *
 * We run the recording in a separate thread to keep the UI responsive.
 * The recording thread function generally gets called with
 *
 * - the configuration parameters (here `name`, `device_param`)
 * - a reference to an `std::atomic<bool>`
 *
 * the shutdown flag indicates that the recording should stop as soon as possible */
void recording_thread_function(
	const BackyardBrains::HIDManagerDevice _device, std::atomic<bool> &shutdown) {
	int _sampleRate = DEFAULT_SAMPLE_RATE;
	int _numOfHidChannels = 2;
	bool _HIDOpened = false;
	clock_t timerUSB = 0;

	// Create an instance of the HIDUsbManager that will live throughout the thread.
	std::cout << "Creating instance of HIDUsbManager" << std::endl;
	BackyardBrains::HIDUsbManager _hidUsbManager;

	// Open the device and get basic info.
	std::cout << "Attempting to open selected device..." << std::endl;
	while (!shutdown && !_HIDOpened) {

		// Scan for devices, rate limit to once per second.
		try {
			clock_t end = clock();
			double elapsed_secs = double(end - timerUSB) / CLOCKS_PER_SEC;
			if (elapsed_secs > 1) {
				timerUSB = end;
				_hidUsbManager.getAllDevicesList();
				_numOfHidChannels = _hidUsbManager.numberOfChannels();
			}
		} catch (int e) { std::cout << "Error HID scan\n"; }
		std::cout << "HID found " << _hidUsbManager.list.size() << " devices." << std::endl;

		// If necessary, open the device.
		if (!_hidUsbManager.deviceOpened()) {
			std::cout << "Attempting to open device..." << std::endl;
			if (_device.deviceType == HID_BOARD_TYPE_NONE) {
				std::cout << " with type HID_BOARD_TYPE_NONE" << std::endl;
			} else if (_device.deviceType == HID_BOARD_TYPE_MUSCLE) {
				std::cout << " with type HID_BOARD_TYPE_MUSCLE" << std::endl;
			} else if (_device.deviceType == HID_BOARD_TYPE_NEURON) {
				std::cout << " with type HID_BOARD_TYPE_NEURON" << std::endl;
			}

			int err = _hidUsbManager.openDevice(_device.deviceType);
			if (err == -1) {
				std::cout << "Successfully opened HID" << std::endl;
			} else {
				std::cout << "Encountered error " << err << std::endl;
			}
		} else {
			std::cout << "Device already opened." << std::endl;
		}

		_HIDOpened = _hidUsbManager.deviceOpened();
		if (_HIDOpened) {
			_sampleRate = _hidUsbManager.maxSamplingRate();
			_numOfHidChannels = _hidUsbManager.numberOfChannels();
			std::cout << "HID Chan:" << _numOfHidChannels << " Samp:" << _sampleRate << "\n";
		}
	}

	//: create an outlet and a send buffer
	lsl::stream_info info(_device.devicePath, "ExG", _numOfHidChannels, _sampleRate, lsl::cf_int32,
		_device.serialNumber);
	lsl::stream_outlet outlet(info);
	std::cout << "Created outlet with type ExG and name " << _device.devicePath << std::endl;

	uint32_t len = 30000;
	std::vector<int32_t> buffer(_numOfHidChannels * len);

	while (!shutdown && _hidUsbManager.deviceOpened()) {
		// get interleaved data for all channels
		int framesRead = _hidUsbManager.readDevice(buffer.data());
		//std::cout << "framesRead = " << framesRead << std::endl;

		if (framesRead == 0) { continue; }
		if (framesRead != -1) {
			// Send data from buffer to LSL
			outlet.push_chunk_multiplexed(buffer.data(), _numOfHidChannels * framesRead);
		}
	}

	std::cout << "Shutdown received or device not opened." << std::endl;

	_hidUsbManager.closeDevice();
	while (_hidUsbManager.deviceOpened())
	{
		// Wait out the device cleanup.
		std::cout << "Wait out device cleanup..." << std::endl;
	}
	buffer.clear();
}


//: ## Toggling the recording state
//: Our record button has two functions: start a recording and
//: stop it if a recording is running already.
void MainWindow::toggleRecording() {
	/*: the `std::unique_ptr` evaluates to false if it doesn't point to an object,
	 * so we need to start a recording.
	 * First, we load the configuration from the UI fields, set the shutdown flag
	 * to false so the recording thread doesn't quit immediately and create the
	 * recording thread. */
	if (!reader) {
		if (_devices.size() > ui->comboBox_device->currentIndex()) {
			shutdown = false;
			/*: `make_unique` allocates a new `std::thread` with our recording
			 * thread function as first parameters and all parameters to the
			 * function after that.
			 * Reference parameters have to be wrapped as a `std::ref`. */
			std::cout << "Spawning recording thread." << std::endl;
			reader = std::make_unique<std::thread>(&recording_thread_function,
				_devices.at(ui->comboBox_device->currentIndex()), std::ref(shutdown));
			ui->linkButton->setText("Unlink");
		}
	} else {
		/*: Shutting a thread involves 3 things:
		 * - setting the shutdown flag so the thread doesn't continue acquiring data
		 * - wait for the thread to complete (`join()`)
		 * - delete the thread object and set the variable to nullptr */
		shutdown = true;
		reader->join();
		reader.reset();
		ui->linkButton->setText("Link");
	}
}

/**
 * Find a config file to load. This is (in descending order or preference):
 * - a file supplied on the command line
 * - [executablename].cfg in one the the following folders:
 *	- the current working directory
 *	- the default config folder, e.g. '~/Library/Preferences' on OS X
 *	- the executable folder
 * @param filename	Optional file name supplied e.g. as command line parameter
 * @return Path to a found config file
 */
QString MainWindow::find_config_file(const char *filename) {
	if (filename) {
		QString qfilename(filename);
		if (!QFileInfo::exists(qfilename))
			QMessageBox(QMessageBox::Warning, "Config file not found",
				QStringLiteral("The file '%1' doesn't exist").arg(qfilename), QMessageBox::Ok,
				this);
		else
			return qfilename;
	}
	QFileInfo exeInfo(QCoreApplication::applicationFilePath());
	QString defaultCfgFilename(exeInfo.completeBaseName() + ".cfg");
	QStringList cfgpaths;
	cfgpaths << QDir::currentPath()
			 << QStandardPaths::standardLocations(QStandardPaths::ConfigLocation) << exeInfo.path();
	for (auto path : cfgpaths) {
		QString cfgfilepath = path + QDir::separator() + defaultCfgFilename;
		if (QFileInfo::exists(cfgfilepath)) return cfgfilepath;
	}
	QMessageBox(QMessageBox::Warning, "No config file not found",
		QStringLiteral("No default config file could be found"), QMessageBox::Ok, this);
	return "";
}


//: Tell the compiler to put the default destructor in this object file
MainWindow::~MainWindow() noexcept = default;
