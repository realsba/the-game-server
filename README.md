# The Game Server

![GitHub release (latest by date)](https://img.shields.io/github/v/release/realsba/the-game-server)
![GitHub](https://img.shields.io/github/license/realsba/the-game-server)
![GitHub issues](https://img.shields.io/github/issues/realsba/the-game-server)

This project serves as the server component for the multiplayer game "The Game" which interprets the concept of the
popular online game agar.io. Players control cells on a virtual petri dish, aiming to accumulate as much mass as
possible by consuming smaller cells and avoiding becoming prey for larger ones.

## Installation

### Prerequisites

- CMake 3.25 or higher
- Boost headers
- OpenSSL
- spdlog
- MySQL++ library

### Build Instructions

```bash
mkdir cmake-build && cd cmake-build
cmake ..
make
cp ../thegame.toml.sample thegame.toml 
````

## Usage
```bash
./thegame
```

## Project Structure
- src Contains the source code files for the server
- tests Contains the unit tests for the server

## Technologies Used
- C++ (C++23)
- Boost
- OpenSSL
- spdlog
- MySQL++

## Contributing
Contributions are welcome! Feel free to open an issue or submit a pull request.

## License
This project is licensed under the MIT License - see the [LICENSE](https://github.com/realsba/the-game-server/blob/main/LICENSE) file for details.

## Author
- Bohdan Sadovyak

## Bugs/Issues
Please report any bugs or issues [here](https://github.com/realsba/the-game-server/issues).