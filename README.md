# Auto Clicker

A simple auto-clicker application built using Qt.   
This program allows users to automate mouse clicks with customizable settings.

![auto-clicker-ui](image.png)

## Features

- Click type selection (left or right click)
- Click duration selection (forever or stop after condition)
- Hotkey assignment for starting / stopping clicks
- Click position (on cursor or random position inside bounding box)
- Click delay (fixed or randomly within bounds)

## Table of Contents

- [Dependencies](#dependencies)
- [Building the Project](#building-the-project)
- [Usage](#usage)
- [License](#license)
- [Contributing](#contributing)
- [Contact](#contact)

## Dependencies

To build and run this project, you will need the following dependencies:

- **Qt 6.6.1**: The application is built using the Qt framework. You can download it from [the official Qt website](https://www.qt.io/download).
- **QHotkey**: A library for handling global hotkeys. This project uses the QHotkey library, which can be found at [Skycoder42/QHotkey](https://github.com/Skycoder42/QHotkey).

## Building the Project

Follow these steps to build the auto-clicker application:

1. **Clone the repository**:
   ```bash
   git clone https://github.com/apesci1/autoclicker.git
   cd autoclicker
   ```

2. **Install Qt**: Make sure you have Qt 6.6.1 installed on your system.   
You can use the Qt Maintenance Tool to install the necessary components.

3. **Open the project in Qt Creator**:
   - Launch Qt Creator.  
   - Open the `CMakelists.txt` file located in the project directory.

4. **Build from source**:
   - Click on the "Build" button in Qt Creator  
   - Alternatively use the following command in the terminal:
     ```bash
     mkdir build
     cd build
     cmake -DCMAKE_PREFIX_PATH=C:\Qt\6.6.1\mingw_64 -DQT_DIR=C:\Qt\6.6.1\mingw_64\lib\cmake -DCMAKE_GENERATOR=Ninja ..
     cmake --build ..
     C:\Qt\6.6.1\mingw_64\bin\windeployqt.exe auto-clicker.exe
     ```
     - mac and x11 cmake parameters will differ slightly.

     **Note**: There is an alternate main branch `main-QHotkey-included` which includes the QHotkey library code, so you don't have to install the library separately. However, please note that this branch may be out of sync with the main branch.

5. **Run the application**:
   - After a successful build, you can run the application directly from Qt Creator or execute the generated binary from the terminal.

## Usage

1. **Launch the application**.
2. **Configure the settings**:
   - Set the click delay (fixed or random).
   - Choose the click position (on cursor or within a bounding box).
   - Select the click type (left or right).
   - Assign a hotkey to start/stop clicking.  
     
3. **Start clicking** by pressing the assigned hotkey or using the UI button.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! If you have suggestions for improvements or new features, feel free to open an issue or submit a pull request.

## Contact

For any inquiries or feedback, please reach out on GitHub: [apesci1](https://github.com/apesci1).