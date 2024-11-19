# CNode Server Template
## Overview
This project provides a unique server template for handling complex processes effectively. In this template, Node.js handles communication-related tasks (e.g., WebSocket and HTTP connections), while C++ handles core computation-intensive processes. This separation of roles allows each technology to focus on what it is best at, which improves performance.

## Installation
### Dependencies
- [Node.js](https://nodejs.org/en/) and [npm](https://www.npmjs.com/)
- [Emscripten](https://emscripten.org/)
- [CMake](https://cmake.org/)

### Build
Install additional dependencies used in the project by the following command:
```
npm install
```
You can build the server and client with the following commands:
```
npm run build
```
After the build is complete, you can visit `localhost` (the default port is `6430`) after executing the following commands:
```
npm run start
```